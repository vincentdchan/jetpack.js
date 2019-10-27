//
// Created by Duzhong Chen on 2019/10/25.
//

#include <jemalloc/jemalloc.h>
#include <stack>
#include <fmt/format.h>
#include "jsx_parser.h"

namespace parser {

    JSXParser::JSXParser(parser::Parser* parser): parser_(*parser) {
    }

    UString JSXParser::GetQualifiedElementName(const Sp<SyntaxNode> &node) {
        UString qualified_name;

        switch (node->type) {
            case SyntaxNodeType::JSXIdentifier: {
                auto id = dynamic_pointer_cast<JSXIdentifier>(node);
                qualified_name = id->name;
                break;
            }

            case SyntaxNodeType::JSXNamespacedName: {
                auto ns = dynamic_pointer_cast<JSXNamespacedName>(node);
                qualified_name += GetQualifiedElementName(ns->namespace_);
                qualified_name += u":";
                qualified_name += GetQualifiedElementName(ns->name);
                break;
            }

            case SyntaxNodeType::JSXMemberExpression: {
                auto expr = dynamic_pointer_cast<JSXMemberExpression>(node);
                qualified_name += GetQualifiedElementName(expr->object);
                qualified_name += u".";
                qualified_name += GetQualifiedElementName(expr->property);
                break;
            }

            default:
                break;

        }

        return qualified_name;
    }

    Sp<JSXElement> JSXParser::ParseJSXRoot() {
        StartJSX();
        auto elem = ParseJSXElement();
        FinishJSX();

        return elem;
    }

    Sp<JSXElement> JSXParser::ParseJSXElement() {
        auto start_marker = parser_.CreateStartMarker();

        auto node = parser_.Alloc<JSXElement>();
        node->opening_element = ParseJSXOpeningElement();

        if (!node->opening_element->self_closing) {
            auto el = ParseComplexJSXElement(std::shared_ptr<MetaJSXElement>(new MetaJSXElement {
                start_marker,
                node->opening_element,
                nullopt,
                node->children,
            }));

            node->children = move(el->children_);
            node->closing_element = el->closing_;
        }

        return parser_.Finalize(start_marker, node);
    }

    Sp<JSXOpeningElement> JSXParser::ParseJSXOpeningElement() {
        auto start_marker = parser_.CreateStartMarker();

        Expect(JsTokenType::LessThan);
        auto node = parser_.Alloc<JSXOpeningElement>();
        node->name = ParseJSXElementName();
        node->attributes = ParseJSXAttributes();
        node->self_closing = Match(JsTokenType::Div);
        if (node->self_closing) {
            Expect(JsTokenType::Div);
        }
        Expect(JsTokenType::GreaterThan);

        return parser_.Finalize(start_marker, node);
    }

    Sp<SyntaxNode> JSXParser::ParseJSXElementName() {
        auto start_marker = parser_.CreateStartMarker();
        auto element_name = ParseJSXIdentifier();

        if (Match(JsTokenType::Colon)) {
            Expect(JsTokenType::Colon);
            auto node = parser_.Alloc<JSXNamespacedName>();
            node->namespace_ = move(element_name);
            node->name = ParseJSXIdentifier();
            return parser_.Finalize(start_marker, node);
        } else if (Match(JsTokenType::Dot)) {
            Sp<SyntaxNode> object = std::move(element_name);
            while (Match(JsTokenType::Dot)) {
                Expect(JsTokenType::Dot);
                auto node = parser_.Alloc<JSXMemberExpression>();
                node->object = std::move(object);
                node->property = ParseJSXIdentifier();
                object = parser_.Finalize(start_marker, node);
            }
            return object;
        }

        return element_name;
    }

