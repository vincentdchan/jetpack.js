//
// Created by Duzhong Chen on 2019/9/20.
//

#include "parser.hpp"

namespace parser {

    bool Parser::IsLexicalDeclaration() {
        auto state = scanner_->SaveState();
        std::vector<Comment> comments;
        scanner_->ScanComments(comments);
        Token next = scanner_->Lex();
        scanner_->RestoreState(state);

        return (next.type_ == JsTokenType::Identifier) ||
               (next.type_ == JsTokenType::Punctuator && next.value_ == u"[") ||
               (next.type_ == JsTokenType::Punctuator && next.value_ == u"{") ||
               (next.type_ == JsTokenType::Keyword && next.value_ == u"let") ||
               (next.type_ == JsTokenType::Keyword && next.value_ == u"yield");
    }

    bool Parser::MatchImportCall() {
        bool match = MatchKeyword(u"import");
        if (match) {
            auto state = scanner_->SaveState();
            std::vector<Comment> comments;
            Token next = scanner_->Lex();
            scanner_->ScanComments(comments);
            scanner_->RestoreState(state);
            match = (next.type_ == JsTokenType::Punctuator) && (next.value_ == u"(");
        }
        return match;
    }

    bool Parser::IsStartOfExpression() {
        bool start = true;

        UString value = lookahead_.value_;
        switch (lookahead_.type_) {
            case JsTokenType::Punctuator: {
                start = (value == u"[") || (value == u")") || (value == u"{") ||
                        (value == u"+") || (value == u"-") ||
                        (value == u"!") || (value == u"~") ||
                        (value == u"++") || (value == u"--") ||
                        (value == u"/") || (value == u"/=");

                break;
            }

            case JsTokenType::Keyword: {
                start = (value == u"class") || (value == u"delete") ||
                        (value == u"function") || (value == u"let") || (value == u"new") ||
                        (value == u"super") || (value == u"this") || (value == u"typeof") ||
                        (value == u"void") || (value == u"yield");

                break;
            }

            default:
                break;
        }

        return start;
    }

    void Parser::ParseFormalParameters(optional<Token> first_restricted, parser::Parser::FormalParameterOptions &options) {

        options.simple = true;
        options.params.clear();
        options.first_restricted = first_restricted;

        Expect('(');
        if (!Match(')')) {
            while (lookahead_.type_ != JsTokenType::EOF_) {
                ParseFormalParameter(options);
                if (Match(')')) {
                    break;
                }
                Expect(',');
                if (Match(')')) {
                    break;
                }
            }
        }
        Expect(')');
    }

    bool Parser::ParseFormalParameter(parser::Parser::FormalParameterOptions &option) {
        if (Match(u"...")) {

        }

        return true;
    }

    Sp<Expression> Parser::ParsePrimaryExpression() {
        auto marker = CreateStartMarker();
        Token token;

        switch (lookahead_.type_) {
            case JsTokenType::Identifier: {
                if ((context_.is_module || context_.await) && lookahead_.value_ == u"await") {
                    ThrowUnexpectedToken(lookahead_);
                }
                if (MatchAsyncFunction()) {
                    return ParseFunctionExpression();
                } else {
                    auto node = Alloc<Identifier>();
                    Token next = NextToken();
                    node->name = next.value_;
                    return Finalize(marker, node);
                }
            }

            case JsTokenType::NumericLiteral:
            case JsTokenType::StringLiteral: {
                if (context_.strict_ && lookahead_.octal_) {
                    ThrowUnexpectedToken(lookahead_);
//                this.tolerateUnexpectedToken(this.lookahead, Messages.StrictOctalLiteral);
                }
                context_.is_assignment_target = false;
                context_.is_binding_element = false;
                token = NextToken();
//            raw = this.getTokenRaw(token);
                auto node = Alloc<Literal>();
                node->value = token.value_;
                return Finalize(marker, node);
            }

            case JsTokenType::BooleanLiteral: {
                context_.is_assignment_target = false;
                context_.is_binding_element = false;
                token = NextToken();
//            raw = this.getTokenRaw(token);
                auto node = Alloc<Literal>();
                node->value = token.value_;
                return Finalize(marker, node);
            }

            case JsTokenType::NullLiteral: {
                context_.is_assignment_target = false;
                context_.is_binding_element = false;
                token = NextToken();
//            raw = this.getTokenRaw(token);
                auto node = Alloc<Literal>();
                node->value = token.value_;
                return Finalize(marker, node);
            }

            case JsTokenType::Template:
                return ParseTemplateLiteral();

            case JsTokenType::Punctuator: {
                switch (lookahead_.value_[0]) {
                    case '(':
                        context_.is_binding_element = false;
                        return InheritCoverGrammar<Expression>([this]() {
                            return ParseGroupExpression();
                        });

                    case '[':
                        return InheritCoverGrammar<Expression>([this]() {
                            return ParseArrayInitializer();
                        });

                    case '{':
                        return InheritCoverGrammar<Expression>([this]() {
                            return ParseObjectInitializer();
                        });

                    case '/':
//                case '/=': TODO:
//                    context_.is_assignment_target = false;
//                    context_.is_binding_element = false;
//                    scanner_->SetIndex(start_marker_.index);
//                    token = this.nextRegexToken();
//                    raw = this.getTokenRaw(token);
//                    expr = this.finalize(node, new Node.RegexLiteral(token.regex
//                    as
//                    RegExp, raw, token.pattern, token.flags));
                        ThrowUnexpectedToken(token);
                        return nullptr;

                    default: {
                        token = NextToken();
                        ThrowUnexpectedToken(token);
                    }

                }
                break;
            }

            case JsTokenType::Keyword:
                if (!context_.strict_ && context_.allow_yield && MatchKeyword(u"yield")) {
                    return ParseIdentifierName();
                } else if (!context_.strict_ && MatchKeyword(u"let")) {
                    token = NextToken();
                    auto id = Alloc<Identifier>();
                    id->name = token.value_;
                    return Finalize(marker, id);
                } else {
                    context_.is_assignment_target = false;
                    context_.is_binding_element = false;
                    if (MatchKeyword(u"function")) {
                        return ParseFunctionExpression();
                    } else if (MatchKeyword(u"this")) {
                        token = NextToken();
                        auto th = Alloc<ThisExpression>();
                        return Finalize(marker, th);
                    } else if (MatchKeyword(u"class")) {
                        return ParseClassExpression();
                    } else if (MatchImportCall()) {
                        return ParseImportCall();
                    } else {
                        ThrowUnexpectedToken(NextToken());
                        return nullptr;
                    }
                }
                break;

            default:
                ThrowUnexpectedToken(NextToken());
                return nullptr;

        }

        // TODO;: check;
        return nullptr;
    }

    Sp<SpreadElement> Parser::ParseSpreadElement() {
        auto marker = CreateStartMarker();

        Expect(u"...");

        auto node = Alloc<SpreadElement>();
        node->argument = InheritCoverGrammar<Expression>([this] {
            return ParseAssignmentExpression();
        });

        return Finalize(marker, node);
    }

    Sp<SyntaxNode> Parser::ParseObjectProperty(bool &has_proto) {
        auto marker = CreateStartMarker();
        Token token = lookahead_;
        VarKind kind;
        bool computed = false;
        bool method = false;
        bool shorthand = false;
        bool is_async = false;

        Sp<SyntaxNode> key = nullptr;

        if (token.type_ == JsTokenType::Identifier) {
            auto id = token.value_;
            NextToken();
            computed = Match('[');
            is_async = !has_line_terminator_ && (id == u"async") &&
                       !Match(':') && !Match('(') && !Match('*') && !Match(',');
            if (is_async) {
                key = ParseObjectPropertyKey();
            } else {
                auto node = Alloc<Identifier>();
                node->name = id;
                key = Finalize(marker, node);
            }
        } else if (Match('*')) {
            NextToken();
        } else {
            computed = Match('[');
            key = ParseObjectPropertyKey();
        }

        return key;
    }

    Sp<SyntaxNode> Parser::ParseObjectPropertyKey() {
        auto marker = CreateStartMarker();
        Token token = NextToken();

        switch (token.type_) {
            case JsTokenType::StringLiteral:
            case JsTokenType::NumericLiteral: {
                if (context_.strict_ && token.octal_) {
                    TolerateUnexpectedToken(token, ParseMessages::StrictOctalLiteral);
                }
                auto node = Alloc<Literal>();
                node->value = token.value_;
                return Finalize(marker, node);
            }

            case JsTokenType::Identifier:
            case JsTokenType::BooleanLiteral:
            case JsTokenType::NullLiteral:
            case JsTokenType::Keyword: {
                auto node = Alloc<Identifier>();
                node->name = token.value_;
                return Finalize(marker, node);
            }

            case JsTokenType::Punctuator:
                if (token.value_ == u"[") {
                    auto result = IsolateCoverGrammar<Expression>([this] {
                        return ParseAssignmentExpression();
                    });
                    Expect(']');
                    return result;
                } else {
                    ThrowUnexpectedToken(token);
                    return nullptr;
                }

            default:
                ThrowUnexpectedToken(token);
                return nullptr;
        }

        return nullptr;
    }

    Sp<Expression> Parser::ParseObjectInitializer() {
        auto marker = CreateStartMarker();

        Expect(u'{');
        auto node = Alloc<ObjectExpression>();
        bool has_proto = false;
        while (!Match(u'}')) {
            Sp<SyntaxNode> prop;
            if (Match(u"...")) {
                prop = ParseSpreadElement();
            } else {
                prop = ParseObjectProperty(has_proto);
            }
            node->properties.push_back(std::move(prop));
            if (!Match(u'}')) {
                ExpectCommaSeparator();
            }
        }
        Expect(u'}');

        return Finalize(marker, node);
    }

