//
// Created by Duzhong Chen on 2019/9/3.
//
#pragma once

#include "parser_common.h"

namespace parser {

    class Parser final: public ParserCommon {
    public:

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
        bool ReinterpretExpressionAsPattern(NodePtr ptr);

        template <typename NodePtr>
        bool ParseGroupExpression(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseArguments(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseIdentifierName(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseNewExpression(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseAsyncArgument(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseAsyncArguments(NodePtr& ptr);

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
        bool ParseBindingList(NodePtr& ptr);

        bool IsLexicalDeclaration();

        template <typename NodePtr>
        bool ParseLexicalDeclaration(NodePtr& ptr);

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

        bool ParseFormalParameters(bool first_restricted, FormalParameterOptions& option);
        bool ParseFormalParameter(FormalParameterOptions& option);

        template <typename NodePtr>
        bool ParseRestElement(std::vector<Sp<SyntaxNode>>& params, NodePtr& ptr);

        template <typename NodePtr>
        bool ParsePattern(std::vector<Sp<SyntaxNode>>& params, VarKind  kind, NodePtr& ptr);

        template <typename NodePtr>
        bool ParsePatternWithDefault(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseVariableIdentifier(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseVariableDeclaration(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseVariableDeclarationList(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseVariableStatement(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseEmptyStatement(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseExpressionStatement(NodePtr& ptr);

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
        bool ParseFunctionDeclaration(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseFunctionExpression(NodePtr& expr);

        template <typename NodePtr>
        bool ParseDirective(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseDirectivePrologues(NodePtr& ptr);

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
        bool ParseClassElementList(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseClassBody(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseClassDeclaration(NodePtr& ptr);

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

        bool ParseTemplateLiteral(Expression*& expr);
        bool ParseGroupExpression(Expression*& expr);


        bool ParseObjectInitializer(Expression*& expr);
        bool ParseIdentifierName(Expression*& expr);
        bool ParseClassExpression(Expression*& expr);
        bool MatchImportCall();
        bool ParseImportCall(Expression*& expr);

        bool MatchAsyncFunction();


        ~Parser() = default;

    };


//    template<typename FromT, typename ToT>
//    bool Finalize(const Marker& marker, FromT from, ToT& to);
    template<typename FromT, typename ToT>
    bool Parser::Finalize(const Parser::Marker &marker, FromT from, ToT& to) {
        static_assert(std::is_convertible<FromT, ToT>::value, "FromT can not convert to ToT");

        if (config_.range) {
            from->range_ = std::make_pair(marker.index, last_marker_.index);
        }

        if (config_.loc) {
            from->loc_.start_ = Position {
                marker.line,
                marker.column,
            };
            from->loc_.end_ = Position {
                last_marker_.line,
                last_marker_.column,
            };
        }

        to = from;
        return true;
    }


    template <typename NodePtr>
    bool Parser::ParsePrimaryExpression(NodePtr& expr) {
        static_assert(std::is_convertible<Expression*, NodePtr>::value, "NodePtr can not convert to AstNode*");

        expr = nullptr;
        auto marker = CreateNode();
        Token token;

        switch (lookahead_.type_) {
            case JsTokenType::Identifier: {
                if ((context_.is_module || context_.await) && lookahead_.value_ == U("await")) {
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
                if (!context_.strict_ && context_.allow_yield && MatchKeyword(U("yield"))) {
                    return ParseIdentifierName(expr);
                } else if (!context_.strict_ && MatchKeyword(U("let"))) {
                    DO(NextToken(&token));
                    auto id = Alloc<Identifier>();
                    id->name = token.value_;
                    return Finalize(marker, id, expr);
                } else {
                    context_.is_assignment_target = false;
                    context_.is_binding_element = false;
                    if (MatchKeyword(U("function"))) {
                        ParseFunctionExpression(expr);
                    } else if (MatchKeyword(U("this"))) {
                        DO(NextToken(&token))
                        auto th = Alloc<ThisExpression>();
                        return Finalize(marker, th, expr);
                    } else if (MatchKeyword(U("class"))) {
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
        static_assert(std::is_convertible<SpreadElement*, NodePtr>::value, "NodePtr can not accept SpreadElement*");
        auto marker = CreateNode();

        DO(Expect(U("...")))

        auto node = Alloc<SpreadElement>();

        DO(InheritCoverGrammar([this, &node]() {
            return ParseAssignmentExpression(node->argument);
        }));

        return Finalize(marker, node, expr);
    }

    template <typename NodePtr>
    bool Parser::ParseArrayInitializer(NodePtr &expr) {
        static_assert(std::is_convertible<ArrayExpression*, NodePtr>::value, "NodePtr can not accept ArrayExpression*");
        auto marker = CreateNode();
        auto node = Alloc<ArrayExpression>();
        Sp<Expression> element = nullptr;

        DO(Expect('['))
        while (!Match(']')) {
            if (Match(',')) {
                NextToken();
                node->elements.push_back(nullptr);
            } else if (Match(U("..."))) {
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
        DO(ParseFormalParameters(false, options))
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
        DO(ParseFormalParameters(false, options))
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
                if (token.value_ == U("[")) {
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
            is_async = !has_line_terminator_ && (id == U("async")) &&
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

    bool Parser::ParseFormalParameters(bool first_restricted, parser::Parser::FormalParameterOptions &options) {

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
        if (Match(U("..."))) {

        }

        return true;
    }

    template <typename NodePtr>
    bool Parser::ParseRestElement(std::vector<Sp<SyntaxNode>>& params, NodePtr &ptr) {
        auto marker = CreateNode();
        auto node = Alloc<RestElement>();

        DO(Expect(U("...")))
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

}
