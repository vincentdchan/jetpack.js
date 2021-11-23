//
// Created by Duzhong Chen on 2019/10/25.
//

#include <stack>
#include <fmt/format.h>
#include "utils/string/UString.h"
#include "utils/string/UChar.h"
#include "JSXParser.h"

namespace jetpack::parser {
    using namespace std;

    JSXParser::JSXParser(Parser& parent, std::shared_ptr<ParserContext> ctx)
    : ParserCommon(ctx), parent_(parent)  {
    }

    std::string JSXParser::GetQualifiedElementName(SyntaxNode* node) {
        std::string qualified_name;

        switch (node->type) {
            case SyntaxNodeType::JSXIdentifier: {
                auto id = dynamic_cast<JSXIdentifier*>(node);
                qualified_name = id->name;
                break;
            }

            case SyntaxNodeType::JSXNamespacedName: {
                auto ns = dynamic_cast<JSXNamespacedName*>(node);
                qualified_name += GetQualifiedElementName(ns->namespace_);
                qualified_name += ":";
                qualified_name += GetQualifiedElementName(ns->name);
                break;
            }

            case SyntaxNodeType::JSXMemberExpression: {
                auto expr = dynamic_cast<JSXMemberExpression*>(node);
                qualified_name += GetQualifiedElementName(expr->object);
                qualified_name += ".";
                qualified_name += GetQualifiedElementName(expr->property);
                break;
            }

            default:
                break;

        }

        return qualified_name;
    }

    Expression* JSXParser::ParseJSXRoot(Scope& scope) {
        StartJSX();
        auto elem = ParseJSXElement(scope);
        FinishJSX();

        if (this->ctx->config_.transpile_jsx) {
            return TranspileJSX(scope, elem);
        }
        return elem;
    }

    inline Identifier* MakeId(AstContext& ctx, const std::string& content) {
        auto id = ctx.Alloc<Identifier>();
        id->name = content;
        return id;
    }

    Expression* JSXParser::TranspileJSX(Scope& scope, JSXElement* jsx) {
        auto result = Alloc<CallExpression>();

        auto left = Alloc<MemberExpression>();
        auto id = MakeId(ctx->ast_context_, "React");
        left->object = id;
        left->property = MakeId(ctx->ast_context_, "createElement");

        scope.AddUnresolvedId(id);

        result->callee = left;

        bool has_added_lit = false;
        if (jsx->opening_element->name->type == SyntaxNodeType::JSXIdentifier) {
            auto jsx_id = dynamic_cast<JSXIdentifier*>(jsx->opening_element->name);
            if (!jsx_id->name.empty()) {
                char16_t ch = jsx_id->name.at(0);
                if (ch >= u'a' && ch <= u'z') {
                    auto new_lit = Alloc<Literal>();
                    new_lit->str_ = jsx_id->name;
                    new_lit->raw = "\"" + jsx_id->name + "\"";

                    result->arguments.push_back(new_lit);

                    has_added_lit = true;
                }
            }
        }

        if (!has_added_lit) {
            // do something
        }

        if (!jsx->opening_element->attributes.empty()) {
            auto obj_expr = Alloc<ObjectExpression>();
            for (auto& attrib : jsx->opening_element->attributes) {
                switch (attrib->type) {
                    case SyntaxNodeType::JSXAttribute: {
                        auto named_attr = dynamic_cast<JSXAttribute*>(attrib);
                        auto jsx_name = dynamic_cast<JSXIdentifier*>(named_attr->name);

                        auto prop = Alloc<Property>();
                        prop->key = MakeId(ctx->ast_context_, jsx_name->name);

                        if (named_attr->value.has_value()) {
                            auto value = *named_attr->value;
                            switch (value->type) {
                                case SyntaxNodeType::JSXElement: {
                                    auto jsx_elem = dynamic_cast<JSXElement*>(value);
                                    prop->value = TranspileJSX(scope, jsx_elem);
                                    break;
                                }

                                case SyntaxNodeType::Literal: {
                                    auto lit = dynamic_cast<Literal*>(value);
                                    prop->value = lit;
                                    break;
                                }

                                case SyntaxNodeType::JSXExpressionContainer: {
                                    auto expr_container = dynamic_cast<JSXExpressionContainer*>(value);
                                    prop->value = expr_container->expression;
                                    break;
                                }

                                default:
                                    break;

                            }

                        } else {
                            auto bool_lit = Alloc<Literal>();
                            bool_lit->ty = Literal::Ty::Boolean;
                            bool_lit->raw = "true";
                            bool_lit->str_ = "true";
                            prop->value = bool_lit;
                        }

                        obj_expr->properties.push_back(prop);
                        break;
                    }

                    case SyntaxNodeType::JSXSpreadAttribute: {
                        auto spread_attr = dynamic_cast<JSXSpreadAttribute*>(attrib);

                        auto spread_element = Alloc<SpreadElement>();

                        spread_element->argument = spread_attr->argument;

                        obj_expr->properties.push_back(spread_element);
                        break;
                    }

                    default:
                        break;

                }
            }
            result->arguments.push_back(obj_expr);
        } else {
            if (!jsx->children.empty()) {
                auto null_lit = Alloc<Literal>();
                null_lit->ty = Literal::Ty::Null;
                null_lit->str_ = "null";
                null_lit->raw = "null";
                result->arguments.push_back(null_lit);
            }
        }

        auto children = TranspileJSXChildren(scope, jsx->children);
        for (auto child : children) {
            result->arguments.push_back(child);
        }

        return result;
    }