    Sp<FunctionExpression> Parser::ParseFunctionExpression() {
        auto marker = CreateStartMarker();

        bool is_async = MatchContextualKeyword(u"async");
        if (is_async) NextToken();

        ExpectKeyword(u"function");

        bool is_generator = is_async ? false : Match(u'*');
        if (is_generator) NextToken();

        std::optional<Sp<Identifier>> id;
        Token first_restricted;

        bool prev_allow_await = context_.await;
        bool prev_allow_yield = context_.allow_yield;
        context_.await = is_async;
        context_.allow_yield = !is_generator;

        std::string message;

        if (Match(u'(')) {
            Token token = lookahead_;

            if (!context_.strict_ && !is_generator && MatchKeyword(u"yield")) {
                id = ParseIdentifierName();
            } else {
                id = ParseVariableIdentifier(VarKind::Invalid);
            }

            if (context_.strict_) {
                if (scanner_->IsRestrictedWord(token.value_)) {
                    TolerateUnexpectedToken(token, ParseMessages::StrictFunctionName);
                }
            } else {
                if (scanner_->IsRestrictedWord(token.value_)) {
                    first_restricted = token;
                    message = ParseMessages::StrictFunctionName;
                } else if (scanner_->IsStrictModeReservedWord(token.value_)) {
                    first_restricted = token;
                    message = ParseMessages::StrictReservedWord;
                }
            }
        }

//        FormalParameterOptions formal;
//        DO(ParseFormalParameters(first_restricted, formal))
        // TODO: parse fomral

        return nullptr;
    }

    Sp<Identifier> Parser::ParseIdentifierName() {
        auto marker = CreateStartMarker();
        auto node = Alloc<Identifier>();
        Token token = NextToken();
        if (!IsIdentifierName(token)) {
            ThrowUnexpectedToken(token);
            return nullptr;
        }
        node->name = token.value_;
        return Finalize(marker, node);
    }

    Sp<Expression> Parser::ParseNewExpression() {
        auto start_marker = CreateStartMarker();

        Sp<Identifier> id = ParseIdentifierName();
        Assert(id->name == u"new", "New expression must start with `new`");

        Sp<Expression> expr;

        if (Match(u'.')) {
            NextToken();
            if (lookahead_.type_ == JsTokenType::Identifier && context_.in_function_body && lookahead_.value_ == u"target") {
                auto node = Alloc<MetaProperty>();
                node->property = ParseIdentifierName();
                node->meta = id;
                expr = node;
            } else {
                ThrowUnexpectedToken(lookahead_);
                return nullptr;
            }
        } else if (MatchKeyword(u"import")) {
            ThrowUnexpectedToken(lookahead_);
            return nullptr;
        } else {
            auto node = Alloc<NewExpression>();
            node->callee = IsolateCoverGrammar<Expression>([this] {
                return ParseLeftHandSideExpression();
            });
            if (Match(u'(')) {
                node->arguments = ParseArguments();
            }
            expr = node;
            context_.is_assignment_target = false;
            context_.is_binding_element = false;
        }

        return Finalize(start_marker, expr);
    }

    Sp<YieldExpression> Parser::ParseYieldExpression() {

        auto marker = CreateStartMarker();
        ExpectKeyword(u"yield");

        auto node = Alloc<YieldExpression>();
        if (has_line_terminator_) {
            bool prev_allow_yield = context_.allow_yield;
            context_.allow_yield = false;
            node->delegate = Match(u'*');
            if (node->delegate) {
                NextToken();
                node->argument = ParseAssignmentExpression();
            } else if (IsStartOfExpression()) {
                node->argument = ParseAssignmentExpression();
            }
            context_.allow_yield = prev_allow_yield;
        }

        return Finalize(marker, node);
    }

    std::vector<Sp<SyntaxNode>> Parser::ParseArguments() {
        std::vector<Sp<SyntaxNode>> result;
        Expect(u'(');
        if (!Match(u')')) {
            Sp<SyntaxNode> expr;
            while (true) {
                if (Match(u"...")) {
                    expr = ParseSpreadElement();
                } else {
                    expr = IsolateCoverGrammar<Expression>([this] {
                        return ParseAssignmentExpression();
                    });
                }
                result.push_back(expr);
                if (Match(u')')) {
                    break;
                }
                ExpectCommaSeparator();
                if (Match(u')')) {
                    break;
                }
            }
        }
        Expect(u')');
        return result;
    }

    Sp<Import> Parser::ParseImportCall() {
        auto marker = CreateStartMarker();
        auto node = Alloc<Import>();
        ExpectKeyword(u"import");
        return Finalize(marker, node);
    }

    Sp<Statement> Parser::ParseDirective() {
        auto token = lookahead_;

        auto marker = CreateStartMarker();
        Sp<Expression> expr = ParseExpression();
        UString directive;
        if (expr->type == SyntaxNodeType::Literal) {
            directive = token.value_.substr(1, token.value_.size() - 1);
        }
        ConsumeSemicolon();

        if (!directive.empty()) {
            auto node = Alloc<Directive>();
            node->expression = expr;
            node->directive = directive;
            return Finalize(marker, node);
        } else {
            auto node = Alloc<ExpressionStatement>();
            node->expression = expr;
            return Finalize(marker, node);
        }
    }

    Sp<Expression> Parser::ParseExpression() {
        auto start_token = lookahead_;
        auto start_marker = CreateStartMarker();
        Sp<Expression> expr = IsolateCoverGrammar<Expression>([this] {
            return ParseAssignmentExpression();
        });

        if (Match(u',')) {
            std::vector<Sp<Expression>> expressions;
            expressions.push_back(expr);
            while (lookahead_.type_ != JsTokenType::EOF_) {
                if (!Match(u',')) {
                    break;
                }
                NextToken();
                Sp<Expression> node = IsolateCoverGrammar<Expression>([this] {
                    return ParseAssignmentExpression();
                });
                expressions.push_back(node);
            }

            auto node = Alloc<SequenceExpression>();
            node->expressions = expressions;
            expr = Finalize(start_marker, node);
        }

        return expr;
    }

    Sp<BlockStatement> Parser::ParseFunctionSourceElements() {
        auto start_marker = CreateStartMarker();

        vector<Sp<SyntaxNode>> body = ParseDirectivePrologues();
        Expect(u'{');

        auto prev_label_set = move(context_.label_set);
        bool prev_in_iteration = context_.in_iteration;
        bool prev_in_switch = context_.in_switch;
        bool prev_in_fun_body = context_.in_function_body;

        context_.label_set = make_unique<unordered_set<UString>>();
        context_.in_iteration = false;
        context_.in_switch = false;
        context_.in_function_body = true;

        while (lookahead_.type_ != JsTokenType::EOF_) {
            if (Match(u'}')) {
                break;
            }

            Sp<SyntaxNode> temp = ParseStatementListItem();
            body.push_back(move(temp));
        }

        Expect('}');

        context_.label_set = move(prev_label_set);
        context_.in_iteration = prev_in_iteration;
        context_.in_switch = prev_in_switch;
        context_.in_function_body = prev_in_fun_body;

        auto node = Alloc<BlockStatement>();
        node->body = move(body);

        return Finalize(start_marker, node);
    }

    std::vector<Sp<SyntaxNode>> Parser::ParseDirectivePrologues() {
        optional<Token> first_restrict;
        std::vector<Sp<SyntaxNode>> result;

        Token token;
        while (true) {
            token = lookahead_;
            if (token.type_ != JsTokenType::StringLiteral) {
                break;
            }

            Sp<Statement> statement = ParseDirective();
            result.push_back(statement);
            if (statement->type != SyntaxNodeType::Directive) {
                break;
            }
            auto directive_ = reinterpret_cast<Directive*>(statement.get());

            if (directive_->directive == u"use strict") {
                context_.strict_ = true;
                if (first_restrict) {
                    TolerateUnexpectedToken(*first_restrict, ParseMessages::StrictOctalLiteral);
                }
                if (!context_.allow_strict_directive) {
                    TolerateUnexpectedToken(token, ParseMessages::IllegalLanguageModeDirective);
                }
            } else {
                if (!first_restrict && token.octal_) {
                    first_restrict = token;
                }
            }
        }

        return result;
    }

    Sp<ClassBody> Parser::ParseClassBody() {
        auto marker = CreateStartMarker();
        auto node = Alloc<ClassBody>();

        node->body = ParseClassElementList();

        return Finalize(marker, node);
    }

    Sp<ClassDeclaration> Parser::ParseClassDeclaration(bool identifier_is_optional) {

        auto marker = CreateStartMarker();

        bool prev_strict = context_.strict_;
        context_.strict_ = true;
        ExpectKeyword(u"class");

        auto node = Alloc<ClassDeclaration>();
        if (identifier_is_optional && (lookahead_.type_ != JsTokenType::Identifier)) {
            // nothing
        } else {
            node->id = ParseVariableIdentifier(VarKind::Invalid);
        }

        if (MatchKeyword(u"extends")) {
            NextToken();
            auto temp = IsolateCoverGrammar<Expression>([this] {
                return ParseLeftHandSideExpressionAllowCall();
            });
            if (temp->type != SyntaxNodeType::Identifier) {
                ThrowUnexpectedToken(Token());
                return nullptr;
            }
            node->super_class = dynamic_pointer_cast<Identifier>(temp);
        }
        node->body = ParseClassBody();
        context_.strict_ = prev_strict;

        return Finalize(marker, node);
    }

    Sp<ClassExpression> Parser::ParseClassExpression() {

        auto marker = CreateStartMarker();
        auto node = Alloc<ClassExpression>();

        bool prev_strict = context_.strict_;
        context_.strict_ = true;
        ExpectKeyword(u"class");

        if (lookahead_.type_ == JsTokenType::Identifier) {
            node->id = ParseVariableIdentifier(VarKind::Invalid);
        }

        if (MatchKeyword(u"extends")) {
            NextToken();
            auto temp = IsolateCoverGrammar<Expression>([this] {
                return ParseLeftHandSideExpressionAllowCall();
            });
            if (temp->type != SyntaxNodeType::Identifier) {
                ThrowUnexpectedToken(Token());
                return nullptr;
            }
            node->super_class = dynamic_pointer_cast<Identifier>(temp);
        }

        node->body = ParseClassBody();
        context_.strict_ = prev_strict;

        return Finalize(marker, node);
    }