    std::vector<Sp<SyntaxNode>> JSXParser::ParseJSXAttributes() {
        std::vector<Sp<SyntaxNode>> result;

        while (!Match(JsTokenType::Div) && !Match(JsTokenType::GreaterThan)) {
            if (Match(JsTokenType::LeftBracket)) {
                result.push_back(ParseJSXSpreadAttribute());
            } else {
                result.push_back(ParseJSXNameValueAttribute());
            }
        }

        return result;
    }

    Sp<JSXParser::MetaJSXElement>
        JSXParser::ParseComplexJSXElement(Sp<JSXParser::MetaJSXElement> el) {
        std::stack<Sp<MetaJSXElement>> el_stack;

        while (!parser_.scanner_->IsEnd()) {
            auto children = ParseJSXChildren();
            el->children_.insert(el->children_.end(), children.begin(), children.end());

            auto start_marker = parser_.CreateStartMarker();
            auto element = ParseJSXBoundaryElement();

            if (element->type == SyntaxNodeType::JSXOpeningElement) {
                auto opening = dynamic_pointer_cast<JSXOpeningElement>(element);
                if (opening->self_closing) {
                    auto child = parser_.Alloc<JSXElement>();
                    child->opening_element = move(opening);
                    el->children_.emplace_back(move(child));
                } else {
                    el_stack.push(el);
                    el = std::shared_ptr<MetaJSXElement>(new MetaJSXElement{
                        start_marker,
                        opening,
                        nullopt,
                        {},
                    });
                }
            } else if (element->type == SyntaxNodeType::JSXClosingElement) {
                auto closing = dynamic_pointer_cast<JSXClosingElement>(element);
                el->closing_ = { closing };
                UString open = GetQualifiedElementName(el->opening_->name);
                UString close = GetQualifiedElementName(closing->name);
                if (open != close) {
                    parser_.TolerateError(fmt::format("Expected corresponding JSX closing tag for {}", utils::To_UTF8(open)));
                }
                if (!el_stack.empty()) {
                    auto node = parser_.Alloc<JSXElement>();
                    node->opening_element = el->opening_;
                    node->closing_element = el->closing_;
                    node->children = el->children_;
                    auto child = parser_.Finalize(el->start_marker_, node);
                    el = el_stack.top();
                    el->children_.push_back(child);
                    el_stack.pop();
                } else {
                    break;
                }
            }
        }

        return el;
    }

    Sp<JSXIdentifier> JSXParser::ParseJSXIdentifier() {
        auto start_marker = parser_.CreateStartMarker();
        auto token = NextJSXToken();
        if (token.type_ != JsTokenType::Identifier) {
            parser_.ThrowUnexpectedToken(token);
        }
        auto id = parser_.Alloc<JSXIdentifier>();
        id->name = token.value_;
        return parser_.Finalize(start_marker, id);
    }

    Sp<JSXSpreadAttribute> JSXParser::ParseJSXSpreadAttribute() {
        auto start_marker = parser_.CreateStartMarker();
        Expect(JsTokenType::LeftBracket);
        Expect(JsTokenType::Spread);

        auto node = parser_.Alloc<JSXSpreadAttribute>();
        FinishJSX();
        node->argument = parser_.ParseAssignmentExpression();
        ReEnterJSX();

        return parser_.Finalize(start_marker, node);
    }

    Sp<JSXAttribute> JSXParser::ParseJSXNameValueAttribute() {
        auto start_marker = parser_.CreateStartMarker();
        auto node = parser_.Alloc<JSXAttribute>();
        node->name = ParseJSXAttributeName();

        if (Match(JsTokenType::Assign)) {
            Expect(JsTokenType::Assign);
            node->value = { ParseJSXAttributeValue() };
        }

        return parser_.Finalize(start_marker, node);
    }

    Sp<SyntaxNode> JSXParser::ParseJSXAttributeName() {
        auto start_marker = parser_.CreateStartMarker();

        auto id = ParseJSXIdentifier();
        if (Match(JsTokenType::Colon)) {
            Expect(JsTokenType::Colon);
            auto node = parser_.Alloc<JSXNamespacedName>();
            node->namespace_ = std::move(id);
            node->name = ParseJSXIdentifier();
            return parser_.Finalize(start_marker, node);
        }

        return id;
    }