    std::vector<SyntaxNode*>
    JSXParser::TranspileJSXChildren(Scope& scope,
                                    const std::vector<SyntaxNode*> &children) {
        std::vector<SyntaxNode*> result;

        for (auto& child : children) {
            switch (child->type) {
                case SyntaxNodeType::JSXText: {
                    auto text = dynamic_cast<JSXText*>(child);

                    auto str_lit = Alloc<Literal>();
                    str_lit->ty = Literal::Ty::String;
                    str_lit->str_ = text->value;
                    str_lit->raw = "\"" + text->raw + "\"";

                    result.push_back(str_lit);
                    break;
                }

                case SyntaxNodeType::JSXElement: {
                    auto child_elem = dynamic_cast<JSXElement*>(child);
                    result.push_back(TranspileJSX(scope, child_elem));
                    break;
                }

                case SyntaxNodeType::JSXExpressionContainer: {
                    auto expr = dynamic_cast<JSXExpressionContainer*>(child);
                    result.push_back(expr->expression);
                    break;
                }

                default:
                    break;

            }

        }

        return result;
    }

    JSXElement* JSXParser::ParseJSXElement(Scope& scope) {
        auto start_marker = CreateStartMarker();

        auto node = Alloc<JSXElement>();
        node->opening_element = ParseJSXOpeningElement(scope);

        if (!node->opening_element->self_closing) {
            auto el = ParseComplexJSXElement(scope, std::shared_ptr<MetaJSXElement>(new MetaJSXElement {
                start_marker,
                node->opening_element,
                nullopt,
                node->children,
            }));

            node->children = move(el->children_);
            node->closing_element = el->closing_;
        }

        return Finalize(start_marker, node);
    }

    JSXOpeningElement* JSXParser::ParseJSXOpeningElement(Scope& scope) {
        auto start_marker = CreateStartMarker();

        JSXExpect(JsTokenType::LessThan);
        auto node = Alloc<JSXOpeningElement>();
        node->name = ParseJSXElementName(scope);
        node->attributes = ParseJSXAttributes(scope);
        node->self_closing = JSXMatch(JsTokenType::Div);
        if (node->self_closing) {
            JSXExpect(JsTokenType::Div);
        }
        JSXExpect(JsTokenType::GreaterThan);

        return Finalize(start_marker, node);
    }

    SyntaxNode* JSXParser::ParseJSXElementName(Scope& scope) {
        auto start_marker = CreateStartMarker();
        auto element_name = ParseJSXIdentifier();

        if (JSXMatch(JsTokenType::Colon)) {
            JSXExpect(JsTokenType::Colon);
            auto node = Alloc<JSXNamespacedName>();
            node->namespace_ = element_name;
            node->name = ParseJSXIdentifier();
            return Finalize(start_marker, node);
        } else if (JSXMatch(JsTokenType::Dot)) {
            SyntaxNode* object = element_name;
            while (JSXMatch(JsTokenType::Dot)) {
                JSXExpect(JsTokenType::Dot);
                auto node = Alloc<JSXMemberExpression>();
                node->object = object;
                node->property = ParseJSXIdentifier();
                object = Finalize(start_marker, node);
            }
            return object;
        }

        return element_name;
    }