    Sp<Expression> Parser::ParseLeftHandSideExpressionAllowCall() {
        Token start_token = lookahead_;
        auto start_marker = CreateStartMarker();
        bool maybe_async = MatchContextualKeyword(u"async");

        bool prev_allow_in = context_.allow_in;
        context_.allow_in = true;

        Sp<Expression> expr;
        if (MatchKeyword(u"super") && context_.in_function_body) {
            auto node = Alloc<Super>();
            auto marker = CreateStartMarker();
            NextToken();
            expr = Finalize(marker, node);
            if (!Match(u'(') && !Match(u'.') && !Match(u'[')) {
                ThrowUnexpectedToken(lookahead_);
                return nullptr;
            }
        } else {
            if (MatchKeyword(u"new")) {
                expr = InheritCoverGrammar<Expression>([this] {
                    return ParseNewExpression();
                });
            } else {
                expr = InheritCoverGrammar<Expression>([this] {
                    return ParsePrimaryExpression();
                });
            }
        }

        while (true) {
            if (Match(u'.')) {
                context_.is_binding_element = false;
                context_.is_assignment_target = true;
                Expect('.');
                auto node = Alloc<StaticMemberExpression>();
                node->property = ParseIdentifierName();
                node->object = expr;
                expr = Finalize(StartNode(start_token), node);
            } else if (Match(u'(')) {
                bool async_arrow = maybe_async && (start_token.line_number_ == lookahead_.line_number_);
                context_.is_binding_element = false;
                context_.is_assignment_target = false;
                auto node = Alloc<CallExpression>();
                if (async_arrow) {
                    node->arguments = ParseAsyncArguments();
                } else {
                    node->arguments = ParseArguments();
                }
                if (expr->type == SyntaxNodeType::Import && node->arguments.size() != 1) {
                    TolerateError(ParseMessages::BadImportCallArity);
                }
                node->callee = expr;
                expr = Finalize(StartNode(start_token), node);
                if (async_arrow && Match(u"=>")) {
                    for (auto &i : node->arguments) {
                        ReinterpretExpressionAsPattern(i);
                    }
                    auto placeholder = Alloc<ArrowParameterPlaceHolder>();
                    placeholder->params = node->arguments;
                    placeholder->async = true;
                    expr = move(placeholder);
                }
            } else if (Match(u'[')) {
                context_.is_binding_element = false;
                context_.is_assignment_target = true;
                Expect(u'[');
                auto node = Alloc<ComputedMemberExpression>();
                node->property = IsolateCoverGrammar<Expression>([this] {
                    return ParseExpression();
                });
                Expect(u']');
                node->object = expr;
                expr = Finalize(StartNode(start_token), node);
            } else if (lookahead_.type_ == JsTokenType::Template && lookahead_.head_) {
                auto node = Alloc<TaggedTemplateExpression>();
                node->quasi = ParseTemplateLiteral();
                node->tag = expr;
                expr = Finalize(StartNode(start_token), node);
            } else {
                break;
            }
        }
        context_.allow_in = prev_allow_in;

        return expr;
    }

    Sp<Expression> Parser::ParseArrayInitializer() {
        auto marker = CreateStartMarker();
        auto node = Alloc<ArrayExpression>();
        Sp<SyntaxNode> element;

        Expect('[');
        while (!Match(']')) {
            if (Match(',')) {
                NextToken();
                node->elements.push_back(nullptr);
            } else if (Match(u"...")) {
                element = ParseSpreadElement();
                if (!Match(']')) {
                    context_.is_assignment_target = false;
                    context_.is_binding_element = false;
                    Expect(',');
                }
                node->elements.push_back(element);
            } else {
                element = InheritCoverGrammar<SyntaxNode>([this] {
                    return ParseAssignmentExpression();
                });
                node->elements.push_back(element);
                if (!Match(']')) {
                    Expect(',');
                }
            }
        }
        Expect(']');

        return Finalize(marker, node);
    }

    Sp<Module> Parser::ParseModule() {
        context_.strict_ = true;
        context_.is_module = true;
        auto marker = CreateStartMarker();
        auto node = make_shared<Module>();
        node->body = ParseDirectivePrologues();
        while (lookahead_.type_ != JsTokenType::EOF_) {
            Sp<SyntaxNode> statement_list_item = ParseStatementListItem();
            node->body.push_back(move(statement_list_item));
        }
        return Finalize(marker, node);
    }

    Sp<SwitchCase> Parser::ParseSwitchCase() {

        auto marker = CreateStartMarker();
        auto node = Alloc<SwitchCase>();

        if (MatchKeyword(u"default")) {
            NextToken();
            node->test.reset();
        } else {
            ExpectKeyword(u"case");
            node->test = ParseExpression();
        }
        Expect(u':');

        while (true) {
            if (Match(u'}') || MatchKeyword(u"default") || MatchKeyword(u"case")) {
                break;
            }
            Sp<Statement> con = ParseStatementListItem();
            node->consequent.push_back(std::move(con));
        }

        return Finalize(marker, node);
    }

    Sp<IfStatement> Parser::ParseIfStatement() {

        auto marker = CreateStartMarker();
        auto node = Alloc<IfStatement>();

        ExpectKeyword(u"if");
        Expect('(');
        node->test = ParseExpression();

        if (!Match(u')') && config_.tolerant) {
            Token token = NextToken();
            ThrowUnexpectedToken(token);
            node->consequent = Finalize(CreateStartMarker(), Alloc<EmptyStatement>());
        } else {
            Expect(u')');
            node->consequent = ParseIfClause();
            if (MatchKeyword(u"else")) {
                NextToken();
                node->alternate = ParseIfClause();
            }
        }

        return Finalize(marker, node);
    }

    Sp<Statement> Parser::ParseIfClause() {
        if (context_.strict_ && MatchKeyword(u"function")) {
            TolerateError(ParseMessages::StrictFunction);
        }
        return ParseStatement();
    }

    Sp<Statement> Parser::ParseStatement() {

        switch (lookahead_.type_) {
            case JsTokenType::BooleanLiteral:
            case JsTokenType::NullLiteral:
            case JsTokenType::NumericLiteral:
            case JsTokenType::StringLiteral:
            case JsTokenType::Template:
            case JsTokenType::RegularExpression:
                return ParseExpressionStatement();

            case JsTokenType::Punctuator: {
                auto& value = lookahead_.value_;
                if (value == u"{") {
                    return ParseBlock();
                } else if (value == u"(") {
                    return ParseExpressionStatement();
                } else if (value == u";") {
                    return ParseEmptyStatement();
                } else {
                    return ParseExpressionStatement();
                }
            }

            case JsTokenType::Identifier: {
                if (MatchAsyncFunction()) {
                    return ParseFunctionDeclaration(false);
                } else {
                    return ParseLabelledStatement();
                }
            }

            case JsTokenType::Keyword: {
                auto& value = lookahead_.value_;
                if (value == u"break") {
                    return ParseBreakStatement();
                } else if (value == u"continue") {
                    return ParseContinueStatement();
                } else if (value == u"debugger") {
                    return ParseDebuggerStatement();
                } else if (value == u"do") {
                    return ParseDoWhileStatement();
                } else if (value == u"for") {
                    return ParseForStatement();
                } else if (value == u"function") {
                    return ParseFunctionDeclaration(false);
                } else if (value == u"if") {
                    return ParseIfStatement();
                } else if (value == u"return") {
                    return ParseReturnStatement();
                } else if (value == u"switch") {
                    return ParseSwitchStatement();
                } else if (value == u"throw") {
                    return ParseThrowStatement();
                } else if (value == u"try") {
                    return ParseTryStatement();
                } else if (value == u"var") {
                    return ParseVariableStatement();
                } else if (value == u"while") {
                    return ParseWhileStatement();
                } else if (value == u"with") {
                    return ParseWithStatement();
                } else {
                    return ParseExpressionStatement();
                }
            }

            default:
                ThrowUnexpectedToken(lookahead_);
                return nullptr;

        }
    }

    Sp<ExpressionStatement> Parser::ParseExpressionStatement() {
        auto marker = CreateStartMarker();
        auto node = Alloc<ExpressionStatement>();

        node->expression  = ParseExpression();
        ConsumeSemicolon();

        return Finalize(marker, node);
    }

    Sp<BlockStatement> Parser::ParseBlock() {
        auto marker = CreateStartMarker();
        auto node = Alloc<BlockStatement>();

        Expect(u'{');
        while (true) {
            if (Match(u'}')) {
                break;
            }
            Sp<SyntaxNode> stmt = ParseStatementListItem();
            node->body.push_back(std::move(stmt));
        }
        Expect(u'}');

        return Finalize(marker, node);
    }

    Sp<EmptyStatement> Parser::ParseEmptyStatement() {
        auto node = Alloc<EmptyStatement>();
        auto marker = CreateStartMarker();
        Expect(u';');
        return Finalize(marker, node);
    }