    Sp<Literal> JSXParser::ParseJSXStringLiteralAttribute() {
        auto start_marker = parser_.CreateStartMarker();
        auto token = NextJSXToken();
        if (token.type_ != JsTokenType::StringLiteral) {
            parser_.ThrowUnexpectedToken(token);
        }

        auto node = parser_.Alloc<Literal>();
        node->raw = parser_.GetTokenRaw(token);
        node->value = token.value_;
        return parser_.Finalize(start_marker, node);
    }

    Sp<JSXExpressionContainer> JSXParser::ParseJSXExpressionAttribute() {
        auto start_marker = parser_.CreateStartMarker();

        Expect(JsTokenType::LeftBracket);
        FinishJSX();

        if (parser_.Match(JsTokenType::RightBracket)) {
            parser_.TolerateError("JSX attributes must only be assigned a non-empty expression");
        }

        auto node = parser_.Alloc<JSXExpressionContainer>();
        node->expression = parser_.ParseAssignmentExpression();

        ReEnterJSX();
        return parser_.Finalize(start_marker, node);
    }

    Sp<SyntaxNode> JSXParser::ParseJSXAttributeValue() {
        if (Match(JsTokenType::LeftBracket)) {
            return ParseJSXExpressionAttribute();
        } else if (Match(JsTokenType::LessThan)) {
            return ParseJSXElement();
        } else {
            return ParseJSXStringLiteralAttribute();
        }
    }

    std::vector<Sp<SyntaxNode>> JSXParser::ParseJSXChildren() {
        std::vector<Sp<SyntaxNode>> result;

        while (!parser_.scanner_->IsEnd()) {
            auto start_marker = parser_.CreateStartMarker();
            auto token = NextJSXText();
            if (token.range_.first < token.range_.second) {
                auto node = parser_.Alloc<JSXText>();
                node->raw = parser_.GetTokenRaw(token);
                node->value = token.value_;
                result.push_back(parser_.Finalize(start_marker, node));
            }
            if (parser_.scanner_->CharAt(parser_.scanner_->Index()) == u'{') {
                auto container = ParseJSXExpressionContainer();
                result.push_back(container);
            } else {
                break;
            }
        }

        return result;
    }

    Sp<JSXExpressionContainer> JSXParser::ParseJSXExpressionContainer() {
        auto start_marker = parser_.CreateStartMarker();

        Expect(JsTokenType::LeftBracket);
        FinishJSX();

        if (Match(JsTokenType::RightBracket)) {
            parser_.TolerateError("JSX attributes must only be assigned a non-empty expression");
        }

        auto node = parser_.Alloc<JSXExpressionContainer>();
        node->expression = parser_.ParseAssignmentExpression();

        ReEnterJSX();

        return parser_.Finalize(start_marker, node);
    }

    void JSXParser::StartJSX() {
        parser_.scanner_->SetIndex(parser_.StartMarker().index);
        parser_.scanner_->SetLineNumber(parser_.StartMarker().line);
        parser_.scanner_->SetLineStart(parser_.StartMarker().index - parser_.StartMarker().column);
    }

    void JSXParser::FinishJSX() {
        parser_.NextToken();
    }

    void JSXParser::ReEnterJSX() {
        StartJSX();
        Expect(JsTokenType::RightBracket);
    }

    Token JSXParser::NextJSXToken() {
        parser_.CollectComments();

        parser_.SetStartMarker({
            parser_.scanner_->Index(),
            parser_.scanner_->LineNumber(),
            parser_.scanner_->Index() - parser_.scanner_->LineStart(),
        });

        Token token = LexJSX();

        parser_.SetLastMarker({
            parser_.scanner_->Index(),
            parser_.scanner_->LineNumber(),
            parser_.scanner_->Index() - parser_.scanner_->LineStart(),
        });

        return token;
    }

