//
// Created by Duzhong Chen on 2019/9/3.
//
#pragma once

#include "parser_common.h"
#include "error_message.h"

#define ASSERT_NOT_NULL(EXPR, MSG) if ((EXPR) == nullptr) { \
        LogError(std::string(#EXPR) + " should not be nullptr " + MSG); \
        return false; \
    }

namespace parser {

    class Parser final: public ParserCommon {
    public:

        Parser(
            shared_ptr<u16string> source,
            const Config& config
        ): ParserCommon(source, config) {

        }

        template<typename T, typename ...Args>
        Sp<T> Alloc(Args && ...args) {
            static_assert(std::is_base_of<SyntaxNode, T>::value, "T not derived from AstNode");

            return Sp<T>(new T);
        }


        template<typename FromT, typename ToT>
        bool Finalize(const Marker& marker, FromT from, ToT& to);


        template <typename NodePtr>
        bool ParsePrimaryExpression(NodePtr& expr);

        template <typename NodePtr>
        bool ParseSpreadElement(NodePtr& ptr);

        template <typename NodePtr>
        bool ParsePropertyMethodFunction(NodePtr& ptr);

        template <typename NodePtr>
        bool ParsePropertyMethodAsyncFunction(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseObjectProperty(bool& has_proto, NodePtr& ptr);

        template <typename NodePtr>
        bool ParseObjectPropertyKey(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseObjectInitializer(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseTemplateHead(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseTemplateElement(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseTemplateLiteral(NodePtr& ptr);

        template <typename NodePtr>
        bool ReinterpretExpressionAsPattern(NodePtr& ptr);

        template <typename NodePtr>
        bool ReinterpretAsCoverFormalsList(NodePtr& ptr, FormalParameterOptions& list);

        template <typename NodePtr>
        bool ParseGroupExpression(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseArguments(std::vector<Sp<NodePtr>>& ptr);

        template <typename NodePtr>
        bool ParseIdentifierName(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseNewExpression(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseAsyncArgument(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseAsyncArguments(std::vector<NodePtr>& ptr);

        template <typename NodePtr>
        bool ParseImportCall(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseLeftHandSideExpressionAllowCall(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseSuper(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseLeftHandSideExpression(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseUpdateExpression(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseAwaitExpression(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseUnaryExpression(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseExponentiationExpression(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseBinaryExpression(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseConditionalExpression(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseAssignmentExpression(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseStatementListItem(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseBlock(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseLexicalBinding(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseBindingList(VarKind kind, NodePtr& ptr);

        bool IsLexicalDeclaration();
        bool IsIdentifierName(Token&);
        bool ExpectCommaSeparator();

        template <typename NodePtr>
        bool ParseLexicalDeclaration(NodePtr& ptr, bool& in_for);

        template <typename NodePtr>
        bool ParseBindingRestElement(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseArrayPattern(NodePtr& ptr);

        template <typename NodePtr>
        bool ParsePropertyPattern(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseRestProperty(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseObjectPattern(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseArrayInitializer(NodePtr& expr);

        template <typename NodePtr>
        bool PeinterpretExpressionAsPattern(NodePtr& ptr);

        bool ParseFormalParameters(optional<Token> first_restricted, FormalParameterOptions& option);
        bool ParseFormalParameter(FormalParameterOptions& option);
        bool IsStartOfExpression();

        template <typename NodePtr>
        bool ParseRestElement(std::vector<Sp<SyntaxNode>>& params, NodePtr& ptr);

        template <typename NodePtr>
        bool ParsePattern(std::vector<Sp<SyntaxNode>>& params, VarKind  kind, NodePtr& ptr);

        template <typename NodePtr>
        bool ParsePatternWithDefault(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseVariableIdentifier(VarKind kind, NodePtr& ptr);

        template <typename NodePtr>
        bool ParseVariableDeclaration(bool in_for, NodePtr& ptr);

        template <typename NodePtr>
        bool ParseVariableDeclarationList(bool in_for, NodePtr& ptr);

        template <typename NodePtr>
        bool ParseVariableStatement(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseEmptyStatement(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseExpressionStatement(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseExpression(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseIfClause(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseIfStatement(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseDoWhileStatement(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseWhileStatement(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseForStatement(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseContinueStatement(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseBreakStatement(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseReturnStatement(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseWithStatement(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseSwitchCase(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseSwitchStatement(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseLabelledStatement(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseThrowStatement(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseCatchClause(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseFinallyClause(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseTryStatement(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseDebuggerStatement(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseStatement(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseFunctionSourceElements(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseFunctionDeclaration(bool identifier_is_optional, NodePtr& ptr);

        template <typename NodePtr>
        bool ParseFunctionExpression(NodePtr& expr);

        template <typename NodePtr>
        bool ParseDirective(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseDirectivePrologues(std::vector<NodePtr>& ptr_vec);

        template <typename NodePtr>
        bool ParseGetterMethod(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseSetterMethod(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseGeneratorMethod(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseYieldExpression(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseClassElement(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseClassElementList(std::vector<NodePtr>& vec);

        template <typename NodePtr>
        bool ParseClassBody(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseClassDeclaration(bool identifier_is_optional, NodePtr& ptr);

        template <typename NodePtr>
        bool ParseClassExpression(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseModule(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseScript(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseModuleSpecifier(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseImportSpecifier(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseNamedImports(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseImportDefaultSpecifier(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseImportNamespaceSpecifier(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseImportDeclaration(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseExportSpecifier(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseExportDeclaration(NodePtr& ptr);

        bool MatchImportCall();

        bool MatchAsyncFunction();


        ~Parser() = default;

    };


//    template<typename FromT, typename ToT>
//    bool Finalize(const Marker& marker, FromT from, ToT& to);
    template<typename FromT, typename ToT>
    bool Parser::Finalize(const Parser::Marker &marker, FromT from, ToT& to) {
        static_assert(std::is_convertible<FromT, ToT>::value, "FromT can not convert to ToT");

        if (config_.range) {
            from->range = std::make_pair(marker.index, LastMarker().index);
        }

        if (config_.loc) {
            from->location.start_ = Position {
                marker.line,
                marker.column,
            };
            from->location.end_ = Position {
                LastMarker().line,
                LastMarker().column,
            };
        }

        to = from;
        return true;
    }

    bool Parser::MatchAsyncFunction() {
        bool match = MatchContextualKeyword(u"async");
        if (match) {
            auto state = scanner_->SaveState();
            std::vector<Comment> comments;
            scanner_->ScanComments(comments);
            Token next;
            scanner_->Lex(next);
            scanner_->RestoreState(state);

            match = (state.line_number_ == next.line_number_) && (next.type_ == JsTokenType::Keyword) && (next.value_ == u"function");
        }

        return match;
    }

    template <typename NodePtr>
    bool Parser::ParsePrimaryExpression(NodePtr& expr) {
        static_assert(std::is_convertible<Sp<Expression>, NodePtr>::value, "NodePtr can not convert to AstNode*");

        expr = nullptr;
        auto marker = CreateNode();
        Token token;

        switch (lookahead_.type_) {
            case JsTokenType::Identifier: {
                if ((context_.is_module || context_.await) && lookahead_.value_ == u"await") {
                    UnexpectedToken(&lookahead_);
                }
                if (MatchAsyncFunction()) {
                    return ParseFunctionExpression(expr);
                } else {
                    auto node = Alloc<Identifier>();
                    Token next;
                    DO(NextToken(&next))
                    node->name = next.value_;
                    return Finalize(marker, node, expr);
                }
            }

            case JsTokenType::NumericLiteral:
            case JsTokenType::StringLiteral: {
                if (context_.strict_ && lookahead_.octal_) {
                    UnexpectedToken(&lookahead_);
//                this.tolerateUnexpectedToken(this.lookahead, Messages.StrictOctalLiteral);
                }
                context_.is_assignment_target = false;
                context_.is_binding_element = false;
                DO(NextToken(&token))
//            raw = this.getTokenRaw(token);
                auto node = Alloc<Literal>();
                node->value = token.value_;
                Finalize(marker, node, expr);
                break;
            }

            case JsTokenType::BooleanLiteral: {
                context_.is_assignment_target = false;
                context_.is_binding_element = false;
                DO(NextToken(&token))
//            raw = this.getTokenRaw(token);
                auto node = Alloc<Literal>();
                node->value = token.value_;
                Finalize(marker, node, expr);
                break;
            }

            case JsTokenType::NullLiteral: {
                context_.is_assignment_target = false;
                context_.is_binding_element = false;
                DO(NextToken(&token))
//            raw = this.getTokenRaw(token);
                auto node = Alloc<Literal>();
                node->value = token.value_;
                Finalize(marker, node, expr);
                break;
            }

            case JsTokenType::Template:
                return ParseTemplateLiteral(expr);

            case JsTokenType::Punctuator: {
                switch (lookahead_.value_[0]) {
                    case '(':
                        context_.is_binding_element = false;
                        return InheritCoverGrammar([this, &expr]() {
                            return ParseGroupExpression(expr);
                        });

                    case '[':
                        return InheritCoverGrammar([this, &expr]() {
                            return ParseArrayInitializer(expr);
                        });

                    case '{':
                        return InheritCoverGrammar([this, &expr]() {
                            return ParseObjectInitializer(expr);
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
                        return false;

                    default: {
                        DO(NextToken(&token))
                        UnexpectedToken(&token);
                        return false;
                    }

                }
                break;
            }

            case JsTokenType::Keyword:
                if (!context_.strict_ && context_.allow_yield && MatchKeyword(u"yield")) {
                    return ParseIdentifierName(expr);
                } else if (!context_.strict_ && MatchKeyword(u"let")) {
                    DO(NextToken(&token));
                    auto id = Alloc<Identifier>();
                    id->name = token.value_;
                    return Finalize(marker, id, expr);
                } else {
                    context_.is_assignment_target = false;
                    context_.is_binding_element = false;
                    if (MatchKeyword(u"function")) {
                        ParseFunctionExpression(expr);
                    } else if (MatchKeyword(u"this")) {
                        DO(NextToken(&token))
                        auto th = Alloc<ThisExpression>();
                        return Finalize(marker, th, expr);
                    } else if (MatchKeyword(u"class")) {
                        return ParseClassExpression(expr);
                    } else if (MatchImportCall()) {
                        return ParseImportCall(expr);
                    } else {
                        DO(NextToken(&token));
                        UnexpectedToken(&token);
                        return false;
                    }
                }
                break;

            default:
                DO(NextToken(&token))
                UnexpectedToken(&token);
                return false;

        }

        return true;
    }


    template <typename NodePtr>
    bool Parser::ParseSpreadElement(NodePtr& expr) {
        static_assert(std::is_convertible<Sp<SpreadElement>, NodePtr>::value, "NodePtr can not accept SpreadElement*");
        auto marker = CreateNode();

        DO(Expect(u"..."))

        auto node = Alloc<SpreadElement>();

        DO(InheritCoverGrammar([this, &node]() {
            return ParseAssignmentExpression(node->argument);
        }));

        return Finalize(marker, node, expr);
    }

    template <typename NodePtr>
    bool Parser::ParseArrayInitializer(NodePtr &expr) {
        static_assert(std::is_convertible<Sp<ArrayExpression>, NodePtr>::value, "NodePtr can not accept ArrayExpression*");
        auto marker = CreateNode();
        auto node = Alloc<ArrayExpression>();
        Sp<SyntaxNode> element = nullptr;

        DO(Expect('['))
        while (!Match(']')) {
            if (Match(',')) {
                NextToken();
                node->elements.push_back(nullptr);
            } else if (Match(u"...")) {
                DO(ParseSpreadElement(element))
                if (!Match(']')) {
                    context_.is_assignment_target = false;
                    context_.is_binding_element = false;
                    Expect(',');
                }
                node->elements.push_back(element);
            } else {
                DO(InheritCoverGrammar([this, &element]() {
                    return ParseAssignmentExpression(element);
                }))
                node->elements.push_back(element);
                if (!Match(']')) {
                    DO(Expect(','))
                }
            }
        }
        DO(Expect(']'))

        return Finalize(marker, node, expr);
    }

    template <typename NodePtr>
    bool Parser::ParsePropertyMethodFunction(NodePtr &ptr) {
        static_assert(std::is_convertible<FunctionExpression*, NodePtr>::value, "NodePtr can not accept FunctionExpression*");
        auto marker = CreateNode();
        auto node = Alloc<FunctionExpression>();

        bool isGenerator = false;

        FormalParameterOptions options;
        bool previous_allow_yield = context_.allow_yield;
        context_.allow_yield = true;
        DO(ParseFormalParameters(nullopt, options))
//        const params = this.parseFormalParameters();
//        const method = this.parsePropertyMethod(params);
        context_.allow_yield = previous_allow_yield;

        return Finalize(marker, node, ptr);
    }

    template <typename NodePtr>
    bool Parser::ParsePropertyMethodAsyncFunction(NodePtr& ptr) {
        static_assert(std::is_convertible<AsyncFunctionExpression*, NodePtr>::value, "NodePtr can not accept FunctionExpression*");
        auto marker = CreateNode();
        auto node = Alloc<AsyncFunctionExpression>();

        bool isGenerator = false;

        FormalParameterOptions options;
        bool previous_allow_yield = context_.allow_yield;
        bool previous_await = context_.await;
        context_.allow_yield = true;
        DO(ParseFormalParameters(nullopt, options))
//        const params = this.parseFormalParameters();
//        const method = this.parsePropertyMethod(params);
        context_.allow_yield = previous_allow_yield;
        context_.await = previous_await;

        return Finalize(marker, node, ptr);
    }

    template <typename NodePtr>
    bool Parser::ParseObjectPropertyKey(NodePtr &ptr) {
        auto marker = CreateNode();
        Token token;
        DO(NextToken(&token))

        switch (token.type_) {
            case JsTokenType::StringLiteral:
            case JsTokenType::NumericLiteral: {
                if (context_.strict_ && token.octal_) {
                    LogError("StrictOctalLiteral");
                }
                auto node = Alloc<Literal>();
                node->value = token.value_;
                return Finalize(marker, node, ptr);
            }

            case JsTokenType::Identifier:
            case JsTokenType::BooleanLiteral:
            case JsTokenType::NullLiteral:
            case JsTokenType::Keyword: {
                auto node = Alloc<Identifier>();
                node->name = token.value_;
                return Finalize(marker, node, ptr);
            }

            case JsTokenType::Punctuator:
                if (token.value_ == u"[") {
                    DO(IsolateCoverGrammar([this, &ptr]() {
                        return ParseAssignmentExpression(ptr);
                    }));
                    DO(Expect(']'))
                } else {
                    UnexpectedToken(&token);
                    return false;
                }
                break;

            default:
                UnexpectedToken(&token);
                return false;
        }

        return true;
    }

    template <typename NodePtr>
    bool Parser::ParseObjectProperty(bool &has_proto, NodePtr &ptr) {
        auto marker = CreateNode();
        Token token = lookahead_;
        VarKind kind;
        bool computed = false;
        bool method = false;
        bool shorthand = false;
        bool is_async = false;

        Sp<SyntaxNode> key = nullptr;

        if (token.type_ == JsTokenType::Identifier) {
            auto id = token.value_;
            DO(NextToken())
            computed = Match('[');
            is_async = !has_line_terminator_ && (id == u"async") &&
                      !Match(':') && !Match('(') && !Match('*') && !Match(',');
            if (is_async) {
                DO(ParseObjectPropertyKey(key))
            } else {
                auto node = Alloc<Identifier>();
                node->name = id;
                DO(Finalize(marker, node, key));
            }
        } else if (Match('*')) {
            DO(NextToken())
        } else {
            computed = Match('[');
            DO(ParseObjectPropertyKey(key))
        }


        return true;
    }

    bool Parser::IsLexicalDeclaration() {
        auto state = scanner_->SaveState();
        std::vector<Comment> comments;
        scanner_->ScanComments(comments);
        Token next;
        DO(scanner_->Lex(next))
        scanner_->RestoreState(state);

        return (next.type_ == JsTokenType::Identifier) ||
            (next.type_ == JsTokenType::Punctuator && next.value_ == u"[") ||
            (next.type_ == JsTokenType::Punctuator && next.value_ == u"{") ||
            (next.type_ == JsTokenType::Keyword && next.value_ == u"let") ||
            (next.type_ == JsTokenType::Keyword && next.value_ == u"yield");
    }

    bool Parser::ParseFormalParameters(optional<Token> first_restricted, parser::Parser::FormalParameterOptions &options) {

        options.simple = true;
        options.params.clear();
        options.first_restricted = first_restricted;

        DO(Expect('('))
        if (!Match(')')) {
            while (lookahead_.type_ != JsTokenType::EOF_) {
                DO(ParseFormalParameter(options))
                if (Match(')')) {
                    break;
                }
                DO(Expect(','))
                if (Match(')')) {
                    break;
                }
            }
        }
        DO(Expect(')'))

        return true;
    }

    bool Parser::ParseFormalParameter(parser::Parser::FormalParameterOptions &option) {
        if (Match(u"...")) {

        }

        return true;
    }

    template <typename NodePtr>
    bool Parser::ParseContinueStatement(NodePtr &ptr) {
        auto marker = CreateNode();
        DO(!ExpectKeyword(u"continue"))
        auto node = Alloc<ContinueStatement>();

        if (lookahead_.type_ == JsTokenType::Identifier && !has_line_terminator_) {
            DO(ParseVariableIdentifier(VarKind::Invalid, node->label))
        }

        DO(ConsumeSemicolon())

        if (!node->label && !context_.in_iteration) {
            LogError("IllegalContinue");
            return false;
        }

        return Finalize(marker, node, ptr);
    }

    template <typename NodePtr>
    bool Parser::ParseVariableIdentifier(VarKind kind, NodePtr &ptr) {
        auto marker = CreateNode();

        Token token;
        DO(NextToken(&token))
        if (token.type_ == JsTokenType::Keyword && token.value_ == u"yield") {
            if (context_.strict_) {
                DO(TolerateUnexpectedToken(&token, ParseMessages::StrictReservedWord))
            } else if (!context_.allow_yield) {
                UnexpectedToken(&token);
                return false;
            }
        } else if (token.type_ == JsTokenType::Identifier) {
            if (context_.strict_ && token.type_ == JsTokenType::Keyword && scanner_->IsStrictModeReservedWord(token.value_)) {
                DO(TolerateUnexpectedToken(&token, ParseMessages::StrictReservedWord))
            } else {
                if (context_.strict_ || token.value_ != u"let" || kind != VarKind::Var) {
                    UnexpectedToken(&token);
                    return false;
                }
            }
        } else if ((context_.is_module || context_.await) && token.type_ == JsTokenType::Identifier && token.value_ == u"await") {
            DO(TolerateUnexpectedToken(&token))
        }

        auto node = Alloc<Identifier>();
        node->name = token.value_;
        return Finalize(marker, node, ptr);
    }

    template <typename NodePtr>
    bool Parser::ParseRestElement(std::vector<Sp<SyntaxNode>>& params, NodePtr &ptr) {
        auto marker = CreateNode();
        auto node = Alloc<RestElement>();

        DO(Expect(u"..."))
        DO(ParsePattern(params, VarKind::Invalid, node->argument))
        if (Match('=')) {
            LogError("DefaultRestParameter");
            return false;
        }
        if (!Match(')')) {
            LogError("ParameterAfterRestParameter");
            return false;
        }

        return Finalize(marker, node, ptr);
    }

    template <typename NodePtr>
    bool Parser::ParseModule(NodePtr &ptr) {
        context_.strict_ = true;
        context_.is_module = true;
        auto marker = CreateNode();
        auto node = make_shared<Module>();
        DO(ParseDirectivePrologues(node->body))
        while (lookahead_.type_ != JsTokenType::EOF_) {
            Sp<SyntaxNode> statement_list_item;
            DO(ParseStatementListItem(statement_list_item))
            node->body.push_back(move(statement_list_item));
        }
        return Finalize(marker, node, ptr);
    }

    template <typename NodePtr>
    bool Parser::ParseStatement(NodePtr &ptr) {
        Sp<Statement> statement;

        switch (lookahead_.type_) {
            case JsTokenType::BooleanLiteral:
            case JsTokenType::NullLiteral:
            case JsTokenType::NumericLiteral:
            case JsTokenType::StringLiteral:
            case JsTokenType::Template:
            case JsTokenType::RegularExpression: {
                DO(ParseExpressionStatement(statement))
                break;
            }

            case JsTokenType::Punctuator: {
                auto& value = lookahead_.value_;
                if (value == u"{") {
                    DO(ParseBlock(statement))
                } else if (value == u"(") {
                    DO(ParseExpressionStatement(statement))
                } else if (value == u";") {
                    DO(ParseEmptyStatement(statement));
                } else {
                    DO(ParseExpressionStatement(statement))
                }
                break;
            }

            case JsTokenType::Identifier: {
                if (MatchAsyncFunction()) {
                    DO(ParseFunctionDeclaration(false, statement))
                } else {
                    DO(ParseLabelledStatement(statement))
                }
                break;
            }

            case JsTokenType::Keyword: {
                auto& value = lookahead_.value_;
                if (value == u"break") {
                    DO(ParseBreakStatement(statement))
                } else if (value == u"continue") {
                    DO(ParseContinueStatement(statement))
                } else if (value == u"debugger") {
                    DO(ParseDebuggerStatement(statement))
                } else if (value == u"do") {
                    DO(ParseDoWhileStatement(statement))
                } else if (value == u"for") {
                    DO(ParseForStatement(statement))
                } else if (value == u"function") {
                    DO(ParseFunctionDeclaration(false, statement))
                } else if (value == u"if") {
                    DO(ParseIfStatement(statement))
                } else if (value == u"return") {
                    DO(ParseReturnStatement(statement))
                } else if (value == u"switch") {
                    DO(ParseSwitchStatement(statement))
                } else if (value == u"throw") {
                    DO(ParseThrowStatement(statement))
                } else if (value == u"try") {
                    DO(ParseTryStatement(statement))
                } else if (value == u"var") {
                    DO(ParseVariableStatement(statement))
                } else if (value == u"while") {
                    DO(ParseWhileStatement(statement))
                } else if (value == u"with") {
                    DO(ParseWithStatement(statement))
                } else {
                    DO(ParseExpressionStatement(statement))
                }
                break;
            }

            default:
                UnexpectedToken(&lookahead_);
                return false;

        }

        ptr = statement;
        return true;
    }

    template <typename NodePtr>
    bool Parser::ParseVariableStatement(NodePtr &ptr) {
        auto marker = CreateNode();
        DO(ExpectKeyword(u"var"))

        auto node = Alloc<VariableDeclaration>();
        DO(ParseVariableDeclarationList(false, node->declarations))
        DO(ConsumeSemicolon())

        node->kind = VarKind::Var;
        return Finalize(marker, node, ptr);
    }

    template <typename NodePtr>
    bool Parser::ParseVariableDeclaration(bool in_for, NodePtr &ptr) {
        auto marker = CreateNode();
        auto node = Alloc<VariableDeclarator>();

        Sp<SyntaxNode> id;
        vector<Sp<SyntaxNode>> params;
        DO(ParsePattern(params, VarKind::Var, id))

        if (context_.strict_ && id->type == SyntaxNodeType::Identifier) {
            auto identifier = dynamic_pointer_cast<Identifier>(id);
            if (scanner_->IsRestrictedWord(identifier->name)) {
                DO(TolerateError(ParseMessages::StrictVarName))
            }
        }

        optional<Sp<Expression>> init;
        if (Match(u'=')) {
            DO(NextToken())
            DO(IsolateCoverGrammar([this, &init] {
                return ParseAssignmentExpression(*init);
            }))
        } else if (id->type != SyntaxNodeType::Identifier && !in_for) {
            DO(Expect(u'='))
        }

        node->id = move(id);
        node->init = init;

        return Finalize(marker, node, ptr);
    }

    template <typename NodePtr>
    bool Parser::ParseSwitchCase(NodePtr &ptr) {
        auto marker = CreateNode();
        auto node = Alloc<SwitchCase>();

        if (MatchKeyword(u"default")) {
            DO(NextToken())
            node->test.reset();
        } else {
            DO(ExpectKeyword(u"case"))
            DO(ParseExpression(*node->test))
        }
        DO(Expect(u':'))

        while (true) {
            if (Match(u'}') || MatchKeyword(u"default") || MatchKeyword(u"case")) {
                break;
            }
            Sp<Statement> con;
            DO(ParseStatementListItem(con))
            node->consequent.push_back(std::move(con));
        }

        return Finalize(marker, node, ptr);
    }

    template <typename NodePtr>
    bool Parser::ParseTemplateLiteral(NodePtr &ptr) {
        auto marker = CreateNode();
        auto node = Alloc<TemplateElement>();

        Sp<TemplateElement> quasi;
        DO(ParseTemplateHead(quasi))
        node->quasis.push_back(quasi);

        // TODO: TemplateLiteral
        return false;
    }

    template <typename NodePtr>
    bool Parser::ParseFunctionDeclaration(bool identifier_is_optional, NodePtr &ptr) {
        auto marker = CreateNode();

        bool is_async = MatchContextualKeyword(u"async");
        if (is_async) {
            DO(NextToken())
        }

        DO(ExpectKeyword(u"function"))

        bool is_generator = is_async ? false : Match(u'*');
        if (is_generator) {
            DO(NextToken())
        }

        optional<Sp<Identifier>> id;
        optional<Token> first_restricted;
        string message;

        if (!identifier_is_optional || !Match(u'(')) {
            Token token = lookahead_;
            DO(ParseVariableIdentifier(VarKind::Invalid, *id))
            if (context_.strict_) {
                if (scanner_->IsRestrictedWord(token.value_)) {
                    DO(TolerateUnexpectedToken(&token, ParseMessages::StrictFunctionName))
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
        DO(ParseFormalParameters(first_restricted, options))
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
            UnexpectedToken(&temp, message);
            return false;
        }
        if (context_.strict_ && options.stricted) {
            Token temp = *options.stricted;
            DO(TolerateUnexpectedToken(&temp, message))
        }

        context_.strict_ = prev_strict;
        context_.allow_strict_directive = prev_allow_strict_directive;
        context_.await = prev_allow_await;
        context_.allow_yield = prev_allow_yield;

        if (is_async) {
            auto node = Alloc<AsyncFunctionDeclaration>();
            node->id = id;
            return Finalize(marker, node, ptr);
        } else {
            auto node = Alloc<FunctionDeclaration>();
            node->id = id;
            node->generator = is_generator;

            return Finalize(marker, node, ptr);
        }
    }

    template <typename NodePtr>
    bool Parser::ParseBlock(NodePtr &ptr) {
        auto marker = CreateNode();
        auto node = Alloc<BlockStatement>();

        DO(Expect(u'{'))
        while (true) {
            if (Match(u'}')) {
                break;
            }
            Sp<SyntaxNode> stmt;
            DO(ParseStatementListItem(stmt))
            node->body.push_back(std::move(stmt));
        }
        DO(Expect(u'}'))

        return Finalize(marker, node, ptr);
    }

    template <typename NodePtr>
    bool Parser::ParseIfClause(NodePtr &ptr) {
        if (context_.strict_ && MatchKeyword(u"function")) {
            LogError("StrictFunction");
        }
        return ParseStatement(ptr);
    }

    template <typename NodePtr>
    bool Parser::ParseIfStatement(NodePtr &ptr) {
        auto marker = CreateNode();
        auto node = Alloc<IfStatement>();

        DO(ExpectKeyword(u"if"))
        DO(Expect('('))
        DO(ParseExpression(node->test))

        if (!Match(u')') && config_.tolerant) {
            Token token;
            NextToken(&token);
            UnexpectedToken(&token);
            Finalize(CreateNode(), Alloc<EmptyStatement>(), node->consequent);
        } else {
            DO(Expect(u')'))
            DO(ParseIfClause(node->consequent))
            if (MatchKeyword(u"else")) {
                NextToken();
                DO(ParseIfClause(node->alternate))
            }
        }

        return Finalize(marker, node, ptr);
    }

    template <typename NodePtr>
    bool Parser::ParseDoWhileStatement(NodePtr &ptr) {
        auto marker = CreateNode();
        auto node = Alloc<DoWhileStatement>();
        DO(ExpectKeyword(u"do"))

        auto previous_in_interation = context_.in_iteration;
        context_.in_iteration = true;
        DO(ParseStatement(node->body))
        context_.in_iteration = previous_in_interation;

        DO(ExpectKeyword(u"while"))
        DO(Expect(u'('))
        DO(ParseExpression(node->test))

        if (!Match(u')') && config_.tolerant) {
            Token token;
            NextToken(&token);
            UnexpectedToken(&token);
        } else {
            DO(Expect(u'('))
            if (Match(u';')) {
                NextToken();
            }
        }

        return Finalize(marker, node, ptr);
    }

    template <typename NodePtr>
    bool Parser::ParseWhileStatement(NodePtr &ptr) {
        auto marker = CreateNode();
        auto node = Alloc<WhileStatement>();

        DO(ExpectKeyword(u"while"))
        DO(Expect(u'('));
        DO(ParseExpression(node->test))

        if (!Match(u')') && config_.tolerant) {
            Token token;
            NextToken(&token);
            UnexpectedToken(&token);
            Finalize(CreateNode(), Alloc<EmptyStatement>(), node->body);
        } else {
            DO(Expect(u')'));

            auto prev_in_interation = context_.in_iteration;
            context_.in_iteration = true;
            DO(ParseStatement(node->body))
            context_.in_iteration = prev_in_interation;
        }

        return Finalize(marker, node, ptr);
    }

    template <typename NodePtr>
    bool Parser::ParseBreakStatement(NodePtr &ptr) {
        auto marker = CreateNode();
        DO(ExpectKeyword(u"break"))

        std::optional<Sp<Identifier>> label;
        if (lookahead_.type_ == JsTokenType::Identifier && !has_line_terminator_) {
            Sp<Identifier> id;
            DO(ParseVariableIdentifier(VarKind::Invalid, id))

            UString key = UString(u"$") + id->name;
            // TODO: labelSet
            label = id;
        }

        DO(ConsumeSemicolon())

        if (!label && !context_.in_iteration && !context_.in_switch) {
            LogError("IllegalBreak");
            return false;
        }

        auto node = Alloc<BreakStatement>();
        node->label = label;
        return Finalize(marker, node, ptr);
    }

    template <typename NodePtr>
    bool Parser::ParseEmptyStatement(NodePtr &ptr) {
        auto node = Alloc<EmptyStatement>();
        auto marker = CreateNode();
        DO(Expect(u';'));
        return Finalize(marker, node, ptr);
    }

    template <typename NodePtr>
    bool Parser::ParseClassBody(NodePtr &ptr) {
        auto marker = CreateNode();
        auto node = Alloc<ClassBody>();

        DO(ParseClassElementList(node->body))

        return Finalize(marker, node, ptr);
    }

    template <typename NodePtr>
    bool Parser::ParseCatchClause(NodePtr &ptr) {
        auto marker = CreateNode();

        DO(ExpectKeyword(u"catch"))

        DO(Expect(u'('))
        if (Match(u')')) {
            UnexpectedToken(&lookahead_);
            return false;
        }

        auto node = Alloc<CatchClause>();

        // TODO: parse params

        DO(Expect(u')'))
        DO(ParseBlock(node->body))

        return Finalize(marker, node, ptr);
    }

    template <typename NodePtr>
    bool Parser::ParseFinallyClause(NodePtr &ptr) {
        DO(ExpectKeyword(u"finally"))
        return ParseBlock(ptr);
    }

    template <typename NodePtr>
    bool Parser::ParseLabelledStatement(NodePtr &ptr) {
        auto start_marker = CreateNode();
        Sp<Expression> expr;
        DO(ParseExpression(expr))

        Sp<Statement> statement;
        if ((expr->type == SyntaxNodeType::Identifier) && Match(u':')) {
            DO(NextToken())

            auto id = dynamic_pointer_cast<Identifier>(expr);
            UString key = UString(u"$") + id->name;

            // TODO: label set

            Sp<Statement> body;

            if (MatchKeyword(u"class")) {
                UnexpectedToken(&lookahead_);
                DO(ParseClassDeclaration(false, body))
            } else if (MatchKeyword(u"function")) {
                Token token = lookahead_;
                Sp<Declaration> declaration;
                DO(ParseFunctionDeclaration(false, declaration))
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
                DO(ParseStatement(body))
            }

            auto node = Alloc<LabeledStatement>();
            node->label = id;
            node->body = body;

            statement = move(node);
        } else {
            DO(ConsumeSemicolon())
            auto node = Alloc<ExpressionStatement>();
            node->expression = expr;
            statement = move(node);
        }
        return Finalize(start_marker, statement, ptr);
    }

    template <typename NodePtr>
    bool Parser::ParseThrowStatement(NodePtr &ptr) {
        auto marker = CreateNode();
        DO(ExpectKeyword(u"throw"))

        if (has_line_terminator_) {
            LogError("NewlineAfterThrow");
            return false;
        }

        auto node = Alloc<ThrowStatement>();
        DO(ParseExpression(node->argument))
        DO(ConsumeSemicolon())

        return Finalize(marker, node, ptr);
    }

    template <typename NodePtr>
    bool Parser::ParseSwitchStatement(NodePtr &ptr) {
        auto marker = CreateNode();
        DO(ExpectKeyword(u"switch"))
        auto node = Alloc<SwitchStatement>();

        DO(Expect(u'('))
        DO(ParseExpression(node->discrimiant))
        DO(Expect(u')'))

        auto prev_in_switch = context_.in_switch;
        context_.in_switch = true;

        bool default_found = false;
        DO(Expect(u'{'))
        while (true) {
            if (Match(u'}')) {
                break;
            }
            Sp<SwitchCase> clause;
            DO(ParseSwitchCase(clause))
            if (!clause->test) {
                if (default_found) {
                    LogError("MultipleDefaultsInSwitch");
                    return false;
                }
                default_found = true;
            }
            node->cases.push_back(clause);
        }
        DO(Expect(u'}'))

        context_.in_switch = prev_in_switch;

        return Finalize(marker, node, ptr);
    }

    template <typename NodePtr>
    bool Parser::ParseExpressionStatement(NodePtr &ptr) {
        auto marker = CreateNode();
        auto node = Alloc<ExpressionStatement>();

        DO(ParseExpression(node->expression))
        DO(ConsumeSemicolon())

        return Finalize(marker, node, ptr);
    }

    template <typename NodePtr>
    bool Parser::ParseConditionalExpression(NodePtr &ptr) {
        auto marker = CreateNode();

        Sp<Expression> expr;
        DO(InheritCoverGrammar([this, &expr] {
            return ParseBinaryExpression(expr);
        }))
        if (Match(u'?')) {
            DO(NextToken())
            auto node = Alloc<ConditionalExpression>();

            bool prev_allow_in = context_.allow_in;
            context_.allow_in = true;

            DO(IsolateCoverGrammar([this, &node] {
                return ParseAssignmentExpression(node->consequent);
            }))
            context_.allow_in = prev_allow_in;

            DO(Expect(u':'))
            DO(IsolateCoverGrammar([this, &node] {
                return ParseAssignmentExpression(node->alternate);
            }))

            expr = move(node);

            context_.is_assignment_target = false;
            context_.is_binding_element = false;
        }

        return Finalize(marker, expr, ptr);
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

    template <typename NodePtr>
    bool Parser::ParseFunctionExpression(NodePtr &expr) {
        auto marker = CreateNode();

        bool is_async = MatchContextualKeyword(u"async");
        if (is_async) NextToken();

        DO(ExpectKeyword(u"function"))

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
                DO(ParseIdentifierName(*id))
            } else {
                DO(ParseVariableIdentifier(VarKind::Invalid, *id))
            }

            if (context_.strict_) {
                if (scanner_->IsRestrictedWord(token.value_)) {
                    DO(TolerateUnexpectedToken(&token, ParseMessages::StrictFunctionName))
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

        return true;
    }

    bool Parser::ExpectCommaSeparator() {
        if (config_.tolerant) {
            Token token = lookahead_;
            if (token.type_ == JsTokenType::Punctuator && token.value_ == u",") {
                DO(NextToken())
            } else if (token.type_ == JsTokenType::Punctuator && token.value_ == u";") {
                DO(NextToken())
                UnexpectedToken(&token);
            } else {
                UnexpectedToken(&token);
            }
        } else {
            return Expect(u',');
        }
        return true;
    }

    template <typename NodePtr>
    bool Parser::ParseObjectInitializer(NodePtr &ptr) {
        auto marker = CreateNode();

        DO(Expect(u'{'))
        auto node = Alloc<ObjectExpression>();
        bool has_proto = false;
        while (!Match(u'}')) {
            Sp<SyntaxNode> prop;
            if (Match(u"...")) {
                DO(ParseSpreadElement(prop))
            } else {
                DO(ParseObjectProperty(has_proto, prop))
            }
            node->properties.push_back(std::move(prop));
            if (!Match(u'}')) {
                DO(ExpectCommaSeparator())
            }
        }
        DO(Expect(u'}'))

        return Finalize(marker, node, ptr);
    }

    template <typename NodePtr>
    bool Parser::ParseYieldExpression(NodePtr &ptr) {
        auto marker = CreateNode();
        DO(ExpectKeyword(u"yield"))

        auto node = Alloc<YieldExpression>();
        if (has_line_terminator_) {
            bool prev_allow_yield = context_.allow_yield;
            context_.allow_yield = false;
            node->delegate = Match(u'*');
            if (node->delegate) {
                DO(NextToken())
                DO(ParseAssignmentExpression(node->argument))
            } else if (IsStartOfExpression()) {
                DO(ParseAssignmentExpression(node->argument))
            }
            context_.allow_yield = prev_allow_yield;
        }

        return Finalize(marker, node, ptr);
    }

    bool Parser::IsIdentifierName(Token& token) {
        return token.type_ == JsTokenType::Identifier ||
            token.type_ == JsTokenType::Keyword ||
            token.type_ == JsTokenType::BooleanLiteral ||
            token.type_ == JsTokenType::NullLiteral;
    }

    template <typename NodePtr>
    bool Parser::ParseIdentifierName(NodePtr &ptr) {
        auto marker = CreateNode();
        auto node = Alloc<Identifier>();
        Token token;
        DO(NextToken(&token))
        if (!IsIdentifierName(token)) {
            UnexpectedToken(&token);
            return false;
        }
        node->name = token.value_;
        return Finalize(marker, node, ptr);
    }

    template <typename NodePtr>
    bool Parser::ParseReturnStatement(NodePtr &ptr) {
        if (!context_.in_function_body) {
            LogError("IllegalReturn");
        }

        auto marker = CreateNode();
        DO(ExpectKeyword(u"return"))

        bool has_arg = (!Match(u';') && !Match(u'}') &&
            !has_line_terminator_ && lookahead_.type_ != JsTokenType::EOF_) ||
            lookahead_.type_ == JsTokenType::StringLiteral ||
            lookahead_.type_ == JsTokenType::Template;

        auto node = Alloc<ReturnStatement>();
        if (has_arg) {
            DO(ParseExpression(*node->argument))
        }

        DO(ConsumeSemicolon())

        return Finalize(marker, node, ptr);
    }

    template <typename NodePtr>
    bool Parser::ParseDebuggerStatement(NodePtr &ptr) {
        auto marker = CreateNode();
        DO(ExpectKeyword(u"keyword"))
        DO(ConsumeSemicolon())
        auto node = Alloc<DebuggerStatement>();
        return Finalize(marker, node, ptr);
    }

    template <typename NodePtr>
    bool Parser::ParseClassExpression(NodePtr &ptr) {
        auto marker = CreateNode();
        auto node = Alloc<ClassExpression>();

        bool prev_strict = context_.strict_;
        context_.strict_ = true;
        DO(ExpectKeyword(u"class"))

        if (lookahead_.type_ == JsTokenType::Identifier) {
            DO(ParseVariableIdentifier(VarKind::Invalid, *node->id))
        }

        if (MatchKeyword(u"extends")) {
            DO(NextToken());
            DO(IsolateCoverGrammar([this, &node] {
                Token token = lookahead_;
                Sp<Expression> temp_node;
                DO(ParseLeftHandSideExpressionAllowCall(temp_node))
                if (temp_node->type != SyntaxNodeType::Identifier) {
                    UnexpectedToken(&token);
                    return false;
                }
                Sp<Identifier> id = dynamic_pointer_cast<Identifier>(temp_node);
                node->super_class = id;
                return true;
            }))
        }

        DO(ParseClassBody(*node->body))
        context_.strict_ = prev_strict;

        return Finalize(marker, node, ptr);
    }

    template <typename NodePtr>
    bool Parser::ParseStatementListItem(NodePtr &ptr) {
        Sp<Statement> statement;
        context_.is_assignment_target = true;
        context_.is_binding_element = true;

        if (lookahead_.type_ == JsTokenType::Keyword) {
            if (lookahead_.value_ == u"export") {
                if (context_.is_module) {
                    DO(TolerateUnexpectedToken(&lookahead_, ParseMessages::IllegalExportDeclaration))
                }
                DO(ParseExportDeclaration(statement))
            } else if (lookahead_.value_ == u"import") {
                if (MatchImportCall()) {
                    DO(ParseExpressionStatement(statement))
                } else {
                    if (!context_.is_module) {
                        string message = "IllegalImportDeclaration";
                        DO(TolerateUnexpectedToken(&lookahead_, ParseMessages::IllegalImportDeclaration))
                    }
                    DO(ParseImportDeclaration(statement))
                }
            } else if (lookahead_.value_ == u"const") {
                bool in_for = false;
                DO(ParseLexicalDeclaration(statement, in_for))
            } else if (lookahead_.value_ == u"function") {
                DO(ParseFunctionDeclaration(false, statement))
            } else if (lookahead_.value_ == u"class") {
                DO(ParseClassDeclaration(false, statement))
            } else if (lookahead_.value_ == u"let") {
                if (IsLexicalDeclaration()) {
                    bool in_for = false;
                    DO(ParseLexicalDeclaration(statement, in_for))
                } else {
                    DO(ParseStatement(statement))
                }
            } else {
                DO(ParseStatement(statement))
            }
        } else {
            DO(ParseStatement(statement))
        }

        ptr = statement;
        return true;
    }

    template <typename NodePtr>
    bool Parser::ParseForStatement(NodePtr &ptr) {
        bool for_in = true;

        std::optional<Sp<SyntaxNode>> init;
        std::optional<Sp<Expression>> test;
        std::optional<Sp<Expression>> update;
        Sp<SyntaxNode> left;
        Sp<SyntaxNode> right;
        auto marker = CreateNode();

        DO(ExpectKeyword(u"for"))
        DO(Expect(u'('))

        if (Match(u';')) {
            NextToken();
        } else {
            if (MatchKeyword(u"var")) {
                auto marker = CreateNode();
                NextToken();

                auto prev_allow_in = context_.allow_in;
                context_.allow_in = true;
                std::vector<Sp<VariableDeclarator>> declarations;
                DO(ParseVariableDeclarationList(for_in, declarations))
                context_.allow_in = prev_allow_in;

                if (declarations.size() == 1 && MatchKeyword(u"in")) {
                    auto decl = declarations[0];
                    if (decl->init && (decl->id->type == SyntaxNodeType::ArrayPattern || decl->id->type == SyntaxNodeType::ObjectPattern || context_.strict_)) {
                        LogError("ForInOfLoopInitializer");
                    }
                    auto node = Alloc<VariableDeclaration>();
                    node->kind = VarKind::Var;
                    Finalize(marker, node, init);
                    NextToken();
                    left = *init;
                    DO(ParseExpression(right))
                    init.reset();
                } else if (declarations.size() == 1 && !declarations[0]->init && MatchKeyword(u"of")) {
                    auto node = Alloc<VariableDeclaration>();
                    node->declarations = declarations;
                    node->kind = VarKind::Var;
                    Finalize(marker, node, init);
                    NextToken();
                    left = *init;
                    DO(ParseAssignmentExpression(right))
                    init.reset();
                    for_in = false;
                } else {
                    auto node = Alloc<VariableDeclaration>();
                    node->declarations = declarations;
                    node->kind = VarKind::Var;
                    Finalize(marker, node, init);
                    DO(Expect(u';'))
                }
            } else if (MatchKeyword(u"const") || MatchKeyword(u"let")) {
                auto marker = CreateNode();
                Token token;
                NextToken(&token);
                VarKind kind;
                if (token.value_ == u"const") {
                    kind = VarKind::Const;
                } else if (token.value_ == u"let") {
                    kind = VarKind::Let;
                }

                if (!context_.strict_ && lookahead_.value_ == u"in") {
                    auto node = Alloc<Identifier>();
                    node->name = token.value_;
                    Finalize(marker, node, init);
                    NextToken();
                    left = *init;
                    DO(ParseExpression(right))
                    init.reset();
                } else {
                    auto prev_allow_in = context_.allow_in;
                    context_.allow_in = false;
                    std::vector<Sp<VariableDeclarator>> declarations;
                    DO(ParseBindingList(VarKind::Const, declarations))
                    context_.allow_in = prev_allow_in;

                    if (declarations.size() == 1 && !declarations[0]->init && MatchKeyword(u"in")) {
                        auto node = Alloc<VariableDeclaration>();
                        node->declarations = declarations;
                        node->kind = kind;
                        Finalize(marker, node, init);
                        NextToken();
                        left = *init;
                        DO(ParseExpression(right))
                        init.reset();
                    } else if (declarations.size() == 1 && !declarations[0]->init && MatchContextualKeyword(u"of")) {
                        auto node = Alloc<VariableDeclaration>();
                        node->declarations = declarations;
                        node->kind = kind;
                        Finalize(marker, node, init);
                        NextToken();
                        left = *init;
                        DO(ParseAssignmentExpression(right))
                        init.reset();
                        for_in = false;
                    } else {
                        DO(ConsumeSemicolon())
                        auto node = Alloc<VariableDeclaration>();
                        node->declarations = declarations;
                        node->kind = kind;
                        Finalize(marker, node, init);
                    }
                }
            } else {
                auto init_start_token = lookahead_;
                auto start_marker = CreateNode();
                auto prev_allow_in = context_.allow_in;
                context_.allow_in = false;
                DO(InheritCoverGrammar([this, &init] {
                    return ParseAssignmentExpression(init);
                }));
                context_.allow_in = prev_allow_in;

                if (MatchKeyword(u"in")) {
                    if (!context_.is_assignment_target || (*init)->type == SyntaxNodeType::AssignmentExpression) {
                        LogError("InvalidLHSInForIn");
                    }

                    NextToken();
                    DO(ReinterpretExpressionAsPattern(*init))
                    left = *init;
                    DO(ParseExpression(right))
                    init.reset();
                } else if (MatchContextualKeyword(u"of")) {
                    if (!context_.is_assignment_target || (*init)->type == SyntaxNodeType::AssignmentExpression) {
                        LogError("InvalidLHSInForIn");
                    }

                    NextToken();
                    DO(ReinterpretExpressionAsPattern(*init))
                    left = *init;
                    DO(ParseAssignmentExpression(right))
                    init.reset();
                    for_in = false;
                } else {
                    if (Match(u',')) {
                        std::vector<Sp<SyntaxNode>> init_seq;
                        init_seq.push_back(*init);

                        while (Match(u',')) {
                            NextToken();
                            Sp<Expression> node;
                            DO(IsolateCoverGrammar([this, &node] {
                                return ParseAssignmentExpression(node);
                            }));
                            init_seq.push_back(node);
                        }

                        Sp<SequenceExpression> node;
                        Finalize(start_marker, node, *init);
                    }
                    DO(Expect(u';'))
                }
            }
        }

        if (!left) {
            if (!Match(u';')) {
                DO(ParseExpression(test))
            }
            DO(Expect(u';'))
            if (!Match(u')')) {
                DO(ParseExpression(update))
            }
        }

        Sp<Statement> body;
        if (!Match(u')') && config_.tolerant) {
            Token tok;
            NextToken(&tok);
            UnexpectedToken(&tok);
            auto node = Alloc<EmptyStatement>();
            Finalize(CreateNode(), node, body);
        } else {
            DO(Expect(u')'))

            auto prev_in_iter = context_.in_iteration;
            context_.in_iteration = true;
            DO(IsolateCoverGrammar([this, &body] {
                return ParseStatement(body);
            }))
            context_.in_iteration = prev_in_iter;
        }

        if (!left) {
            auto node = Alloc<ForStatement>();
            node->init = init;
            node->test = test;
            node->update = update;
            node->body = body;
            return Finalize(marker, node, ptr);
        } else if (for_in) {
            auto node = Alloc<ForInStatement>();
            node->left = left;
            node->right = right;
            node->body = body;
            return Finalize(marker, node, ptr);
        } else {
            auto node = Alloc<ForOfStatement>();
            node->left = left;
            node->right = right;
            node->body = body;
            return Finalize(marker, node, ptr);
        }
    }

    template <typename NodePtr>
    bool Parser::ParseTryStatement(NodePtr &ptr) {
        auto marker = CreateNode();
        DO(ExpectKeyword(u"try"))
        auto node = Alloc<TryStatement>();

        DO(ParseBlock(node->block))
        if (MatchKeyword(u"catch")) {
            DO(ParseCatchClause(*node->handler))
        }
        if (MatchKeyword(u"finally")) {
            DO(ParseFinallyClause(*node->finalizer))
        }

        return Finalize(marker, node, ptr);
    }

    template <typename NodePtr>
    bool Parser::ParseDirectivePrologues(std::vector<NodePtr> &ptr_vec) {
        std::unique_ptr<Token> first_restrict;

        Token token;
        while (true) {
            token = lookahead_;
            if (token.type_ != JsTokenType::StringLiteral) {
                break;
            }

            Sp<Statement> statement;
            DO(ParseDirective(statement))
            ptr_vec.push_back(statement);
            if (statement->type != SyntaxNodeType::Directive) {
                break;
            }
            auto directive_ = reinterpret_cast<Directive*>(statement.get());

            if (directive_->directive == u"use strict") {
                context_.strict_ = true;
                if (first_restrict) {
                    DO(TolerateUnexpectedToken(first_restrict.get(), ParseMessages::StrictOctalLiteral))
                }
                if (!context_.allow_strict_directive) {
                    DO(TolerateUnexpectedToken(&token, ParseMessages::IllegalLanguageModeDirective))
                }
            } else {
                if (!first_restrict && token.octal_) {
                    first_restrict = make_unique<Token>(token);
                }
            }
        }

        return true;
    }

    bool Parser::MatchImportCall() {
        bool match = MatchKeyword(u"import");
        if (match) {
            auto state = scanner_->SaveState();
            std::vector<Comment> comments;
            Token next;
            scanner_->Lex(next);
            scanner_->ScanComments(comments);
            scanner_->RestoreState(state);
            match = (next.type_ == JsTokenType::Punctuator) && (next.value_ == u"(");
        }
        return match;
    }

    template <typename NodePtr>
    bool Parser::ParseImportCall(NodePtr& ptr) {
        auto marker = CreateNode();
        auto node = Alloc<Import>();
        DO(ExpectKeyword(u"import"))
        return Finalize(marker, node, ptr);
    }

    template <typename NodePtr>
    bool Parser::ParseDirective(NodePtr &ptr) {
        auto token = lookahead_;

        auto marker = CreateNode();
        Sp<Expression> expr;
        DO(ParseExpression(expr))
        UString directive;
        if (expr->type == SyntaxNodeType::Literal) {
            directive = token.value_.substr(1, token.value_.size() - 1);
        }
        DO(ConsumeSemicolon())

        if (!directive.empty()) {
            auto node = Alloc<Directive>();
            node->expression = expr;
            node->directive = directive;
            return Finalize(marker, node, ptr);
        } else {
            auto node = Alloc<ExpressionStatement>();
            node->expression = expr;
            return Finalize(marker, node, ptr);
        }
    }

    template <typename NodePtr>
    bool Parser::ParseExpression(NodePtr &ptr) {
        auto start_token = lookahead_;
        auto start_marker = CreateNode();
        Sp<Expression> expr;
        DO(IsolateCoverGrammar([this, &expr] {
            return ParseAssignmentExpression(expr);
        }));

        if (Match(u',')) {
            std::vector<Sp<Expression>> expressions;
            expressions.push_back(expr);
            while (lookahead_.type_ != JsTokenType::EOF_) {
                if (!Match(u',')) {
                    break;
                }
                NextToken();
                Sp<Expression> node;
                DO(IsolateCoverGrammar([this, &node] {
                    return ParseAssignmentExpression(node);
                }))
                expressions.push_back(node);
            }

            auto node = Alloc<SequenceExpression>();
            node->expressions = expressions;
            Finalize(start_marker, node, expr);
        }

        ASSERT_NOT_NULL(expr, "ParseExpression")
        ptr = move(expr);
        return true;
    }

    template <typename NodePtr>
    bool Parser::ParseFunctionSourceElements(NodePtr &ptr) {
        auto start_marker = CreateNode();

        vector<Sp<SyntaxNode>> body;
        DO(ParseDirectivePrologues(body))
        DO(Expect(u'{'))

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

            Sp<SyntaxNode> temp;
            DO(ParseStatementListItem(temp))
            body.push_back(move(temp));
        }

        DO(Expect('}'))

        context_.label_set = move(prev_label_set);
        context_.in_iteration = prev_in_iteration;
        context_.in_switch = prev_in_switch;
        context_.in_function_body = prev_in_fun_body;

        auto node = Alloc<BlockStatement>();
        node->body = move(body);

        return Finalize(start_marker, node, ptr);
    }

    template <typename NodePtr>
    bool Parser::ParseAssignmentExpression(NodePtr &ptr) {
        Sp<Expression> expr;

        if (context_.allow_yield && MatchKeyword(u"yield")) {
            DO(ParseYieldExpression(expr))
        } else {
            auto start_marker = CreateNode();
            auto token = lookahead_;
            DO(ParseConditionalExpression(expr))

            if (token.type_ == JsTokenType::Identifier && (token.line_number_ == lookahead_.line_number_) && token.value_ == u"async") {
                if (lookahead_.type_ == JsTokenType::Identifier || MatchKeyword(u"yield")) {
                    Sp<Expression> arg;
                    DO(ParsePrimaryExpression(arg))
                    DO(ReinterpretExpressionAsPattern(arg))

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
                        DO(TolerateUnexpectedToken(&lookahead_))
                    }
                    context_.first_cover_initialized_name_error.reset();

                    bool prev_strict = context_.strict_;
                    bool prev_allow_strict_directive = context_.allow_strict_directive;
                    context_.allow_strict_directive = list.simple;

                    bool prev_allow_yield = context_.allow_yield;
                    bool prev_await = context_.await;
                    context_.allow_yield = true;
                    context_.await = is_async;

                    auto marker = CreateNode();
                    DO(Expect(u"=>"))
                    Sp<SyntaxNode> body;

                    if (Match(u'{')) {
                        bool prev_allow_in = context_.allow_in;
                        context_.allow_in = true;
                        DO(ParseFunctionSourceElements(body))
                        context_.allow_in = prev_allow_in;
                    } else {
                        DO(IsolateCoverGrammar([this, &body] {
                            return ParseAssignmentExpression(body);
                        }))
                    }

                    bool expression = body->type != SyntaxNodeType::BlockStatement;

                    if (context_.strict_ && list.first_restricted) {
                        Token temp = *list.first_restricted;
                        UnexpectedToken(&temp, list.message);
                        return false;
                    }
                    if (context_.strict_ && list.stricted) {
                        Token temp = *list.stricted;
                        DO(TolerateUnexpectedToken(&temp, list.message))
                    }

                    if (is_async) {
                        auto node = Alloc<AsyncArrowFunctionExpression>();
                        node->params = list.params;
                        node->body = body;
                        node->expression = expression;
                        Finalize(marker, node, expr);
                    } else {
                        auto node = Alloc<ArrowFunctionExpression>();
                        node->params = list.params;
                        node->body = body;
                        node->expression = expression;
                        Finalize(marker, node, expr);
                    }

                    context_.strict_ = prev_strict;
                    context_.allow_strict_directive = prev_allow_strict_directive;
                    context_.allow_yield = prev_allow_yield;
                    context_.await = prev_await;
                }
            } else {

                if (MatchAssign()) {
                    if (!context_.is_assignment_target) {
                        DO(TolerateError(ParseMessages::InvalidLHSInAssignment))
                    }

                    if (context_.strict_ && expr->type == SyntaxNodeType::Identifier) {
                        auto id = dynamic_pointer_cast<Identifier>(expr);
                        if (scanner_->IsRestrictedWord(id->name)) {
                            DO(TolerateUnexpectedToken(&token, ParseMessages::StrictLHSAssignment))
                        }
                        if (scanner_->IsStrictModeReservedWord(id->name)) {
                            DO(TolerateUnexpectedToken(&token, ParseMessages::StrictReservedWord))
                        }
                    }

                    if (!Match(u'=')) {
                        context_.is_assignment_target = false;
                        context_.is_binding_element = false;
                    } else {
                        DO(ReinterpretExpressionAsPattern(expr))
                    }

                    DO(NextToken(&token))
                    auto operator_ = token.value_;
                    Sp<Expression> right;
                    DO(IsolateCoverGrammar([this, &right] {
                        return ParseAssignmentExpression(right);
                    }))
                    auto temp = Alloc<AssignmentExpression>();
                    temp->operator_ = operator_;
                    temp->left = expr;
                    temp->right = right;
                    Finalize(start_marker, temp, expr);
                    context_.first_cover_initialized_name_error.reset();
                }
            }

        }

        // TODO: wait to complete
        ASSERT_NOT_NULL(expr, "ParseAssignmentExpression")
        ptr = move(expr);
        return true;
    }

    template <typename NodePtr>
    bool Parser::ParseWithStatement(NodePtr &ptr) {
        if (context_.strict_) {
            LogError("StrictModeWith");
        }

        auto marker = CreateNode();
        auto node = Alloc<WithStatement>();

        DO(ExpectKeyword(u"with"))
        DO(Expect(u'('))

        DO(ParseExpression(node->object))

        if (!Match(u')') && config_.tolerant) {
            Token token;
            DO(NextToken(&token))
            UnexpectedToken(&token);
            auto empty = Alloc<EmptyStatement>();
            Finalize(marker, empty, node->body);
        } else {
            DO(Expect(u')'))
            DO(ParseStatement(node->body))
        }

        return Finalize(marker, node, ptr);
    }

    template <typename NodePtr>
    bool Parser::ParseTemplateHead(NodePtr &ptr) {
        return false;
    }

    template <typename NodePtr>
    bool Parser::ParseImportDeclaration(NodePtr &ptr) {
        if (context_.in_function_body) {
            LogError("IllegalImportDeclaration");
            return false;
        }

//        auto marker = CreateNode();
//        DO(ExpectKeyword(u"import"))
//
//        Sp<Literal> src;
//        std::vector<Sp<ImportDeSpecifier>> specifiers;
//        if (lookahead_.type_ == JsTokenType::StringLiteral) {
//            DO(ParseModuleSpecifier(src))
//        } else {
//            if (Match(u'{')) {
//                ParseName
//            }
//        }
        return false;
    }

    template <typename NodePtr>
    bool Parser::ParseBindingList(VarKind kind, NodePtr &ptr) {
        // TODO
        return false;
    }

    template <typename NodePtr>
    bool Parser::ParseGroupExpression(NodePtr& ptr) {
        // TODO
        return false;
    }

    template <typename NodePtr>
    bool Parser::ParseUpdateExpression(NodePtr &ptr) {
        Sp<Expression> expr;
        auto start_marker = CreateNode();

        if (Match(u"++") || Match(u"--")) {
            auto marker = CreateNode();
            Token token;
            DO(NextToken(&token))
            DO(InheritCoverGrammar([this, &expr] {
                return ParseUnaryExpression(expr);
            }))
            if (context_.strict_ && expr->type == SyntaxNodeType::Identifier) {
                auto id = dynamic_pointer_cast<Identifier>(expr);
                if (scanner_->IsRestrictedWord(id->name)) {
                    DO(TolerateError(ParseMessages::StrictLHSPrefix));
                }
            }
            if (!context_.is_assignment_target) {
                DO(TolerateError(ParseMessages::InvalidLHSInAssignment));
            }
            auto node = Alloc<UpdateExpression>();
            node->prefix = true;
            node->operator_ = token.value_;
            node->argument = expr;
            Finalize(marker, node, expr);
            context_.is_assignment_target = false;
            context_.is_binding_element = false;
        } else {
            DO(InheritCoverGrammar([this, &expr] {
                return ParseLeftHandSideExpressionAllowCall(expr);
            }))
            if (!has_line_terminator_ && lookahead_.type_ == JsTokenType::Punctuator) {
                if (Match(u"++") || Match(u"--")) {
                    if (context_.strict_ && expr->type == SyntaxNodeType::Identifier) {
                        auto id = dynamic_pointer_cast<Identifier>(expr);
                        if (scanner_->IsRestrictedWord(id->name)) {
                            DO(TolerateError(ParseMessages::StrictLHSPostfix))
                        }
                    }
                    if (!context_.is_assignment_target) {
                        DO(TolerateError(ParseMessages::InvalidLHSInAssignment))
                    }
                    context_.is_assignment_target = false;
                    context_.is_binding_element = false;
                    auto node = Alloc<UpdateExpression>();
                    node->prefix = false;
                    Token token;
                    DO(NextToken(&token))
                    node->operator_ = token.value_;
                    node->argument = expr;
                    Finalize(start_marker, node, expr);
                }
            }
        }

        ptr = move(expr);
        return true;
    }

    template <typename NodePtr>
    bool Parser::ParseAwaitExpression(NodePtr &ptr) {
        auto marker = CreateNode();
        DO(NextToken())
        auto node = Alloc<AwaitExpression>();
        DO(ParseUnaryExpression(node->argument))
        return Finalize(marker, node, ptr);
    }

    template <typename NodePtr>
    bool Parser::ParseUnaryExpression(NodePtr &ptr) {
        Sp<Expression> expr;

        if (
            Match(u'+') || Match(u'-') || Match(u'~') || Match(u'!') ||
            MatchKeyword(u"delete") || MatchKeyword(u"void") || MatchKeyword(u"typeof")
        ) {
            auto marker = CreateNode();
            Token token = lookahead_;
            DO(NextToken())
            DO(InheritCoverGrammar([this, &expr] {
                return ParseUnaryExpression(expr);
            }))
            auto node = Alloc<UnaryExpression>();
            node->operator_ = token.value_;
            node->argument = expr;
            Finalize(marker, node, expr);
            if (context_.strict_ && node->operator_ == u"delete" && node->argument->type == SyntaxNodeType::Identifier) {
                DO(TolerateError(ParseMessages::StrictDelete))
            }
            context_.is_assignment_target = false;
            context_.is_binding_element = false;
        } else if (context_.await && MatchContextualKeyword(u"await")) {
            DO(ParseAwaitExpression(expr))
        } else {
            DO(ParseUpdateExpression(expr))
        }

        ptr = move(expr);
        return true;
    }

    template <typename NodePtr>
    bool Parser::ParseBinaryExpression(NodePtr &ptr) {
        auto start_token = lookahead_;

        Sp<Expression> expr;
        DO(InheritCoverGrammar([this, &expr] {
            return ParseExponentiationExpression(expr);
        }))

        Token token = lookahead_;
        int prec = BinaryPrecedence(token);
        if (prec > 0) {
            DO(NextToken())

            context_.is_assignment_target = false;
            context_.is_binding_element = false;

            stack<Token> markers;
            markers.push(start_token);
            markers.push(lookahead_);

            Sp<Expression> left = expr;
            Sp<Expression> right;
            DO(IsolateCoverGrammar([this, &right] {
                return ParseExponentiationExpression(right);
            }))

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
                Token next_;
                DO(NextToken(&next_))
                op_stack.push(next_.value_);
                precedences.push(prec);
                markers.push(lookahead_);
                Sp<Expression> temp_expr;
                DO(IsolateCoverGrammar([this, &temp_expr] {
                    return ParseExponentiationExpression(temp_expr);
                }))
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
                Finalize(CreateNode(), node, expr);
                last_marker = marker;
            }
        }

        ptr = move(expr);
        return true;
    }

    template <typename NodePtr>
    bool Parser::ParseExponentiationExpression(NodePtr& ptr) {
        auto start = CreateNode();

        Sp<Expression> expr;
        DO(InheritCoverGrammar([this, &expr] {
            return ParseUnaryExpression(expr);
        }))
        if (expr->type != SyntaxNodeType::UnaryExpression && Match(u"**")) {
            DO(NextToken())
            context_.is_assignment_target = false;
            context_.is_binding_element = false;
            auto node = Alloc<BinaryExpression>();
            node->left = expr;
            DO(IsolateCoverGrammar([this, &node] {
                return ParseExponentiationExpression(node->right);
            }))
            node->operator_ = u"**";
            Finalize(start, node, expr);
        }

        ptr = move(expr);
        return true;
    }

    template <typename NodePtr>
    bool Parser::ParseExportDeclaration(NodePtr &ptr) {
        // TODO
        return false;
    }

    template <typename NodePtr>
    bool Parser::ParseClassElementList(std::vector<NodePtr> &vec) {
        // TODO
        return false;
    }

    template <typename NodePtr>
    bool Parser::ParseLexicalDeclaration(NodePtr &ptr, bool& in_for) {
        // TODO
        return false;
    }

    template <typename NodePtr>
    bool Parser::ParseVariableDeclarationList(bool in_for, NodePtr &ptr) {
        // TODO
        return false;
    }

    template <typename NodePtr>
    bool Parser::ReinterpretExpressionAsPattern(NodePtr& ptr) {
        // TODO
        return false;
    }

    template <typename NodePtr>
    bool Parser::ReinterpretAsCoverFormalsList(NodePtr& ptr, FormalParameterOptions& list) {
        // TODO
        return false;
    }

    template <typename NodePtr>
    bool Parser::ParseNewExpression(NodePtr &ptr) {
        auto start_marker = CreateNode();

        Sp<Identifier> id;
        DO(ParseIdentifierName(id))
        if (id->name == u"new") {
            LogError("New expression must start with `new`");
            return false;
        }

        Sp<Expression> expr;

        if (Match(u'.')) {
            DO(NextToken())
            if (lookahead_.type_ == JsTokenType::Identifier && context_.in_function_body && lookahead_.value_ == u"target") {
                auto node = Alloc<MetaProperty>();
                DO(ParseIdentifierName(node->property))
                node->meta = id;
                expr = node;
            } else {
                UnexpectedToken(&lookahead_);
                return false;
            }
        } else if (MatchKeyword(u"import")) {
            UnexpectedToken(&lookahead_);
            return false;
        } else {
            auto node = Alloc<NewExpression>();
            DO(IsolateCoverGrammar([this, &node] {
                return ParseLeftHandSideExpression(node->callee);
            }))
            if (Match(u'(')) {
                DO(ParseArguments(node->arguments))
            }
            expr = node;
            context_.is_assignment_target = false;
            context_.is_binding_element = false;
        }

        return Finalize(start_marker, expr, ptr);
    }

    template <typename NodePtr>
    bool Parser::ParseSuper(NodePtr &ptr) {
        auto start_marker = CreateNode();

        DO(ExpectKeyword(u"super"))
        if (!Match(u'[') && !Match(u'.')) {
            UnexpectedToken(&lookahead_);
            return false;
        }

        auto node = Alloc<Super>();
        return Finalize(start_marker, node, ptr);
    }

    template <typename NodePtr>
    bool Parser::ParseLeftHandSideExpression(NodePtr &ptr) {
        if (!context_.allow_in) {
            LogError("callee of new expression always allow in keyword.");
            return false;
        }

        auto start_marker = CreateNode();
        Sp<Expression> expr;
        if (MatchKeyword(u"super") && context_.in_function_body) {
            DO(ParseSuper(expr))
        } else {
            DO(InheritCoverGrammar([this, &expr] {
                if (MatchKeyword(u"new")) {
                    DO(ParseNewExpression(expr))
                } else {
                    DO(ParsePrimaryExpression(expr))
                }
                return true;
            }))
        }

        while (true) {
            if (Match(u'[')) {
                context_.is_binding_element = false;
                context_.is_assignment_target = true;
                DO(Expect(u'['))
                auto node = Alloc<ComputedMemberExpression>();
                DO(IsolateCoverGrammar([this, &node] {
                    return ParseExpression(node->property);
                }))
                DO(Expect(u']'))
                node->object = move(expr);
                Finalize(start_marker, node, expr);
            } else if (Match(u'.')) {
                context_.is_binding_element = false;
                context_.is_assignment_target = true;
                DO(Expect('.'))
                auto node = Alloc<StaticMemberExpression>();
                DO(ParseIdentifierName(node->property))
                node->object = move(expr);
                Finalize(start_marker, node, expr);
            } else if (lookahead_.type_ == JsTokenType::Template && lookahead_.head_) {
                auto node = Alloc<TaggedTemplateExpression>();
                DO(ParseTemplateLiteral(node->quasi))
                node->tag = move(expr);
                Finalize(start_marker, node, expr);
            } else {
                break;
            }
        }

        ptr = move(expr);
        return true;
    }

    template <typename NodePtr>
    bool Parser::ParseLeftHandSideExpressionAllowCall(NodePtr &ptr) {
        Token start_token = lookahead_;
        auto start_marker = CreateNode();
        bool maybe_async = MatchContextualKeyword(u"async");

        bool prev_allow_in = context_.allow_in;
        context_.allow_in = true;

        Sp<Expression> expr;
        if (MatchKeyword(u"super") && context_.in_function_body) {
            auto node = Alloc<Super>();
            auto marker = CreateNode();
            DO(NextToken())
            Finalize(marker, node, expr);
            if (!Match(u'(') && !Match(u'.') && !Match(u'[')) {
                UnexpectedToken(&lookahead_);
                return false;
            }
        } else {
            if (MatchKeyword(u"new")) {
                DO(InheritCoverGrammar([this, &expr] {
                    return ParseNewExpression(expr);
                }))
            } else {
                DO(InheritCoverGrammar([this, &expr] {
                    return ParsePrimaryExpression(expr);
                }))
            }
        }

        while (true) {
            if (Match(u'.')) {
                context_.is_binding_element = false;
                context_.is_assignment_target = true;
                DO(Expect('.'))
                auto node = Alloc<StaticMemberExpression>();
                DO(ParseIdentifierName(node->property))
                node->object = expr;
                Finalize(start_marker, node, expr);
            } else if (Match(u'(')) {
                bool async_arrow = maybe_async && (start_token.line_number_ == lookahead_.line_number_);
                context_.is_binding_element = false;
                context_.is_assignment_target = false;
                auto node = Alloc<CallExpression>();
                if (async_arrow) {
                    DO(ParseAsyncArguments(node->arguments))
                } else {
                    DO(ParseArguments(node->arguments))
                }
                if (expr->type == SyntaxNodeType::Import && node->arguments.size() != 1) {
                    DO(TolerateError(ParseMessages::BadImportCallArity))
                }
                node->callee = expr;
                Finalize(start_marker, node, expr);
                if (async_arrow && Match(u"=>")) {
                    for (auto &i : node->arguments) {
                        DO(ReinterpretExpressionAsPattern(i))
                    }
                    auto placeholder = Alloc<ArrowParameterPlaceHolder>();
                    placeholder->params = node->arguments;
                    placeholder->async = true;
                    expr = move(placeholder);
                }
            } else if (Match(u'[')) {
                context_.is_binding_element = false;
                context_.is_assignment_target = true;
                DO(Expect(u'['))
                auto node = Alloc<ComputedMemberExpression>();
                DO(IsolateCoverGrammar([this, &node] {
                    return ParseExpression(node->property);
                }))
                DO(Expect(u']'))
                node->object = expr;
                Finalize(start_marker, node, expr);
            } else if (lookahead_.type_ == JsTokenType::Template && lookahead_.head_) {
                auto node = Alloc<TaggedTemplateExpression>();
                DO(ParseTemplateLiteral(node->quasi))
                node->tag = expr;
                Finalize(start_marker, node, expr);
            } else {
                break;
            }
        }
        context_.allow_in = prev_allow_in;

        ptr = move(expr);
        return true;
    }

    template <typename NodePtr>
    bool Parser::ParseArguments(std::vector<Sp<NodePtr>> &ptr) {
        DO(Expect(u'('))
        if (!Match(u')')) {
            Sp<NodePtr> expr;
            while (true) {
                if (Match(u"...")) {
                    DO(ParseSpreadElement(expr))
                } else {
                    DO(IsolateCoverGrammar([this, &expr] {
                        return ParseAssignmentExpression(expr);
                    }))
                }
                ptr.push_back(expr);
                if (Match(u')')) {
                    break;
                }
                DO(ExpectCommaSeparator())
                if (Match(u')')) {
                    break;
                }
            }
        }
        DO(Expect(u')'))
        return true;
    }

    template <typename NodePtr>
    bool Parser::ParseAsyncArguments(std::vector<NodePtr>& ptr) {
        return false;
    }

    template <typename NodePtr>
    bool Parser::ParseClassDeclaration(bool identifier_is_optional, NodePtr &ptr) {
        auto marker = CreateNode();

        bool prev_strict = context_.strict_;
        context_.strict_ = true;
        DO(ExpectKeyword(u"class"))

        auto node = Alloc<ClassDeclaration>();
        if (identifier_is_optional && (lookahead_.type_ != JsTokenType::Identifier)) {
            // nothing
        } else {
            DO(ParseVariableIdentifier(VarKind::Invalid, *node->id))
        }

        if (MatchKeyword(u"extends")) {
            DO(NextToken())
            DO(IsolateCoverGrammar([this, &node] {
                Token token = lookahead_;
                Sp<Expression> temp_node;
                DO(ParseLeftHandSideExpressionAllowCall(temp_node))
                if (temp_node->type != SyntaxNodeType::Identifier) {
                    UnexpectedToken(&token);
                    return false;
                }
                Sp<Identifier> id = dynamic_pointer_cast<Identifier>(temp_node);
                node->super_class = id;
                return true;
            }))
        }
        DO(ParseClassBody(node->body))
        context_.strict_ = prev_strict;

        return Finalize(marker, node, ptr);
    }

}