    Sp<Declaration> Parser::ParseFunctionDeclaration(bool identifier_is_optional) {
        auto marker = CreateStartMarker();

        bool is_async = MatchContextualKeyword(u"async");
        if (is_async) {
            NextToken();
        }

        ExpectKeyword(u"function");

        bool is_generator = is_async ? false : Match(u'*');
        if (is_generator) {
            NextToken();
        }

        optional<Sp<Identifier>> id;
        optional<Token> first_restricted;
        string message;

        if (!identifier_is_optional || !Match(u'(')) {
            Token token = lookahead_;
            id = ParseVariableIdentifier(VarKind::Invalid);
            if (context_.strict_) {
                if (scanner_->IsRestrictedWord(token.value_)) {
                    TolerateUnexpectedToken(token, ParseMessages::StrictFunctionName);
                }
            } else {
                if (scanner_->IsRestrictedWord(token.value_)) {
                    first_restricted = token;
                    message = ParseMessages::StrictFunctionName;
                } else {
                    first_restricted = token;
                    message = ParseMessages::StrictReservedWord;
                }
            }
        }

        bool prev_allow_await = context_.await;
        bool prev_allow_yield = context_.allow_yield;
        context_.await = is_async;
        context_.allow_yield = !is_generator;

        FormalParameterOptions options;
        ParseFormalParameters(first_restricted, options);
        first_restricted = options.first_restricted;
        if (!options.message.empty()) {
            message = move(options.message);
        }

        bool prev_strict = context_.strict_;
        bool prev_allow_strict_directive = context_.allow_strict_directive;
        context_.allow_strict_directive = options.simple;

        // TODO: pasre function source elements

        if (context_.strict_ && first_restricted) {
            Token temp = *first_restricted;
            ThrowUnexpectedToken(temp, message);
        }
        if (context_.strict_ && options.stricted) {
            Token temp = *options.stricted;
            TolerateUnexpectedToken(temp, message);
        }

        context_.strict_ = prev_strict;
        context_.allow_strict_directive = prev_allow_strict_directive;
        context_.await = prev_allow_await;
        context_.allow_yield = prev_allow_yield;

        if (is_async) {
            auto node = Alloc<AsyncFunctionDeclaration>();
            node->id = id;
            return Finalize(marker, node);
        } else {
            auto node = Alloc<FunctionDeclaration>();
            node->id = id;
            node->generator = is_generator;

            return Finalize(marker, node);
        }
    }

    Sp<Statement> Parser::ParseLabelledStatement() {
        auto start_marker = CreateStartMarker();
        Sp<Expression> expr = ParseExpression();

        Sp<Statement> statement;
        if ((expr->type == SyntaxNodeType::Identifier) && Match(u':')) {
            NextToken();

            auto id = dynamic_pointer_cast<Identifier>(expr);
            UString key = UString(u"$") + id->name;

            // TODO: label set

            Sp<Statement> body;

            if (MatchKeyword(u"class")) {
                TolerateUnexpectedToken(lookahead_);
                body = ParseClassDeclaration(false);
            } else if (MatchKeyword(u"function")) {
                Token token = lookahead_;
                Sp<Declaration> declaration = ParseFunctionDeclaration(false);
                // TODO: check generator
//                if (context_.strict_) {
//                    string message = "StrictFunction";
//                    UnexpectedToken(&token, &message);
//                } else if (declaration->generator) {
//                    string message = "GeneratorInLegacyContext";
//                    UnexpectedToken(&token, &message);
//                }
                body = move(declaration);
            } else {
                body = ParseStatement();
            }

            auto node = Alloc<LabeledStatement>();
            node->label = id;
            node->body = body;

            statement = move(node);
        } else {
            ConsumeSemicolon();
            auto node = Alloc<ExpressionStatement>();
            node->expression = expr;
            statement = move(node);
        }
        return Finalize(start_marker, statement);
    }

    Sp<BreakStatement> Parser::ParseBreakStatement() {

        auto marker = CreateStartMarker();
        ExpectKeyword(u"break");

        std::optional<Sp<Identifier>> label;
        if (lookahead_.type_ == JsTokenType::Identifier && !has_line_terminator_) {
            Sp<Identifier> id = ParseVariableIdentifier(VarKind::Invalid);

            UString key = UString(u"$") + id->name;
            // TODO: labelSet
            label = id;
        }

        ConsumeSemicolon();

        if (!label && !context_.in_iteration && !context_.in_switch) {
            ThrowError(ParseMessages::IllegalBreak);
            return nullptr;
        }

        auto node = Alloc<BreakStatement>();
        node->label = label;
        return Finalize(marker, node);
    }

    Sp<ContinueStatement> Parser::ParseContinueStatement() {
        auto marker = CreateStartMarker();
        ExpectKeyword(u"continue");
        auto node = Alloc<ContinueStatement>();

        if (lookahead_.type_ == JsTokenType::Identifier && !has_line_terminator_) {
            // TODO: label
            node->label = ParseVariableIdentifier(VarKind::Invalid);
        }

        ConsumeSemicolon();

        if (!node->label && !context_.in_iteration) {
            ThrowError(ParseMessages::IllegalContinue);
            return nullptr;
        }

        return Finalize(marker, node);
    }

    Sp<DebuggerStatement> Parser::ParseDebuggerStatement() {
        auto marker = CreateStartMarker();
        ExpectKeyword(u"keyword");
        ConsumeSemicolon();
        auto node = Alloc<DebuggerStatement>();
        return Finalize(marker, node);
    }

    Sp<DoWhileStatement> Parser::ParseDoWhileStatement() {
        auto marker = CreateStartMarker();
        auto node = Alloc<DoWhileStatement>();
        ExpectKeyword(u"do");

        auto previous_in_interation = context_.in_iteration;
        context_.in_iteration = true;
        node->body = ParseStatement();
        context_.in_iteration = previous_in_interation;

        ExpectKeyword(u"while");
        Expect(u'(');
        node->test = ParseExpression();

        if (!Match(u')') && config_.tolerant) {
            ThrowUnexpectedToken(NextToken());
        } else {
            Expect(u'(');
            if (Match(u';')) {
                NextToken();
            }
        }

        return Finalize(marker, node);
    }

    Sp<Statement> Parser::ParseForStatement() {
        bool for_in = true;

        std::optional<Sp<SyntaxNode>> init;
        std::optional<Sp<Expression>> test;
        std::optional<Sp<Expression>> update;
        Sp<SyntaxNode> left;
        Sp<SyntaxNode> right;
        auto marker = CreateStartMarker();

        ExpectKeyword(u"for");
        Expect(u'(');

        if (Match(u';')) {
            NextToken();
        } else {
            if (MatchKeyword(u"var")) {
                auto marker = CreateStartMarker();
                NextToken();

                auto prev_allow_in = context_.allow_in;
                context_.allow_in = true;
                std::vector<Sp<VariableDeclarator>> declarations = ParseVariableDeclarationList(for_in);
                context_.allow_in = prev_allow_in;

                if (declarations.size() == 1 && MatchKeyword(u"in")) {
                    auto decl = declarations[0];
                    if (decl->init && (decl->id->type == SyntaxNodeType::ArrayPattern || decl->id->type == SyntaxNodeType::ObjectPattern || context_.strict_)) {
                        TolerateError(ParseMessages::ForInOfLoopInitializer);
                    }
                    auto node = Alloc<VariableDeclaration>();
                    node->kind = VarKind::Var;
                    init = Finalize(marker, node);
                    NextToken();
                    left = *init;
                    right = ParseExpression();
                    init.reset();
                } else if (declarations.size() == 1 && !declarations[0]->init && MatchKeyword(u"of")) {
                    auto node = Alloc<VariableDeclaration>();
                    node->declarations = declarations;
                    node->kind = VarKind::Var;
                    init = Finalize(marker, node);
                    NextToken();
                    left = *init;
                    right = ParseAssignmentExpression();
                    init.reset();
                    for_in = false;
                } else {
                    auto node = Alloc<VariableDeclaration>();
                    node->declarations = declarations;
                    node->kind = VarKind::Var;
                    init = Finalize(marker, node);
                    Expect(u';');
                }
            } else if (MatchKeyword(u"const") || MatchKeyword(u"let")) {
                auto marker = CreateStartMarker();
                Token token = NextToken();
                VarKind kind;
                if (token.value_ == u"const") {
                    kind = VarKind::Const;
                } else if (token.value_ == u"let") {
                    kind = VarKind::Let;
                }

                if (!context_.strict_ && lookahead_.value_ == u"in") {
                    auto node = Alloc<Identifier>();
                    node->name = token.value_;
                    init = Finalize(marker, node);
                    NextToken();
                    left = *init;
                    right = ParseExpression();
                    init.reset();
                } else {
                    auto prev_allow_in = context_.allow_in;
                    context_.allow_in = false;
                    std::vector<Sp<VariableDeclarator>> declarations = ParseBindingList(VarKind::Const);
                    context_.allow_in = prev_allow_in;

                    if (declarations.size() == 1 && !declarations[0]->init && MatchKeyword(u"in")) {
                        auto node = Alloc<VariableDeclaration>();
                        node->declarations = declarations;
                        node->kind = kind;
                        init = Finalize(marker, node);
                        NextToken();
                        left = *init;
                        right = ParseExpression();
                        init.reset();
                    } else if (declarations.size() == 1 && !declarations[0]->init && MatchContextualKeyword(u"of")) {
                        auto node = Alloc<VariableDeclaration>();
                        node->declarations = declarations;
                        node->kind = kind;
                        init = Finalize(marker, node);
                        NextToken();
                        left = *init;
                        right = ParseAssignmentExpression();
                        init.reset();
                        for_in = false;
                    } else {
                        ConsumeSemicolon();
                        auto node = Alloc<VariableDeclaration>();
                        node->declarations = declarations;
                        node->kind = kind;
                        init = Finalize(marker, node);
                    }
                }
            } else {
                auto init_start_token = lookahead_;
                auto start_marker = CreateStartMarker();
                auto prev_allow_in = context_.allow_in;
                context_.allow_in = false;
                init = InheritCoverGrammar<Expression>([this] {
                    return ParseAssignmentExpression();
                });
                context_.allow_in = prev_allow_in;

                if (MatchKeyword(u"in")) {
                    if (!context_.is_assignment_target || (*init)->type == SyntaxNodeType::AssignmentExpression) {
                        TolerateError(ParseMessages::InvalidLHSInForIn);
                    }

                    NextToken();
                    ReinterpretExpressionAsPattern(*init);
                    left = *init;
                    right = ParseExpression();
                    init.reset();
                } else if (MatchContextualKeyword(u"of")) {
                    if (!context_.is_assignment_target || (*init)->type == SyntaxNodeType::AssignmentExpression) {
                        TolerateError(ParseMessages::InvalidLHSInForIn);
                    }

                    NextToken();
                    ReinterpretExpressionAsPattern(*init);
                    left = *init;
                    right = ParseAssignmentExpression();
                    init.reset();
                    for_in = false;
                } else {
                    if (Match(u',')) {
                        std::vector<Sp<SyntaxNode>> init_seq;
                        init_seq.push_back(*init);

                        while (Match(u',')) {
                            NextToken();
                            Sp<Expression> node = IsolateCoverGrammar<Expression>([this] {
                                return ParseAssignmentExpression();
                            });
                            init_seq.push_back(node);
                        }

                        Sp<SequenceExpression> node;
                        // TODO: ???
                        init = Finalize(start_marker, node);
                    }
                    Expect(u';');
                }
            }
        }

        if (!left) {
            if (!Match(u';')) {
                test = ParseExpression();
            }
            Expect(u';');
            if (!Match(u')')) {
                update = ParseExpression();
            }
        }

        Sp<Statement> body;
        if (!Match(u')') && config_.tolerant) {
            TolerateUnexpectedToken(NextToken());
            auto node = Alloc<EmptyStatement>();
            body = Finalize(CreateStartMarker(), node);
        } else {
            Expect(u')');

            auto prev_in_iter = context_.in_iteration;
            context_.in_iteration = true;
            body = IsolateCoverGrammar<Statement>([this] {
                return ParseStatement();
            });
            context_.in_iteration = prev_in_iter;
        }

        if (!left) {
            auto node = Alloc<ForStatement>();
            node->init = init;
            node->test = test;
            node->update = update;
            node->body = body;
            return Finalize(marker, node);
        } else if (for_in) {
            auto node = Alloc<ForInStatement>();
            node->left = left;
            node->right = right;
            node->body = body;
            return Finalize(marker, node);
        } else {
            auto node = Alloc<ForOfStatement>();
            node->left = left;
            node->right = right;
            node->body = body;
            return Finalize(marker, node);
        }

    }