    Token JSXParser::NextJSXText() {
        parser_.SetStartMarker({
            parser_.scanner_->Index(),
            parser_.scanner_->LineNumber(),
            parser_.scanner_->Index() - parser_.scanner_->LineStart(),
       });

        auto start = parser_.scanner_->Index();

        UString text;
        while (!parser_.scanner_->IsEnd()) {
            char16_t ch = parser_.scanner_->CharAt(parser_.scanner_->Index());
            if (ch == u'{' || ch == u'<') {
                break;
            }
            parser_.scanner_->IncreaseIndex();
            text.push_back(ch);
            if (utils::IsLineTerminator(ch)) {
                parser_.scanner_->SetLineNumber(parser_.scanner_->LineNumber() + 1);
                if (ch == u'\r' && parser_.scanner_->CharAt(parser_.scanner_->Index()) == u'\n') {
                    parser_.scanner_->IncreaseIndex();
                }
                parser_.scanner_->SetLineStart(parser_.scanner_->Index());
            }
        }


        parser_.SetLastMarker({
            parser_.scanner_->Index(),
            parser_.scanner_->LineNumber(),
            parser_.scanner_->Index() - parser_.scanner_->LineStart(),
        });

        Token token;
        token.value_ = move(text);
        token.line_number_ = parser_.scanner_->LineNumber();
        token.line_start_ = parser_.scanner_->LineStart();
        token.range_ = {
            start,
            parser_.scanner_->Index(),
        };

        return token;
    }