    std::vector<SyntaxNode*> JSXParser::ParseJSXAttributes(Scope& scope) {
        std::vector<SyntaxNode*> result;

        while (!JSXMatch(JsTokenType::Div) && !JSXMatch(JsTokenType::GreaterThan)) {
            if (JSXMatch(JsTokenType::LeftBracket)) {
                result.push_back(ParseJSXSpreadAttribute(scope));
            } else {
                result.push_back(ParseJSXNameValueAttribute(scope));
            }
        }

        return result;
    }

    Sp<JSXParser::MetaJSXElement>
        JSXParser::ParseComplexJSXElement(Scope& scope, Sp<JSXParser::MetaJSXElement> el) {
        std::stack<Sp<MetaJSXElement>> el_stack;

        while (!ctx->scanner_->IsEnd()) {
            auto children = ParseJSXChildren(scope);
            el->children_.insert(el->children_.end(), children.begin(), children.end());

            auto start_marker = CreateStartMarker();
            auto element = ParseJSXBoundaryElement(scope);

            if (element->type == SyntaxNodeType::JSXOpeningElement) {
                auto opening = dynamic_cast<JSXOpeningElement*>(element);
                if (opening->self_closing) {
                    auto child = Alloc<JSXElement>();
                    child->opening_element = opening;
                    el->children_.emplace_back(child);
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
                auto closing = dynamic_cast<JSXClosingElement*>(element);
                el->closing_ = { closing };
                auto open = GetQualifiedElementName(el->opening_->name);
                auto close = GetQualifiedElementName(closing->name);
                if (open != close) {
                    TolerateError(fmt::format("Expected corresponding JSX closing tag for {}", open));
                }
                if (!el_stack.empty()) {
                    auto node = Alloc<JSXElement>();
                    node->opening_element = el->opening_;
                    node->closing_element = el->closing_;
                    node->children = el->children_;
                    auto child = Finalize(el->start_marker_, node);
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

    JSXIdentifier* JSXParser::ParseJSXIdentifier() {
        auto start_marker = CreateStartMarker();
        auto token = NextJSXToken();
        if (token.type != JsTokenType::Identifier) {
            ThrowUnexpectedToken(token);
        }
        auto id = Alloc<JSXIdentifier>();
        id->name = token.value;
        return Finalize(start_marker, id);
    }

    JSXSpreadAttribute* JSXParser::ParseJSXSpreadAttribute(Scope& scope) {
        auto start_marker = CreateStartMarker();
        JSXExpect(JsTokenType::LeftBracket);
        JSXExpect(JsTokenType::Spread);

        auto node = Alloc<JSXSpreadAttribute>();
        FinishJSX();

        node->argument = parent_.ParseAssignmentExpression(scope);
        ReEnterJSX();

        return Finalize(start_marker, node);
    }

    JSXAttribute* JSXParser::ParseJSXNameValueAttribute(Scope& scope) {
        auto start_marker = CreateStartMarker();
        auto node = Alloc<JSXAttribute>();
        node->name = ParseJSXAttributeName();

        if (JSXMatch(JsTokenType::Assign)) {
            JSXExpect(JsTokenType::Assign);
            node->value = { ParseJSXAttributeValue(scope) };
        }

        return Finalize(start_marker, node);
    }

    SyntaxNode* JSXParser::ParseJSXAttributeName() {
        auto start_marker = CreateStartMarker();

        auto id = ParseJSXIdentifier();
        if (JSXMatch(JsTokenType::Colon)) {
            JSXExpect(JsTokenType::Colon);
            auto node = Alloc<JSXNamespacedName>();
            node->namespace_ = id;
            node->name = ParseJSXIdentifier();
            return Finalize(start_marker, node);
        }

        return id;
    }

    Literal* JSXParser::ParseJSXStringLiteralAttribute() {
        auto start_marker = CreateStartMarker();
        auto token = NextJSXToken();
        if (token.type != JsTokenType::StringLiteral) {
            ThrowUnexpectedToken(token);
        }

        auto node = Alloc<Literal>();
        node->raw = GetTokenRaw(token);
        node->ty = Literal::Ty::String;
        node->str_ = token.value;
        return Finalize(start_marker, node);
    }

    JSXExpressionContainer* JSXParser::ParseJSXExpressionAttribute(Scope& scope) {
        auto start_marker = CreateStartMarker();

        JSXExpect(JsTokenType::LeftBracket);
        FinishJSX();

        if (Match(JsTokenType::RightBracket)) {
            TolerateError("JSX attributes must only be assigned a non-empty expression");
        }

        auto node = Alloc<JSXExpressionContainer>();

        node->expression = parent_.ParseAssignmentExpression(scope);

        ReEnterJSX();
        return Finalize(start_marker, node);
    }

    SyntaxNode* JSXParser::ParseJSXAttributeValue(Scope& scope) {
        if (JSXMatch(JsTokenType::LeftBracket)) {
            return ParseJSXExpressionAttribute(scope);
        } else if (JSXMatch(JsTokenType::LessThan)) {
            return ParseJSXElement(scope);
        } else {
            return ParseJSXStringLiteralAttribute();
        }
    }

    std::vector<SyntaxNode*> JSXParser::ParseJSXChildren(Scope& scope) {
        std::vector<SyntaxNode*> result;

        while (!ctx->scanner_->IsEnd()) {
            auto start_marker = CreateStartMarker();
            auto token = NextJSXText();
            if (token.range.first < token.range.second) {
                auto node = Alloc<JSXText>();
                node->raw = GetTokenRaw(token);
                node->value = token.value;
                result.push_back(Finalize(start_marker, node));
            }
            if (ctx->scanner_->CharAt(ctx->scanner_->Index().u8) == u'{') {
                auto container = ParseJSXExpressionContainer(scope);
                result.push_back(container);
            } else {
                break;
            }
        }

        return result;
    }

    JSXExpressionContainer* JSXParser::ParseJSXExpressionContainer(Scope& scope) {
        auto start_marker = CreateStartMarker();

        JSXExpect(JsTokenType::LeftBracket);
        FinishJSX();

        if (JSXMatch(JsTokenType::RightBracket)) {
            TolerateError("JSX attributes must only be assigned a non-empty expression");
        }

        auto node = Alloc<JSXExpressionContainer>();

        node->expression = parent_.ParseAssignmentExpression(scope);

        ReEnterJSX();

        return Finalize(start_marker, node);
    }

    void JSXParser::StartJSX() {
        ctx->scanner_->SetIndex(StartMarker().cursor);
        ctx->scanner_->SetLineNumber(StartMarker().line);
        ctx->scanner_->SetLineStart(StartMarker().cursor.u16 - StartMarker().column);
    }

    void JSXParser::FinishJSX() {
        NextToken();
    }

    void JSXParser::ReEnterJSX() {
        StartJSX();
        JSXExpect(JsTokenType::RightBracket);
    }

    Token JSXParser::NextJSXToken() {
        CollectComments();

        Scanner& scanner = *ctx->scanner_;

        SetStartMarker({
            scanner.Index(),
            scanner.LineNumber(),
            scanner.Index().u16 - scanner.LineStart(),
        });

        Token token = LexJSX();

        SetLastMarker({
            scanner.Index(),
            scanner.LineNumber(),
            scanner.Index().u16 - scanner.LineStart(),
        });

        return token;
    }

    Token JSXParser::NextJSXText() {
        Scanner& scanner = *ctx->scanner_;

        SetStartMarker({
            scanner.Index(),
            scanner.LineNumber(),
            scanner.Index().u16 - scanner.LineStart(),
        });

        auto start = scanner.Index();

        std::string text;
        while (!scanner.IsEnd()) {
            char ch = scanner.CharAt(scanner.Index().u8);
            if (ch == u'{' || ch == u'<') {
                break;
            }
            scanner.NextChar();
            text.push_back(ch);
            if (UChar::IsLineTerminator(ch)) {
                scanner.SetLineNumber(scanner.LineNumber() + 1);
                if (ch == '\r' && scanner.Peek() == '\n') {
                    scanner.NextChar();
                }
                scanner.SetLineStart(scanner.Index().u8);
            }
        }

        SetLastMarker({
            scanner.Index(),
            scanner.LineNumber(),
            scanner.Index().u16 - scanner.LineStart(),
        });

        Token token;
        token.value = move(text);
        token.line_number = scanner.LineNumber();
        token.line_start = scanner.LineStart();
        token.range = {
            start.u8,
            scanner.Index().u8,
        };

        return token;
    }

    Token JSXParser::LexJSX() {
        auto& scanner = *ctx->scanner_;

        char cp = scanner.Peek();

        Token token;
        // < > / : = { }
        switch (cp) {
            case '<': {
                scanner.NextChar();
                token.type = JsTokenType::LessThan;
                token.value.push_back(cp);
                break;
            }

            case '>': {
                scanner.NextChar();
                token.type = JsTokenType::GreaterThan;
                token.value.push_back(cp);
                break;
            }

            case '/': {
                scanner.NextChar();
                token.type = JsTokenType::Div;
                token.value.push_back(cp);
                break;
            }

            case ':': {
                scanner.NextChar();
                token.type = JsTokenType::Colon;
                token.value.push_back(cp);
                break;
            }

            case '=': {
                scanner.NextChar();
                token.type = JsTokenType::Assign;
                token.value.push_back(cp);
                break;
            }

            case '{': {
                scanner.NextChar();
                token.type = JsTokenType::LeftBracket;
                token.value.push_back(cp);
                break;
            }

            case '}': {
                scanner.NextChar();
                token.type = JsTokenType::RightBracket;
                token.value.push_back(cp);
                break;
            }

            case '"':
            case '\'': {
                auto start = scanner.Index();
                char16_t quote = cp;
                scanner.NextChar();
                std::string str;
                while (!scanner.IsEnd()) {
                    char ch = scanner.CharAt(scanner.Index().u8);
                    scanner.NextChar();
                    if (ch == quote) {
                        break;
                    } else if (ch == '&') {
                        str += ScanXHTMLEntity(quote);
                    } else {
                        str.push_back(ch);
                    }
                }

                token.type = JsTokenType::StringLiteral;
                token.value = move(str);
                token.line_number = scanner.LineNumber();
                token.line_start = scanner.LineStart();
                token.range = {
                    start.u8,
                    scanner.Index().u8,
                };

                return token;
            }

            case '.': {
                auto index = scanner.Index();
                char n1 = scanner.Peek(1);
                char n2 = scanner.Peek(2);
                std::string value;

                if (n1 == '.' && n2 == '.') {
                    token.type = JsTokenType::Spread;
                    value = "...";
                } else {
                    token.type = JsTokenType::Dot;
                    value.push_back('.');
                }

                auto tmp = index;
                tmp.u8 += value.size();
                tmp.u16 += value.size();
                scanner.SetIndex(tmp);

                token.line_number = scanner.LineNumber();
                token.line_start = scanner.LineStart();
                token.value = std::move(value);
                token.range = {
                    index.u8,
                    scanner.Index().u8,
                };

                return token;
            }

            case '`': {
                token.type = JsTokenType::Template;
                token.line_number = scanner.LineNumber();
                token.line_start = scanner.LineStart(),
                token.range = {
                    scanner.Index().u8,
                    scanner.Index().u8,
                    // TODO: + 1?
                };
                return token;
            }

            default:
                token.type = JsTokenType::Invalid;
        }

        if (token.type != JsTokenType::Invalid) {
            token.line_number = scanner.LineNumber();
            token.line_start = scanner.LineStart();
            token.range = {
                scanner.Index().u8 - 1,
                scanner.Index().u8,
            };
            return token;
        }

        if (UChar::IsIdentifierStart(cp) && cp != '\\') {
            auto start = scanner.Index();
            scanner.NextChar();
            while (!scanner.IsEnd()) {
                char ch = scanner.Peek();
                if (UChar::IsIdentifierPart(ch) && (ch != '\\')) {
                    scanner.NextChar();
                } else if (ch == '-') {
                    // Hyphen (char code 45) can be part of an identifier.
                    scanner.NextChar();
                } else {
                    break;
                }
            }
            std::string_view id = scanner.Source()->View().substr(start.u8, scanner.Index().u8 - start.u8);

            token.type = JsTokenType::Identifier;
            token.value = std::string(id);
            token.line_start = scanner.LineStart();
            token.line_number = scanner.LineNumber();
            token.range = {
                start.u8,
                scanner.Index().u8,
            };
            return token;
        }

        return scanner.Lex();
    }

    std::string JSXParser::ScanXHTMLEntity(char quote) {
        std::string result = "&";

        bool valid = true;
        bool terminated = false;
        bool numeric = false;
        bool hex = false;

        Scanner& scanner = *ctx->scanner_;

        while (!scanner.IsEnd() && valid && !terminated) {
            char ch = scanner.Peek();
            if (ch == quote) {
                break;
            }
            terminated = (ch == ';');
            result.push_back(ch);
            scanner.NextChar();
            if (!terminated) {
                switch (result.size()) {
                    case 2:
                        // e.g. '&#123;'
                        numeric = (ch == '#');
                        break;

                    case 3:
                        if (numeric) {
                            // e.g. '&#x41;'
                            hex = (ch == 'x');
                            valid = hex || UChar::IsDecimalDigit(ch);
                            numeric &= !hex;
                        }
                        break;

                    default:
                        valid &= !(numeric && !UChar::IsDecimalDigit(ch));
                        valid &= !(UChar::IsHexDigit(ch));
                        break;

                }
            }
        }

        if (valid && terminated && result.size() > 2) {
            InitXHTMLEntities();

            // e.g. '&#x41;' becomes just '#x41'
            std::string str = result.substr(1, result.size() - 2);
            if (numeric && str.size() > 1) {
                std::string utf8 = str.substr(1);
                result.push_back(std::stoi(utf8));
            } else if (hex && str.size() > 2) {
                std::string utf8 = "0" + str.substr(1);
                result.push_back(std::stoi(utf8, 0, 16));
            } else if (!numeric&& !hex && XHTMLEntities->find(str) != XHTMLEntities->end()) {
                result.push_back((*XHTMLEntities)[str]);
            }
        }

        return result;
    }

    SyntaxNode* JSXParser::ParseJSXBoundaryElement(Scope& scope) {
        auto start_marker = CreateStartMarker();

        JSXExpect(JsTokenType::LessThan);
        if (JSXMatch(JsTokenType::Div)) {
            JSXExpect(JsTokenType::Div);
            auto node = Alloc<JSXClosingElement>();
            node->name = ParseJSXElementName(scope);
            JSXExpect(JsTokenType::GreaterThan);
            return Finalize(start_marker, node);
        }

        auto node = Alloc<JSXOpeningElement>();
        node->name = ParseJSXElementName(scope);
        node->attributes = ParseJSXAttributes(scope);
        node->self_closing = JSXMatch(JsTokenType::Div);
        if (node->self_closing) {
            JSXExpect(JsTokenType::Div);
        }
        JSXExpect(JsTokenType::GreaterThan);

        return Finalize(start_marker, node);
    }

    Token JSXParser::PeekJSXToken() {
        auto state =  ctx->scanner_->SaveState();

        std::vector<Sp<Comment>> comments;
        ctx->scanner_->ScanComments(comments);
        Token next = LexJSX();
        ctx->scanner_->RestoreState(state);

        return next;
    }

    void JSXParser::InitXHTMLEntities() {
        if (XHTMLEntities) return;

        XHTMLEntities.reset(new robin_hood::unordered_map<std::string, char16_t> {
            {"quot", 0x0022},
            {"amp", 0x0026},
            {"apos", 0x0027},
            {"mapgt", 0x003E},
            {"nbsp", 0x00A0},
            {"iexcl", 0x00A1},
            {"cent", 0x00A2},
            {"pound", 0x00A3},
            {"curren", 0x00A4},
            {"yen", 0x00A5},
            {"brvbar", 0x00A6},
            {"sect", 0x00A7},
            {"uml", 0x00A8},
            {"copy", 0x00A9},
            {"ordf", 0x00AA},
            {"laquo", 0x00AB},
            {"not", 0x00AC},
            {"shy", 0x00AD},
            {"reg", 0x00AE},
            {"macr", 0x00AF},
            {"deg", 0x00B0},
            {"plusmn", 0x00B1},
            {"sup2", 0x00B2},
            {"sup3", 0x00B3},
            {"acute", 0x00B4},
            {"micro", 0x00B5},
            {"para", 0x00B6},
            {"middot", 0x00B7},
            {"cedil", 0x00B8},
            {"sup1", 0x00B9},
            {"ordm", 0x00BA},
            {"raquo", 0x00BB},
            {"frac14", 0x00BC},
            {"frac12", 0x00BD},
            {"frac34", 0x00BE},
            {"iquest", 0x00BF},
            {"Agrave", 0x00C0},
            {"Aacute", 0x00C1},
            {"Acirc", 0x00C2},
            {"Atilde", 0x00C3},
            {"Auml", 0x00C4},
            {"Aring", 0x00C5},
            {"AElig", 0x00C6},
            {"Ccedil", 0x00C7},
            {"Egrave", 0x00C8},
            {"Eacute", 0x00C9},
            {"Ecirc", 0x00CA},
            {"Euml", 0x00CB},
            {"Igrave", 0x00CC},
            {"Iacute", 0x00CD},
            {"Icirc", 0x00CE},
            {"Iuml", 0x00CF},
            {"ETH", 0x00D0},
            {"Ntilde", 0x00D1},
            {"Ograve", 0x00D2},
            {"Oacute", 0x00D3},
            {"Ocirc", 0x00D4},
            {"Otilde", 0x00D5},
            {"Ouml", 0x00D6},
            {"times", 0x00D7},
            {"Oslash", 0x00D8},
            {"Ugrave", 0x00D9},
            {"Uacute", 0x00DA},
            {"Ucirc", 0x00DB},
            {"Uuml", 0x00DC},
            {"Yacute", 0x00DD},
            {"THORN", 0x00DE},
            {"szlig", 0x00DF},
            {"agrave", 0x00E0},
            {"aacute", 0x00E1},
            {"acirc", 0x00E2},
            {"atilde", 0x00E3},
            {"auml", 0x00E4},
            {"aring", 0x00E5},
            {"aelig", 0x00E6},
            {"ccedil", 0x00E7},
            {"egrave", 0x00E8},
            {"eacute", 0x00E9},
            {"ecirc", 0x00EA},
            {"euml", 0x00EB},
            {"igrave", 0x00EC},
            {"iacute", 0x00ED},
            {"icirc", 0x00EE},
            {"iuml", 0x00EF},
            {"eth", 0x00F0},
            {"ntilde", 0x00F1},
            {"ograve", 0x00F2},
            {"oacute", 0x00F3},
            {"ocirc", 0x00F4},
            {"otilde", 0x00F5},
            {"ouml", 0x00F6},
            {"divide", 0x00F7},
            {"oslash", 0x00F8},
            {"ugrave", 0x00F9},
            {"uacute", 0x00FA},
            {"ucirc", 0x00FB},
            {"uuml", 0x00FC},
            {"yacute", 0x00FD},
            {"thorn", 0x00FE},
            {"yuml", 0x00FF},
            {"OElig", 0x0152},
            {"oelig", 0x0153},
            {"Scaron", 0x0160},
            {"scaron", 0x0161},
            {"Yuml", 0x0178},
            {"fnof", 0x0192},
            {"circ", 0x02C6},
            {"tilde", 0x02DC},
            {"Alpha", 0x0391},
            {"Beta", 0x0392},
            {"Gamma", 0x0393},
            {"Delta", 0x0394},
            {"Epsilon", 0x0395},
            {"Zeta", 0x0396},
            {"Eta", 0x0397},
            {"Theta", 0x0398},
            {"Iota", 0x0399},
            {"Kappa", 0x039A},
            {"Lambda", 0x039B},
            {"Mu", 0x039C},
            {"Nu", 0x039D},
            {"Xi", 0x039E},
            {"Omicron", 0x039F},
            {"Pi", 0x03A0},
            {"Rho", 0x03A1},
            {"Sigma", 0x03A3},
            {"Tau", 0x03A4},
            {"Upsilon", 0x03A5},
            {"Phi", 0x03A6},
            {"Chi", 0x03A7},
            {"Psi", 0x03A8},
            {"Omega", 0x03A9},
            {"alpha", 0x03B1},
            {"beta", 0x03B2},
            {"gamma", 0x03B3},
            {"delta", 0x03B4},
            {"epsilon", 0x03B5},
            {"zeta", 0x03B6},
            {"eta", 0x03B7},
            {"theta", 0x03B8},
            {"iota", 0x03B9},
            {"kappa", 0x03BA},
            {"lambda", 0x03BB},
            {"mu", 0x03BC},
            {"nu", 0x03BD},
            {"xi", 0x03BE},
            {"omicron", 0x03BF},
            {"pi", 0x03C0},
            {"rho", 0x03C1},
            {"sigmaf", 0x03C2},
            {"sigma", 0x03C3},
            {"tau", 0x03C4},
            {"upsilon", 0x03C5},
            {"phi", 0x03C6},
            {"chi", 0x03C7},
            {"psi", 0x03C8},
            {"omega", 0x03C9},
            {"thetasym", 0x03D1},
            {"upsih", 0x03D2},
            {"piv", 0x03D6},
            {"ensp", 0x2002},
            {"emsp", 0x2003},
            {"thinsp", 0x2009},
            {"zwnj", 0x200C},
            {"zwj", 0x200D},
            {"lrm", 0x200E},
            {"rlm", 0x200F},
            {"ndash", 0x2013},
            {"mdash", 0x2014},
            {"lsquo", 0x2018},
            {"rsquo", 0x2019},
            {"sbquo", 0x201A},
            {"ldquo", 0x201C},
            {"rdquo", 0x201D},
            {"bdquo", 0x201E},
            {"dagger", 0x2020},
            {"Dagger", 0x2021},
            {"bull", 0x2022},
            {"hellip", 0x2026},
            {"permil", 0x2030},
            {"prime", 0x2032},
            {"Prime", 0x2033},
            {"lsaquo", 0x2039},
            {"rsaquo", 0x203A},
            {"oline", 0x203E},
            {"frasl", 0x2044},
            {"euro", 0x20AC},
            {"image", 0x2111},
            {"weierp", 0x2118},
            {"real", 0x211C},
            {"trade", 0x2122},
            {"alefsym", 0x2135},
            {"larr", 0x2190},
            {"uarr", 0x2191},
            {"rarr", 0x2192},
            {"darr", 0x2193},
            {"harr", 0x2194},
            {"crarr", 0x21B5},
            {"lArr", 0x21D0},
            {"uArr", 0x21D1},
            {"rArr", 0x21D2},
            {"dArr", 0x21D3},
            {"hArr", 0x21D4},
            {"forall", 0x2200},
            {"part", 0x2202},
            {"exist", 0x2203},
            {"empty", 0x2205},
            {"nabla", 0x2207},
            {"isin", 0x2208},
            {"notin", 0x2209},
            {"ni", 0x220B},
            {"prod", 0x220F},
            {"sum", 0x2211},
            {"minus", 0x2212},
            {"lowast", 0x2217},
            {"radic", 0x221A},
            {"prop", 0x221D},
            {"infin", 0x221E},
            {"ang", 0x2220},
            {"and", 0x2227},
            {"or", 0x2228},
            {"cap", 0x2229},
            {"cup", 0x222A},
            {"int", 0x222B},
            {"there4", 0x2234},
            {"sim", 0x223C},
            {"cong", 0x2245},
            {"asymp", 0x2248},
            {"ne", 0x2260},
            {"equiv", 0x2261},
            {"le", 0x2264},
            {"ge", 0x2265},
            {"sub", 0x2282},
            {"sup", 0x2283},
            {"nsub", 0x2284},
            {"sube", 0x2286},
            {"supe", 0x2287},
            {"oplus", 0x2295},
            {"otimes", 0x2297},
            {"perp", 0x22A5},
            {"sdot", 0x22C5},
            {"lceil", 0x2308},
            {"rceil", 0x2309},
            {"lfloor", 0x230A},
            {"rfloor", 0x230B},
            {"loz", 0x25CA},
            {"spades", 0x2660},
            {"clubs", 0x2663},
            {"hearts", 0x2665},
            {"diams", 0x2666},
            {"lang", 0x27E8},
            {"rang", 0x27E9}
        });

    }

    std::unique_ptr<HashMap<std::string, char16_t>> JSXParser::XHTMLEntities;

}