    Sp<ReturnStatement> Parser::ParseReturnStatement() {
        if (!context_.in_function_body) {
            TolerateError(ParseMessages::IllegalReturn);
        }

        auto marker = CreateStartMarker();
        ExpectKeyword(u"return");

        bool has_arg = (!Match(u';') && !Match(u'}') &&
                        !has_line_terminator_ && lookahead_.type_ != JsTokenType::EOF_) ||
                       lookahead_.type_ == JsTokenType::StringLiteral ||
                       lookahead_.type_ == JsTokenType::Template;

        auto node = Alloc<ReturnStatement>();
        if (has_arg) {
            node->argument = ParseExpression();
        }

        ConsumeSemicolon();

        return Finalize(marker, node);
    }

    Sp<SwitchStatement> Parser::ParseSwitchStatement() {
        auto marker = CreateStartMarker();
        ExpectKeyword(u"switch");
        auto node = Alloc<SwitchStatement>();

        Expect(u'(');
        node->discrimiant = ParseExpression();
        Expect(u')');

        auto prev_in_switch = context_.in_switch;
        context_.in_switch = true;

        bool default_found = false;
        Expect(u'{');
        while (true) {
            if (Match(u'}')) {
                break;
            }
            Sp<SwitchCase> clause = ParseSwitchCase();
            if (!clause->test) {
                if (default_found) {
                    ThrowError(ParseMessages::MultipleDefaultsInSwitch);
                    return nullptr;
                }
                default_found = true;
            }
            node->cases.push_back(clause);
        }
        Expect(u'}');

        context_.in_switch = prev_in_switch;

        return Finalize(marker, node);
    }

    Sp<ThrowStatement> Parser::ParseThrowStatement() {
        auto marker = CreateStartMarker();
        ExpectKeyword(u"throw");

        if (has_line_terminator_) {
            ThrowError(ParseMessages::NewlineAfterThrow);
            return nullptr;
        }

        auto node = Alloc<ThrowStatement>();
        node->argument = ParseExpression();
        ConsumeSemicolon();

        return Finalize(marker, node);
    }

    Sp<TryStatement> Parser::ParseTryStatement() {
        auto marker = CreateStartMarker();
        ExpectKeyword(u"try");
        auto node = Alloc<TryStatement>();

        node->block = ParseBlock();
        if (MatchKeyword(u"catch")) {
            node->handler = ParseCatchClause();
        }
        if (MatchKeyword(u"finally")) {
            node->finalizer = ParseFinallyClause();
        }

        return Finalize(marker, node);
    }

    Sp<CatchClause> Parser::ParseCatchClause() {
        auto marker = CreateStartMarker();

        ExpectKeyword(u"catch");

        Expect(u'(');
        if (Match(u')')) {
            ThrowUnexpectedToken(lookahead_);
            return nullptr;
        }

        auto node = Alloc<CatchClause>();

        // TODO: parse params

        Expect(u')');
        node->body = ParseBlock();

        return Finalize(marker, node);
    }

    Sp<BlockStatement> Parser::ParseFinallyClause() {
        ExpectKeyword(u"finally");
        return ParseBlock();
    }

    Sp<VariableDeclaration> Parser::ParseVariableStatement() {
        auto marker = CreateStartMarker();
        ExpectKeyword(u"var");

        auto node = Alloc<VariableDeclaration>();
        node->declarations = ParseVariableDeclarationList(false);
        ConsumeSemicolon();

        node->kind = VarKind::Var;
        return Finalize(marker, node);
    }

    Sp<VariableDeclarator> Parser::ParseVariableDeclaration(bool in_for) {
        auto marker = CreateStartMarker();
        auto node = Alloc<VariableDeclarator>();

        vector<Token> params;
        Sp<SyntaxNode> id = ParsePattern(params, VarKind::Var);

        if (context_.strict_ && id->type == SyntaxNodeType::Identifier) {
            auto identifier = dynamic_pointer_cast<Identifier>(id);
            if (scanner_->IsRestrictedWord(identifier->name)) {
                TolerateError(ParseMessages::StrictVarName);
            }
        }

        optional<Sp<Expression>> init;
        if (Match(u'=')) {
            NextToken();
            init = IsolateCoverGrammar<Expression>([this] {
                return ParseAssignmentExpression();
            });
        } else if (id->type != SyntaxNodeType::Identifier && !in_for) {
            Expect(u'=');
        }

        node->id = move(id);
        node->init = init;

        return Finalize(marker, node);
    }

    std::vector<Sp<VariableDeclarator>> Parser::ParseVariableDeclarationList(bool in_for) {
        // TODO:
        return {};
    }

    Sp<WhileStatement> Parser::ParseWhileStatement() {
        auto marker = CreateStartMarker();
        auto node = Alloc<WhileStatement>();

        ExpectKeyword(u"while");
        Expect(u'(');
        node->test = ParseExpression();

        if (!Match(u')') && config_.tolerant) {
            TolerateUnexpectedToken(NextToken());
            node->body = Finalize(CreateStartMarker(), Alloc<EmptyStatement>());
        } else {
            Expect(u')');

            auto prev_in_interation = context_.in_iteration;
            context_.in_iteration = true;
            node->body = ParseStatement();
            context_.in_iteration = prev_in_interation;
        }

        return Finalize(marker, node);
    }

    Sp<WithStatement> Parser::ParseWithStatement() {
        if (context_.strict_) {
            TolerateError(ParseMessages::StrictModeWith);
        }

        auto marker = CreateStartMarker();
        auto node = Alloc<WithStatement>();

        ExpectKeyword(u"with");
        Expect(u'(');

        node->object = ParseExpression();

        if (!Match(u')') && config_.tolerant) {
            TolerateUnexpectedToken(NextToken());
            auto empty = Alloc<EmptyStatement>();
            node->body = Finalize(marker, empty);
        } else {
            Expect(u')');
            node->body = ParseStatement();
        }

        return Finalize(marker, node);
    }

    std::vector<Sp<VariableDeclarator>> Parser::ParseBindingList(VarKind kind) {
        // TODO:
        return {};
    }

    Sp<RestElement> Parser::ParseRestElement(std::vector<Token> &params) {
        auto marker = CreateStartMarker();
        auto node = Alloc<RestElement>();

        Expect(u"...");
        node->argument = ParsePattern(params, VarKind::Invalid);
        if (Match('=')) {
            ThrowError(ParseMessages::DefaultRestParameter);
            return nullptr;
        }
        if (!Match(')')) {
            ThrowError(ParseMessages::ParameterAfterRestParameter);
            return nullptr;
        }

        return Finalize(marker, node);
    }

    Sp<Statement> Parser::ParseStatementListItem() {
        Sp<Statement> statement;
        context_.is_assignment_target = true;
        context_.is_binding_element = true;

        if (lookahead_.type_ == JsTokenType::Keyword) {
            if (lookahead_.value_ == u"export") {
                if (context_.is_module) {
                    TolerateUnexpectedToken(lookahead_, ParseMessages::IllegalExportDeclaration);
                }
                statement = ParseExportDeclaration();
            } else if (lookahead_.value_ == u"import") {
                if (MatchImportCall()) {
                    statement = ParseExpressionStatement();
                } else {
                    if (!context_.is_module) {
                        TolerateUnexpectedToken(lookahead_, ParseMessages::IllegalImportDeclaration);
                    }
                    statement = ParseImportDeclaration();
                }
            } else if (lookahead_.value_ == u"const") {
                bool in_for = false;
                statement = ParseLexicalDeclaration(in_for);
            } else if (lookahead_.value_ == u"function") {
                statement = ParseFunctionDeclaration(false);
            } else if (lookahead_.value_ == u"class") {
                statement = ParseClassDeclaration(false);
            } else if (lookahead_.value_ == u"let") {
                if (IsLexicalDeclaration()) {
                    bool in_for = false;
                    statement = ParseLexicalDeclaration(in_for);
                } else {
                    statement = ParseStatement();
                }
            } else {
                statement = ParseStatement();
            }
        } else {
            statement = ParseStatement();
        }

        return statement;
    }