    Token JSXParser::LexJSX() {
        char16_t cp = parser_.scanner_->CharAt(parser_.scanner_->Index());

        Token token;
        // < > / : = { }
        switch (cp) {
            case u'<': {
                parser_.scanner_->IncreaseIndex();
                token.type_ = JsTokenType::LessThan;
                token.value_.push_back(cp);
                break;
            }

            case u'>': {
                parser_.scanner_->IncreaseIndex();
                token.type_ = JsTokenType::GreaterThan;
                token.value_.push_back(cp);
                break;
            }

            case u'/': {
                parser_.scanner_->IncreaseIndex();
                token.type_ = JsTokenType::Div;
                token.value_.push_back(cp);
                break;
            }

            case u':': {
                parser_.scanner_->IncreaseIndex();
                token.type_ = JsTokenType::Colon;
                token.value_.push_back(cp);
                break;
            }

            case u'=': {
                parser_.scanner_->IncreaseIndex();
                token.type_ = JsTokenType::Assign;
                token.value_.push_back(cp);
                break;
            }

            case u'{': {
                parser_.scanner_->IncreaseIndex();
                token.type_ = JsTokenType::LeftBracket;
                token.value_.push_back(cp);
                break;
            }

            case u'}': {
                parser_.scanner_->IncreaseIndex();
                token.type_ = JsTokenType::RightBracket;
                token.value_.push_back(cp);
                break;
            }

            case u'"':
            case u'\'': {
                auto start = parser_.scanner_->Index();
                char16_t quote = cp;
                parser_.scanner_->IncreaseIndex();
                UString str;
                while (!parser_.scanner_->IsEnd()) {
                    char16_t ch = parser_.scanner_->CharAt(parser_.scanner_->Index());
                    parser_.scanner_->IncreaseIndex();
                    if (ch == quote) {
                        break;
                    } else if (ch == u'&') {
                        str += ScanXHTMLEntity(quote);
                    } else {
                        str.push_back(ch);
                    }
                }

                token.type_ = JsTokenType::StringLiteral;
                token.value_ = move(str);
                token.line_number_ = parser_.scanner_->LineNumber();
                token.line_start_ = parser_.scanner_->LineStart();
                token.range_ = {
                    start,
                    parser_.scanner_->Index(),
                };

                return token;
            }

            case u'.': {
                auto index = parser_.scanner_->Index();
                char16_t n1 = parser_.scanner_->CharAt(index + 1);
                char16_t n2 = parser_.scanner_->CharAt(index + 2);
                UString value;

                if (n1 == u'.' && n2 == u'.') {
                    token.type_ = JsTokenType::Spread;
                    value = u"...";
                } else {
                    token.type_ = JsTokenType::Dot;
                    value.push_back(u'.');
                }

                parser_.scanner_->SetIndex(index + value.size());

                token.line_number_ = parser_.scanner_->LineNumber();
                token.line_start_ = parser_.scanner_->LineStart();
                token.range_ = {
                    index,
                    parser_.scanner_->Index(),
                };

                return token;
            }

            case u'`': {
                token.type_ = JsTokenType::Template;
                token.line_number_ = parser_.scanner_->LineNumber();
                token.line_start_ = parser_.scanner_->LineStart(),
                token.range_ = {
                    parser_.scanner_->Index(),
                    parser_.scanner_->Index(),
                };
                return token;
            }

            default:
                token.type_ = JsTokenType::Invalid;
        }

        if (token.type_ != JsTokenType::Invalid) {
            token.line_number_ = parser_.scanner_->LineNumber();
            token.line_start_ = parser_.scanner_->LineStart();
            token.range_ = {
                parser_.scanner_->Index() - 1,
                parser_.scanner_->Index(),
            };
            return token;
        }

        if (utils::IsIdentifierStart(cp) && cp != 92) {
            auto start = parser_.scanner_->Index();
            parser_.scanner_->IncreaseIndex();
            while (!parser_.scanner_->IsEnd()) {
                char16_t ch = parser_.scanner_->CharAt(parser_.scanner_->Index());
                if (utils::IsIdentifierPart(ch) && (ch != 92)) {
                    parser_.scanner_->IncreaseIndex();
                } else if (ch == 45) {
                    // Hyphen (char code 45) can be part of an identifier.
                    parser_.scanner_->IncreaseIndex();
                } else {
                    break;
                }
            }
            UString id = parser_.scanner_
                ->Source()
                ->substr(start, parser_.scanner_->Index() - start);

            token.type_ = JsTokenType::Identifier;
            token.value_ = move(id);
            token.line_start_ = parser_.scanner_->LineStart();
            token.line_number_ = parser_.scanner_->LineNumber();
            token.range_ = {
                start,
                parser_.scanner_->Index(),
            };
            return token;
        }

        return parser_.scanner_->Lex();
    }

    UString JSXParser::ScanXHTMLEntity(char16_t quote) {
        UString result = u"&";

        bool valid = true;
        bool terminated = false;
        bool numeric = false;
        bool hex = false;

        while (!parser_.scanner_->IsEnd() && valid && !terminated) {
            char16_t ch = parser_.scanner_->CharAt(parser_.scanner_->Index());
            if (ch == quote) {
                break;
            }
            terminated = (ch == u';');
            result.push_back(ch);
            parser_.scanner_->IncreaseIndex();
            if (!terminated) {
                switch (result.size()) {
                    case 2:
                        // e.g. '&#123;'
                        numeric = (ch == u'#');
                        break;

                    case 3:
                        if (numeric) {
                            // e.g. '&#x41;'
                            hex = (ch == u'x');
                            valid = hex || utils::IsDecimalDigit(ch);
                            numeric &= !hex;
                        }
                        break;

                    default:
                        valid &= !(numeric && !utils::IsDecimalDigit(ch));
                        valid &= !(utils::IsHexDigit(ch));
                        break;

                }
            }
        }

        if (valid && terminated && result.size() > 2) {
            InitXHTMLEntities();

            // e.g. '&#x41;' becomes just '#x41'
            UString str = result.substr(1, result.size() - 2);
            if (numeric && str.size() > 1) {
                std::string utf8 = utils::To_UTF8(str.substr(1));
                result.push_back(std::stoi(utf8));
            } else if (hex && str.size() > 2) {
                std::string utf8 = string("0") + utils::To_UTF8(str.substr(1));
                result.push_back(std::stoi(utf8, 0, 16));
            } else if (!numeric&& !hex && XHTMLEntities->find(str) != XHTMLEntities->end()) {
                result.push_back((*XHTMLEntities)[str]);
            }
        }

        return result;
    }