    Sp<ExportSpecifier> Parser::ParseExportSpecifier() {
        auto start_marker = CreateStartMarker();
        auto node = Alloc<ExportSpecifier>();

        node->local = ParseIdentifierName();
        node->exported = node->local;
        if (MatchContextualKeyword(u"as")) {
            NextToken();
            node->exported = ParseIdentifierName();
        }

        return Finalize(start_marker, node);
    }

    Sp<Declaration> Parser::ParseExportDeclaration() {
        if (context_.in_function_body) {
            ThrowError(ParseMessages::IllegalExportDeclaration);
        }

        auto start_marker = CreateStartMarker();
        ExpectKeyword(u"export");

        Sp<Declaration> export_decl;
        if (MatchKeyword(u"default")) {
            NextToken();
            if (MatchKeyword(u"function")) {
                auto node = Alloc<ExportDefaultDeclaration>();
                node->declaration = ParseFunctionDeclaration(true);
                export_decl = Finalize(start_marker, node);
            } else if (MatchKeyword(u"class")) {
                auto node = Alloc<ExportDefaultDeclaration>();
                node->declaration = ParseClassExpression();
                export_decl = Finalize(start_marker, node);
            } else if (MatchContextualKeyword(u"async")) {
                auto node = Alloc<ExportDefaultDeclaration>();
                if (MatchAsyncFunction()) {
                    node->declaration = ParseFunctionDeclaration(true);
                } else {
                    node->declaration = ParseAssignmentExpression();
                }
                export_decl = Finalize(start_marker, node);
            } else {
                if (MatchContextualKeyword(u"from")) {
                    ThrowError(ParseMessages::UnexpectedToken, utils::To_UTF8(lookahead_.value_));
                }
                Sp<SyntaxNode> decl;
                if (Match(u'{')) {
                    decl = ParseObjectInitializer();
                } else if (Match(u'[')) {
                    decl = ParseArrayInitializer();
                } else {
                    decl = ParseAssignmentExpression();
                }
                ConsumeSemicolon();
                auto node = Alloc<ExportDefaultDeclaration>();
                node->declaration = move(decl);
                export_decl = Finalize(start_marker, node);
            }

        } else if (Match(u'*')) {
            NextToken();
            if (!MatchContextualKeyword(u"from")) {
                string message;
                if (!lookahead_.value_.empty()) {
                    message = ParseMessages::UnexpectedToken;
                } else {
                    message = ParseMessages::MissingFromClause;
                }
                ThrowError(message, utils::To_UTF8(lookahead_.value_));
            }
            NextToken();
            auto node = Alloc<ExportAllDeclaration>();
            node->source = ParseModuleSpecifier();
            ConsumeSemicolon();
            export_decl = Finalize(start_marker, node);
        } else if (lookahead_.type_ == JsTokenType::Keyword) {
            auto node = Alloc<ExportNamedDeclaration>();

            if (lookahead_.value_ == u"let" || lookahead_.value_ == u"const") {
                bool in_for = false;
                node->declaration = ParseLexicalDeclaration(in_for);
            } else if (lookahead_.value_ == u"var" || lookahead_.value_ == u"class" || lookahead_.value_ == u"function") {
                node->declaration = ParseStatementListItem();
            } else {
                ThrowUnexpectedToken(lookahead_);
            }

            export_decl = Finalize(start_marker, node);

        } else if (MatchAsyncFunction()) {
            auto node = Alloc<ExportNamedDeclaration>();
            node->declaration = ParseFunctionDeclaration(false);
            export_decl = Finalize(start_marker, node);
        } else {
            auto node = Alloc<ExportNamedDeclaration>();
            bool is_export_from_id = false;

            Expect(u'{');
            while (!Match(u'}')) {
                is_export_from_id = is_export_from_id || MatchKeyword(u"default");
                node->specifiers.push_back(ParseExportSpecifier());
                if (!Match(u'}')) {
                    Expect(u',');
                }
            }
            Expect(u'}');

            if (MatchContextualKeyword(u"from")) {
                NextToken();
                node->source = ParseModuleSpecifier();
                ConsumeSemicolon();
            } else if (is_export_from_id) {
                string message;
                if (!lookahead_.value_.empty()) {
                    message = ParseMessages::UnexpectedToken;
                } else {
                    message = ParseMessages::MissingFromClause;
                }
                ThrowError(message, utils::To_UTF8(lookahead_.value_));
            } else {
                ConsumeSemicolon();
            }

            export_decl = Finalize(start_marker, node);
        }

        return export_decl;
    }

    Sp<Expression> Parser::ParseAssignmentExpression() {
        Sp<Expression> expr;

        if (context_.allow_yield && MatchKeyword(u"yield")) {
            expr = ParseYieldExpression();
        } else {
            auto start_marker = CreateStartMarker();
            auto token = lookahead_;
            expr = ParseConditionalExpression();

            if (token.type_ == JsTokenType::Identifier && (token.line_number_ == lookahead_.line_number_) && token.value_ == u"async") {
                if (lookahead_.type_ == JsTokenType::Identifier || MatchKeyword(u"yield")) {
                    Sp<Expression> arg =ParsePrimaryExpression();
                    ReinterpretExpressionAsPattern(arg);

                    auto node = Alloc<ArrowParameterPlaceHolder>();
                    node->params.push_back(arg);
                    node->async = true;
                    expr = move(node);

                }
            }

            if (expr->type == SyntaxNodeType::ArrowParameterPlaceHolder || Match(u"=>")) {

                auto placefolder = dynamic_pointer_cast<ArrowParameterPlaceHolder>(expr);
                context_.is_assignment_target = false;
                context_.is_binding_element = false;
                bool is_async = placefolder->async;

                FormalParameterOptions list;
                if (ReinterpretAsCoverFormalsList(expr, list)) {
                    if (has_line_terminator_) {
                        TolerateUnexpectedToken(lookahead_);
                    }
                    context_.first_cover_initialized_name_error.reset();

                    bool prev_strict = context_.strict_;
                    bool prev_allow_strict_directive = context_.allow_strict_directive;
                    context_.allow_strict_directive = list.simple;

                    bool prev_allow_yield = context_.allow_yield;
                    bool prev_await = context_.await;
                    context_.allow_yield = true;
                    context_.await = is_async;

                    auto marker = CreateStartMarker();
                    Expect(u"=>");
                    Sp<SyntaxNode> body;

                    if (Match(u'{')) {
                        bool prev_allow_in = context_.allow_in;
                        context_.allow_in = true;
                        body = ParseFunctionSourceElements();
                        context_.allow_in = prev_allow_in;
                    } else {
                        body = IsolateCoverGrammar<SyntaxNode>([this] {
                            return ParseAssignmentExpression();
                        });
                    }

                    bool expression = body->type != SyntaxNodeType::BlockStatement;

                    if (context_.strict_ && list.first_restricted) {
                        Token temp = *list.first_restricted;
                        ThrowUnexpectedToken(temp, list.message);
                        return nullptr;
                    }
                    if (context_.strict_ && list.stricted) {
                        Token temp = *list.stricted;
                        TolerateUnexpectedToken(temp, list.message);
                    }

                    if (is_async) {
                        auto node = Alloc<AsyncArrowFunctionExpression>();
                        node->params = list.params;
                        node->body = body;
                        node->expression = expression;
                        expr = Finalize(marker, node);
                    } else {
                        auto node = Alloc<ArrowFunctionExpression>();
                        node->params = list.params;
                        node->body = body;
                        node->expression = expression;
                        expr = Finalize(marker, node);
                    }

                    context_.strict_ = prev_strict;
                    context_.allow_strict_directive = prev_allow_strict_directive;
                    context_.allow_yield = prev_allow_yield;
                    context_.await = prev_await;
                }
            } else {

                if (MatchAssign()) {
                    if (!context_.is_assignment_target) {
                        TolerateError(ParseMessages::InvalidLHSInAssignment);
                    }

                    if (context_.strict_ && expr->type == SyntaxNodeType::Identifier) {
                        auto id = dynamic_pointer_cast<Identifier>(expr);
                        if (scanner_->IsRestrictedWord(id->name)) {
                            TolerateUnexpectedToken(token, ParseMessages::StrictLHSAssignment);
                        }
                        if (scanner_->IsStrictModeReservedWord(id->name)) {
                            TolerateUnexpectedToken(token, ParseMessages::StrictReservedWord);
                        }
                    }

                    if (!Match(u'=')) {
                        context_.is_assignment_target = false;
                        context_.is_binding_element = false;
                    } else {
                        ReinterpretExpressionAsPattern(expr);
                    }

                    token = NextToken();
                    auto operator_ = token.value_;
                    Sp<Expression> right = IsolateCoverGrammar<Expression>([this] {
                        return ParseAssignmentExpression();
                    });
                    auto temp = Alloc<AssignmentExpression>();
                    temp->operator_ = operator_;
                    temp->left = expr;
                    temp->right = right;
                    expr = Finalize(start_marker, temp);
                    context_.first_cover_initialized_name_error.reset();
                }
            }

        }

        // TODO: wait to complete
        return expr;
    }

    Sp<Expression> Parser::ParseConditionalExpression() {
        auto marker = CreateStartMarker();

        Sp<Expression> expr = InheritCoverGrammar<Expression>([this] {
            return ParseBinaryExpression();
        });

        if (Match(u'?')) {
            NextToken();
            auto node = Alloc<ConditionalExpression>();

            bool prev_allow_in = context_.allow_in;
            context_.allow_in = true;

            node->consequent = IsolateCoverGrammar<Expression>([this] {
                return ParseAssignmentExpression();
            });
            context_.allow_in = prev_allow_in;

            Expect(u':');
            node->alternate = IsolateCoverGrammar<Expression>([this] {
                return ParseAssignmentExpression();
            });

            expr = move(node);

            context_.is_assignment_target = false;
            context_.is_binding_element = false;
        }

        return Finalize(marker, expr);
    }