    Sp<SyntaxNode> JSXParser::ParseJSXBoundaryElement() {
        auto start_marker = parser_.CreateStartMarker();

        Expect(JsTokenType::LessThan);
        if (Match(JsTokenType::Div)) {
            Expect(JsTokenType::Div);
            auto node = parser_.Alloc<JSXClosingElement>();
            node->name = ParseJSXElementName();
            Expect(JsTokenType::GreaterThan);
            return parser_.Finalize(start_marker, node);
        }

        auto node = parser_.Alloc<JSXOpeningElement>();
        node->name = ParseJSXElementName();
        node->attributes = ParseJSXAttributes();
        node->self_closing = Match(JsTokenType::Div);
        if (node->self_closing) {
            Expect(JsTokenType::Div);
        }
        Expect(JsTokenType::GreaterThan);

        return parser_.Finalize(start_marker, node);
    }

    Token JSXParser::PeekJSXToken() {
        auto state =  parser_.scanner_->SaveState();

        std::vector<Sp<Comment>> comments;
        parser_.scanner_->ScanComments(comments);
        Token next = LexJSX();
        parser_.scanner_->RestoreState(state);

        return next;
    }

    void JSXParser::InitXHTMLEntities() {
        if (XHTMLEntities) return;

        XHTMLEntities.reset(new robin_hood::unordered_map<std::u16string, char16_t> {
            {u"quot", 0x0022},
            {u"amp", 0x0026},
            {u"apos", 0x0027},
            {u"mapgt", 0x003E},
            {u"nbsp", 0x00A0},
            {u"iexcl", 0x00A1},
            {u"cent", 0x00A2},
            {u"pound", 0x00A3},
            {u"curren", 0x00A4},
            {u"yen", 0x00A5},
            {u"brvbar", 0x00A6},
            {u"sect", 0x00A7},
            {u"uml", 0x00A8},
            {u"copy", 0x00A9},
            {u"ordf", 0x00AA},
            {u"laquo", 0x00AB},
            {u"not", 0x00AC},
            {u"shy", 0x00AD},
            {u"reg", 0x00AE},
            {u"macr", 0x00AF},
            {u"deg", 0x00B0},
            {u"plusmn", 0x00B1},
            {u"sup2", 0x00B2},
            {u"sup3", 0x00B3},
            {u"acute", 0x00B4},
            {u"micro", 0x00B5},
            {u"para", 0x00B6},
            {u"middot", 0x00B7},
            {u"cedil", 0x00B8},
            {u"sup1", 0x00B9},
            {u"ordm", 0x00BA},
            {u"raquo", 0x00BB},
            {u"frac14", 0x00BC},
            {u"frac12", 0x00BD},
            {u"frac34", 0x00BE},
            {u"iquest", 0x00BF},
            {u"Agrave", 0x00C0},
            {u"Aacute", 0x00C1},
            {u"Acirc", 0x00C2},
            {u"Atilde", 0x00C3},
            {u"Auml", 0x00C4},
            {u"Aring", 0x00C5},
            {u"AElig", 0x00C6},
            {u"Ccedil", 0x00C7},
            {u"Egrave", 0x00C8},
            {u"Eacute", 0x00C9},
            {u"Ecirc", 0x00CA},
            {u"Euml", 0x00CB},
            {u"Igrave", 0x00CC},
            {u"Iacute", 0x00CD},
            {u"Icirc", 0x00CE},
            {u"Iuml", 0x00CF},
            {u"ETH", 0x00D0},
            {u"Ntilde", 0x00D1},
            {u"Ograve", 0x00D2},
            {u"Oacute", 0x00D3},
            {u"Ocirc", 0x00D4},
            {u"Otilde", 0x00D5},
            {u"Ouml", 0x00D6},
            {u"times", 0x00D7},
            {u"Oslash", 0x00D8},
            {u"Ugrave", 0x00D9},
            {u"Uacute", 0x00DA},
            {u"Ucirc", 0x00DB},
            {u"Uuml", 0x00DC},
            {u"Yacute", 0x00DD},
            {u"THORN", 0x00DE},
            {u"szlig", 0x00DF},
            {u"agrave", 0x00E0},
            {u"aacute", 0x00E1},
            {u"acirc", 0x00E2},
            {u"atilde", 0x00E3},
            {u"auml", 0x00E4},
            {u"aring", 0x00E5},
            {u"aelig", 0x00E6},
            {u"ccedil", 0x00E7},
            {u"egrave", 0x00E8},
            {u"eacute", 0x00E9},
            {u"ecirc", 0x00EA},
            {u"euml", 0x00EB},
            {u"igrave", 0x00EC},
            {u"iacute", 0x00ED},
            {u"icirc", 0x00EE},
            {u"iuml", 0x00EF},
            {u"eth", 0x00F0},
            {u"ntilde", 0x00F1},
            {u"ograve", 0x00F2},
            {u"oacute", 0x00F3},
            {u"ocirc", 0x00F4},
            {u"otilde", 0x00F5},
            {u"ouml", 0x00F6},
            {u"divide", 0x00F7},
            {u"oslash", 0x00F8},
            {u"ugrave", 0x00F9},
            {u"uacute", 0x00FA},
            {u"ucirc", 0x00FB},
            {u"uuml", 0x00FC},
            {u"yacute", 0x00FD},
            {u"thorn", 0x00FE},
            {u"yuml", 0x00FF},
            {u"OElig", 0x0152},
            {u"oelig", 0x0153},
            {u"Scaron", 0x0160},
            {u"scaron", 0x0161},
            {u"Yuml", 0x0178},
            {u"fnof", 0x0192},
            {u"circ", 0x02C6},
            {u"tilde", 0x02DC},
            {u"Alpha", 0x0391},
            {u"Beta", 0x0392},
            {u"Gamma", 0x0393},
            {u"Delta", 0x0394},
            {u"Epsilon", 0x0395},
            {u"Zeta", 0x0396},
            {u"Eta", 0x0397},
            {u"Theta", 0x0398},
            {u"Iota", 0x0399},
            {u"Kappa", 0x039A},
            {u"Lambda", 0x039B},
            {u"Mu", 0x039C},
            {u"Nu", 0x039D},
            {u"Xi", 0x039E},
            {u"Omicron", 0x039F},
            {u"Pi", 0x03A0},
            {u"Rho", 0x03A1},
            {u"Sigma", 0x03A3},
            {u"Tau", 0x03A4},
            {u"Upsilon", 0x03A5},
            {u"Phi", 0x03A6},
            {u"Chi", 0x03A7},
            {u"Psi", 0x03A8},
            {u"Omega", 0x03A9},
            {u"alpha", 0x03B1},
            {u"beta", 0x03B2},
            {u"gamma", 0x03B3},
            {u"delta", 0x03B4},
            {u"epsilon", 0x03B5},
            {u"zeta", 0x03B6},
            {u"eta", 0x03B7},
            {u"theta", 0x03B8},
            {u"iota", 0x03B9},
            {u"kappa", 0x03BA},
            {u"lambda", 0x03BB},
            {u"mu", 0x03BC},
            {u"nu", 0x03BD},
            {u"xi", 0x03BE},
            {u"omicron", 0x03BF},
            {u"pi", 0x03C0},
            {u"rho", 0x03C1},
            {u"sigmaf", 0x03C2},
            {u"sigma", 0x03C3},
            {u"tau", 0x03C4},
            {u"upsilon", 0x03C5},
            {u"phi", 0x03C6},
            {u"chi", 0x03C7},
            {u"psi", 0x03C8},
            {u"omega", 0x03C9},
            {u"thetasym", 0x03D1},
            {u"upsih", 0x03D2},
            {u"piv", 0x03D6},
            {u"ensp", 0x2002},
            {u"emsp", 0x2003},
            {u"thinsp", 0x2009},
            {u"zwnj", 0x200C},
            {u"zwj", 0x200D},
            {u"lrm", 0x200E},
            {u"rlm", 0x200F},
            {u"ndash", 0x2013},
            {u"mdash", 0x2014},
            {u"lsquo", 0x2018},
            {u"rsquo", 0x2019},
            {u"sbquo", 0x201A},
            {u"ldquo", 0x201C},
            {u"rdquo", 0x201D},
            {u"bdquo", 0x201E},
            {u"dagger", 0x2020},
            {u"Dagger", 0x2021},
            {u"bull", 0x2022},
            {u"hellip", 0x2026},
            {u"permil", 0x2030},
            {u"prime", 0x2032},
            {u"Prime", 0x2033},
            {u"lsaquo", 0x2039},
            {u"rsaquo", 0x203A},
            {u"oline", 0x203E},
            {u"frasl", 0x2044},
            {u"euro", 0x20AC},
            {u"image", 0x2111},
            {u"weierp", 0x2118},
            {u"real", 0x211C},
            {u"trade", 0x2122},
            {u"alefsym", 0x2135},
            {u"larr", 0x2190},
            {u"uarr", 0x2191},
            {u"rarr", 0x2192},
            {u"darr", 0x2193},
            {u"harr", 0x2194},
            {u"crarr", 0x21B5},
            {u"lArr", 0x21D0},
            {u"uArr", 0x21D1},
            {u"rArr", 0x21D2},
            {u"dArr", 0x21D3},
            {u"hArr", 0x21D4},
            {u"forall", 0x2200},
            {u"part", 0x2202},
            {u"exist", 0x2203},
            {u"empty", 0x2205},
            {u"nabla", 0x2207},
            {u"isin", 0x2208},
            {u"notin", 0x2209},
            {u"ni", 0x220B},
            {u"prod", 0x220F},
            {u"sum", 0x2211},
            {u"minus", 0x2212},
            {u"lowast", 0x2217},
            {u"radic", 0x221A},
            {u"prop", 0x221D},
            {u"infin", 0x221E},
            {u"ang", 0x2220},
            {u"and", 0x2227},
            {u"or", 0x2228},
            {u"cap", 0x2229},
            {u"cup", 0x222A},
            {u"int", 0x222B},
            {u"there4", 0x2234},
            {u"sim", 0x223C},
            {u"cong", 0x2245},
            {u"asymp", 0x2248},
            {u"ne", 0x2260},
            {u"equiv", 0x2261},
            {u"le", 0x2264},
            {u"ge", 0x2265},
            {u"sub", 0x2282},
            {u"sup", 0x2283},
            {u"nsub", 0x2284},
            {u"sube", 0x2286},
            {u"supe", 0x2287},
            {u"oplus", 0x2295},
            {u"otimes", 0x2297},
            {u"perp", 0x22A5},
            {u"sdot", 0x22C5},
            {u"lceil", 0x2308},
            {u"rceil", 0x2309},
            {u"lfloor", 0x230A},
            {u"rfloor", 0x230B},
            {u"loz", 0x25CA},
            {u"spades", 0x2660},
            {u"clubs", 0x2663},
            {u"hearts", 0x2665},
            {u"diams", 0x2666},
            {u"lang", 0x27E8},
            {u"rang", 0x27E9}
        });

    }

    std::unique_ptr<robin_hood::unordered_map<std::u16string, char16_t>> JSXParser::XHTMLEntities;

}