    Sp<Expression> Parser::ParseBinaryExpression() {
        auto start_token = lookahead_;

        Sp<Expression> expr = InheritCoverGrammar<Expression>([this] {
            return ParseExponentiationExpression();
        });

        Token token = lookahead_;
        int prec = BinaryPrecedence(token);
        if (prec > 0) {
            NextToken();

            context_.is_assignment_target = false;
            context_.is_binding_element = false;

            stack<Token> markers;
            markers.push(start_token);
            markers.push(lookahead_);

            Sp<Expression> left = expr;
            Sp<Expression> right = IsolateCoverGrammar<Expression>([this] {
                return ParseExponentiationExpression();
            });

            stack<int> precedences;
            stack<Sp<Expression>> expr_stack;
            stack<UString> op_stack;

            op_stack.push(token.value_);
            expr_stack.push(left);
            expr_stack.push(right);
            precedences.push(prec);

            while (true) {
                prec = BinaryPrecedence(lookahead_);
                if (prec <= 0) break;

                // reduce
                while ((expr_stack.size() >= 2) && (prec <= precedences.top())) {
                    right = expr_stack.top();
                    expr_stack.pop();

                    auto operator_ = move(op_stack.top());
                    op_stack.pop();

                    left = expr_stack.top();
                    expr_stack.pop();

                    markers.pop();

                    auto node = Alloc<BinaryExpression>();
                    node->operator_ = operator_;
                    node->left = left;
                    node->right = right;
                    expr_stack.push(move(node));
                }

                // shift
                Token next_ = NextToken();
                op_stack.push(next_.value_);
                precedences.push(prec);
                markers.push(lookahead_);
                Sp<Expression> temp_expr = IsolateCoverGrammar<Expression>([this] {
                    return ParseExponentiationExpression();
                });
                expr_stack.push(temp_expr);
            }

            expr = expr_stack.top();

            auto last_marker = markers.top();
            markers.pop();
            while (!expr_stack.empty()) {
                auto marker = markers.top();
                markers.pop();
                auto last_line_start = last_marker.line_start_;
                auto operator_ = move(op_stack.top());
                op_stack.pop();
                auto node = Alloc<BinaryExpression>();
                node->operator_ = operator_;
                node->left = expr_stack.top();
                expr_stack.pop();
                node->right = expr;
                expr = Finalize(CreateStartMarker(), node);
                last_marker = marker;
            }
        }

        return expr;
    }

    Sp<Expression> Parser::ParseExponentiationExpression() {
        auto start = CreateStartMarker();

        Sp<Expression> expr = InheritCoverGrammar<Expression>([this] {
            return ParseUnaryExpression();
        });

        if (expr->type != SyntaxNodeType::UnaryExpression && Match(u"**")) {
            NextToken();
            context_.is_assignment_target = false;
            context_.is_binding_element = false;
            auto node = Alloc<BinaryExpression>();
            node->left = expr;
            node->right = IsolateCoverGrammar<Expression>([this, &node] {
                return ParseExponentiationExpression();
            });
            node->operator_ = u"**";
            expr = Finalize(start, node);
        }

        return expr;
    }

    Sp<Expression> Parser::ParseUnaryExpression() {
        Sp<Expression> expr;

        if (
            Match(u'+') || Match(u'-') || Match(u'~') || Match(u'!') ||
            MatchKeyword(u"delete") || MatchKeyword(u"void") || MatchKeyword(u"typeof")
            ) {
            auto marker = CreateStartMarker();
            Token token = lookahead_;
            NextToken();
            expr = InheritCoverGrammar<Expression>([this] {
                return ParseUnaryExpression();
            });
            auto node = Alloc<UnaryExpression>();
            node->operator_ = token.value_;
            node->argument = expr;
            expr = Finalize(marker, node);
            if (context_.strict_ && node->operator_ == u"delete" && node->argument->type == SyntaxNodeType::Identifier) {
                TolerateError(ParseMessages::StrictDelete);
            }
            context_.is_assignment_target = false;
            context_.is_binding_element = false;
        } else if (context_.await && MatchContextualKeyword(u"await")) {
            expr = ParseAwaitExpression();
        } else {
            expr = ParseUpdateExpression();
        }

        return expr;
    }

    Sp<AwaitExpression> Parser::ParseAwaitExpression() {
        auto marker = CreateStartMarker();
        NextToken();
        auto node = Alloc<AwaitExpression>();
        node->argument = ParseUnaryExpression();
        return Finalize(marker, node);
    }

    Sp<Expression> Parser::ParseUpdateExpression() {
        Sp<Expression> expr;
        auto start_marker = CreateStartMarker();

        if (Match(u"++") || Match(u"--")) {
            auto marker = CreateStartMarker();
            Token token = NextToken();
            expr = InheritCoverGrammar<Expression>([this] {
                return ParseUnaryExpression();
            });
            if (context_.strict_ && expr->type == SyntaxNodeType::Identifier) {
                auto id = dynamic_pointer_cast<Identifier>(expr);
                if (scanner_->IsRestrictedWord(id->name)) {
                    TolerateError(ParseMessages::StrictLHSPrefix);
                }
            }
            if (!context_.is_assignment_target) {
                TolerateError(ParseMessages::InvalidLHSInAssignment);
            }
            auto node = Alloc<UpdateExpression>();
            node->prefix = true;
            node->operator_ = token.value_;
            node->argument = expr;
            expr = Finalize(marker, node);
            context_.is_assignment_target = false;
            context_.is_binding_element = false;
        } else {
            expr = InheritCoverGrammar<Expression>([this] {
                return ParseLeftHandSideExpressionAllowCall();
            });
            if (!has_line_terminator_ && lookahead_.type_ == JsTokenType::Punctuator) {
                if (Match(u"++") || Match(u"--")) {
                    if (context_.strict_ && expr->type == SyntaxNodeType::Identifier) {
                        auto id = dynamic_pointer_cast<Identifier>(expr);
                        if (scanner_->IsRestrictedWord(id->name)) {
                            TolerateError(ParseMessages::StrictLHSPostfix);
                        }
                    }
                    if (!context_.is_assignment_target) {
                        TolerateError(ParseMessages::InvalidLHSInAssignment);
                    }
                    context_.is_assignment_target = false;
                    context_.is_binding_element = false;
                    auto node = Alloc<UpdateExpression>();
                    node->prefix = false;
                    Token token = NextToken();
                    node->operator_ = token.value_;
                    node->argument = expr;
                    expr = Finalize(start_marker, node);
                }
            }
        }

        return expr;
    }

    Sp<Expression> Parser::ParseLeftHandSideExpression() {
        Assert(context_.allow_in, "callee of new expression always allow in keyword.");

        auto start_marker = CreateStartMarker();
        Sp<Expression> expr;
        if (MatchKeyword(u"super") && context_.in_function_body) {
            expr = ParseSuper();
        } else {
            expr = InheritCoverGrammar<Expression>([this] {
                if (MatchKeyword(u"new")) {
                    return ParseNewExpression();
                } else {
                    return ParsePrimaryExpression();
                }
            });
        }

        while (true) {
            if (Match(u'[')) {
                context_.is_binding_element = false;
                context_.is_assignment_target = true;
                Expect(u'[');
                auto node = Alloc<ComputedMemberExpression>();
                node->property = IsolateCoverGrammar<Expression>([this] {
                    return ParseExpression();
                });
                Expect(u']');
                node->object = move(expr);
                expr = Finalize(start_marker, node);
            } else if (Match(u'.')) {
                context_.is_binding_element = false;
                context_.is_assignment_target = true;
                Expect('.');
                auto node = Alloc<StaticMemberExpression>();
                node->property = ParseIdentifierName();
                node->object = move(expr);
                expr = Finalize(start_marker, node);
            } else if (lookahead_.type_ == JsTokenType::Template && lookahead_.head_) {
                auto node = Alloc<TaggedTemplateExpression>();
                node->quasi = ParseTemplateLiteral();
                node->tag = move(expr);
                expr = Finalize(start_marker, node);
            } else {
                break;
            }
        }

        return expr;
    }

    Sp<Super> Parser::ParseSuper() {
        auto start_marker = CreateStartMarker();

        ExpectKeyword(u"super");
        if (!Match(u'[') && !Match(u'.')) {
            ThrowUnexpectedToken(lookahead_);
            return nullptr;
        }

        auto node = Alloc<Super>();
        return Finalize(start_marker, node);
    }

    Sp<ImportDeclaration> Parser::ParseImportDeclaration() {
        if (context_.in_function_body) {
            ThrowError(ParseMessages::IllegalImportDeclaration);
        }

        auto start_marker = CreateStartMarker();
        ExpectKeyword(u"import");

        Sp<Literal> src;
        std::vector<Sp<SyntaxNode>> specifiers;

        if (lookahead_.type_ == JsTokenType::StringLiteral) {
            src = ParseModuleSpecifier();
        } else {
            if (Match(u'{')) {
                auto children = ParseNamedImports();
                specifiers.insert(specifiers.end(), children.begin(), children.end());
            } else if (Match(u'*')) {
                specifiers.push_back(ParseImportNamespaceSpecifier());
            } else if (IsIdentifierName(lookahead_) && !MatchKeyword(u"default")) {
                auto default_ = ParseImportDefaultSpecifier();
                specifiers.push_back(move(default_));

                if (Match(u',')) {
                    NextToken();
                    if (Match(u'*')) {
                        specifiers.push_back(ParseImportNamespaceSpecifier());
                    } else if (Match(u'*')) {
                        auto children = ParseNamedImports();
                        specifiers.insert(specifiers.end(), children.begin(), children.end());
                    } else {
                        ThrowUnexpectedToken(lookahead_);
                    }
                }
            } else {
                ThrowUnexpectedToken(NextToken());
            }

            if (!MatchContextualKeyword(u"from")) {
                string message;
                if (!lookahead_.value_.empty()) {
                    message = ParseMessages::UnexpectedToken;
                } else {
                    message = ParseMessages::MissingFromClause;
                }
                ThrowError(message, utils::To_UTF8(lookahead_.value_));
            }
            NextToken();
            src = ParseModuleSpecifier();
        }
        ConsumeSemicolon();

        auto node = Alloc<ImportDeclaration>();
        node->specifiers = move(specifiers);
        node->source = move(src);

        return Finalize(start_marker, node);
    }

    std::vector<Sp<SyntaxNode>> Parser::ParseNamedImports() {
        Expect(u'{');
        std::vector<Sp<SyntaxNode>> specifiers;
        while (!Match(u'}')) {
            specifiers.push_back(ParseImportSpecifier());
            if (!Match(u'}')) {
                Expect(u',');
            }
        }
        Expect(u'}');

        return specifiers;
    }

    Sp<ImportSpecifier> Parser::ParseImportSpecifier() {
        auto start_marker = CreateStartMarker();

        Sp<Identifier> imported;
        Sp<Identifier> local;
        if (lookahead_.type_ == JsTokenType::Identifier) {
            imported = ParseVariableIdentifier(VarKind::Invalid);
            local = imported;
            if (MatchContextualKeyword(u"as")) {
                NextToken();
                local = ParseVariableIdentifier(VarKind::Invalid);
            }
        } else {
            imported = ParseIdentifierName();
            local = imported;
            if (MatchContextualKeyword(u"as")) {
                NextToken();
                local = ParseVariableIdentifier(VarKind::Invalid);
            } else {
                ThrowUnexpectedToken(NextToken());
            }
        }

        auto node = Alloc<ImportSpecifier>();
        node->local = local;
        node->imported = imported;
        return Finalize(start_marker, node);
    }

    Sp<Literal> Parser::ParseModuleSpecifier() {
        auto start_marker = CreateStartMarker();

        if (lookahead_.type_ != JsTokenType::StringLiteral) {
            ThrowError(ParseMessages::InvalidModuleSpecifier);
        }

        Token token = NextToken();
        auto node = Alloc<Literal>();
        node->value = token.value_;
        node->raw = token.value_;
        return Finalize(start_marker, node);
    }

    Sp<ImportDefaultSpecifier> Parser::ParseImportDefaultSpecifier() {
        auto start_marker = CreateStartMarker();
        auto local = ParseIdentifierName();
        auto node = Alloc<ImportDefaultSpecifier>();
        node->local = move(local);
        return Finalize(start_marker, node);
    }

    Sp<ImportNamespaceSpecifier> Parser::ParseImportNamespaceSpecifier() {
        auto start_marker = CreateStartMarker();

        Expect(u'*');
        if (MatchContextualKeyword(u"as")) {
            ThrowError(ParseMessages::NoAsAfterImportNamespace);
        }
        NextToken();
        auto local = ParseIdentifierName();

        auto node = Alloc<ImportNamespaceSpecifier>();
        node->local = move(local);
        return Finalize(start_marker, node);
    }

    Sp<Declaration> Parser::ParseLexicalDeclaration(bool &in_for) {
        return nullptr;
    }

    Sp<Identifier> Parser::ParseVariableIdentifier(VarKind kind) {
        auto marker = CreateStartMarker();

        Token token = NextToken();
        if (token.type_ == JsTokenType::Keyword && token.value_ == u"yield") {
            if (context_.strict_) {
                TolerateUnexpectedToken(token, ParseMessages::StrictReservedWord);
            } else if (!context_.allow_yield) {
                ThrowUnexpectedToken(token);
                return nullptr;
            }
        } else if (token.type_ == JsTokenType::Identifier) {
            if (context_.strict_ && token.type_ == JsTokenType::Keyword && scanner_->IsStrictModeReservedWord(token.value_)) {
                TolerateUnexpectedToken(token, ParseMessages::StrictReservedWord);
            } else {
                if (context_.strict_ || token.value_ != u"let" || kind != VarKind::Var) {
                    ThrowUnexpectedToken(token);
                    return nullptr;
                }
            }
        } else if ((context_.is_module || context_.await) && token.type_ == JsTokenType::Identifier && token.value_ == u"await") {
            TolerateUnexpectedToken(token);
        }

        auto node = Alloc<Identifier>();
        node->name = token.value_;
        return Finalize(marker, node);
    }

    Sp<SyntaxNode> Parser::ParsePattern(std::vector<Token> &params, VarKind kind) {
        Sp<SyntaxNode> pattern;

        if (Match(u'[')) {
            pattern = ParseArrayPattern(params, kind);
        } else if (Match(u'{')) {
            pattern = ParseObjectPattern(params, kind);
        } else {
            if (MatchKeyword(u"let") && (kind == VarKind::Const || kind == VarKind::Let)) {
                TolerateUnexpectedToken(lookahead_, ParseMessages::LetInLexicalBinding);
            }
            params.push_back(lookahead_);
            pattern = ParseVariableIdentifier(kind);
        }

        return pattern;
    }

    Sp<SyntaxNode> Parser::ParsePatternWithDefault(std::vector<Token> &params, VarKind kind) {
        Token start_token_ = lookahead_;

        auto pattern = ParsePattern(params, kind);
        if (Match(u'=')) {
            NextToken();
            bool prev_allow_yield = context_.allow_yield;
            context_.allow_yield = true;
            auto right = IsolateCoverGrammar<Expression>([this] {
                return ParseAssignmentExpression();
            });
            context_.allow_yield = prev_allow_yield;
            auto node = Alloc<AssignmentPattern>();
            node->left = move(pattern);
            node->right = move(right);
            pattern = node;
        }

        return pattern;
    }

    Sp<RestElement> Parser::ParseBindingRestElement(std::vector<Token> &params, VarKind kind) {
        auto start_marker = CreateStartMarker();

        Expect(u"...");
        auto node = Alloc<RestElement>();
        node->argument = ParsePattern(params, kind);

        return Finalize(start_marker, node);
    }

    Sp<ArrayPattern> Parser::ParseArrayPattern(std::vector<Token> &params, VarKind kind) {
        auto start_marker = CreateStartMarker();

        Expect(u'[');
        std::vector<Sp<SyntaxNode>> elements;
        while (!Match(u']')) {
            if (Match(u',')) {
                NextToken();
                // TODO: push nullopt
                elements.push_back(nullptr);
            } else {
                if (Match(u"...")) {
                    elements.push_back(ParseBindingRestElement(params, kind));
                    break;
                } else {
                    elements.push_back(ParsePatternWithDefault(params, kind));
                }
                if (!Match(u']')) {
                    Expect(u',');
                }
            }
        }
        Expect(u']');

        auto node = Alloc<ArrayPattern>();
        node->elements = move(elements);
        return Finalize(start_marker, node);
    }

    Sp<Property> Parser::ParsePropertyPattern(std::vector<Token> &params, VarKind kind) {
        auto start_marker = CreateStartMarker();

        auto node = Alloc<Property>();
        node->computed = false;
        node->shorthand = false;
        node->method = false;

        if (lookahead_.type_ == JsTokenType::Identifier) {
            Token keyToken = lookahead_;
            node->key = ParseVariableIdentifier(VarKind::Invalid);
            auto id_ = Alloc<Identifier>();
            id_->name = keyToken.value_;
            auto init = Finalize(start_marker, id_);
            if (Match(u'=')) {
                params.push_back(keyToken);
                node->shorthand = true;
                NextToken();
                auto expr = ParseAssignmentExpression();
                auto assign = Alloc<AssignmentPattern>();
                assign->left = move(init);
                assign->right = move(expr);
                node->value = Finalize(StartNode(keyToken), assign);
            } else if (!Match(u':')) {
                params.push_back(keyToken);
                node->shorthand = true;
                node->value = move(init);
            } else {
                Expect(u':');
                node->value = ParsePatternWithDefault(params, kind);
            }
        } else {
            node->computed = Match(u'[');
            node->key = ParseObjectPropertyKey();
            Expect(u':');
            node->value = ParsePatternWithDefault(params, kind);
        }

        return Finalize(start_marker, node);
    }

    Sp<RestElement> Parser::ParseRestProperty(std::vector<Token> &params, VarKind kind) {
        auto start_marker = CreateStartMarker();
        Expect(u"...");
        auto arg = ParsePattern(params, VarKind::Invalid);
        if (Match(u'=')) {
            ThrowError(ParseMessages::DefaultRestProperty);
        }
        if (!Match(u'}')) {
            ThrowError(ParseMessages::PropertyAfterRestProperty);
        }
        auto node = Alloc<RestElement>();
        node->argument = move(arg);
        return Finalize(start_marker, node);
    }

    Sp<ObjectPattern> Parser::ParseObjectPattern(std::vector<Token> &params, VarKind kind) {
        auto start_marker = CreateStartMarker();
        std::vector<Sp<SyntaxNode>> props;

        Expect(u'{');
        while (!Match(u'}')) {
            if (Match(u"...")) {
                props.push_back(ParseRestProperty(params, kind));
            } else {
                props.push_back(ParsePropertyPattern(params, kind));
            }

            if (!Match(u'}')) {
                Expect(u',');
            }
        }
        Expect(u'}');

        auto node = Alloc<ObjectPattern>();
        node->properties = move(props);
        return Finalize(start_marker, node);
    }

    std::vector<Sp<SyntaxNode>> Parser::ParseAsyncArguments() {
        return {};
    }

    Sp<Expression> Parser::ParseGroupExpression() {
        return nullptr;
    }

    Sp<TemplateLiteral> Parser::ParseTemplateLiteral() {
        return nullptr;
    }

    std::vector<Sp<Property>> Parser::ParseClassElementList() {
        return {};
    }

}

