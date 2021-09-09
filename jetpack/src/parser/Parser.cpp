//
// Created by Duzhong Chen on 2019/9/20.
//

#include "Parser.hpp"
#include "JSXParser.h"
#include "TypescriptParser.h"
#include "ConstantFolding.h"
#include "tokenizer/Token.h"

namespace jetpack::parser {
    using namespace std;

    Parser::Parser(std::shared_ptr<ParserContext> state):
    ParserCommon(state) {

        ctx->start_marker_ = ParserContext::Marker {
            0,
            ctx->scanner_->LineNumber(),
            0,
        };

        ctx->last_marker_ = ParserContext::Marker {
            0,
            ctx->scanner_->LineNumber(),
            0,
        };

        NextToken();

        ctx->last_marker_ = ParserContext::Marker {
            ctx->scanner_->Index(),
            ctx->scanner_->LineNumber(),
            ctx->scanner_->Column(),
        };
    }

    Sp<Pattern> Parser::ReinterpretExpressionAsPattern(const Sp<SyntaxNode> &expr) {
        switch (expr->type) {
            case SyntaxNodeType::Identifier:
                return dynamic_pointer_cast<Identifier>(expr);

            case SyntaxNodeType::RestElement:
                return dynamic_pointer_cast<RestElement>(expr);

            case SyntaxNodeType::MemberExpression:
                return dynamic_pointer_cast<MemberExpression>(expr);

            case SyntaxNodeType::AssignmentPattern:
                return dynamic_pointer_cast<AssignmentPattern>(expr);

            case SyntaxNodeType::SpreadElement: {
                auto that = dynamic_pointer_cast<SpreadElement>(expr);
                auto node = Alloc<RestElement>();
                node->argument = ReinterpretExpressionAsPattern(that->argument);
                return node;
            }

            case SyntaxNodeType::ArrayExpression: {
                auto that = dynamic_pointer_cast<ArrayExpression>(expr);
                auto node = Alloc<ArrayPattern>();
                for (auto& i : that->elements) {
                    if (i.has_value()) {
                        node->elements.push_back(ReinterpretExpressionAsPattern(*i));
                    } else {
                        node->elements.push_back(nullopt);
                    }
                }
                return node;
            }

            case SyntaxNodeType::ObjectExpression: {
                auto that = dynamic_pointer_cast<ObjectExpression>(expr);
                auto node = Alloc<ObjectPattern>();
                for (auto& i : that->properties) {
                    if (i->type == SyntaxNodeType::SpreadElement) {
                        node->properties.push_back(ReinterpretExpressionAsPattern(i));
                    } else {
                        auto prop = dynamic_pointer_cast<Property>(i);
                        auto new_prop = Alloc<Property>();
                        new_prop->key = prop->key;
                        if (prop->value) {
                            new_prop->value = ReinterpretExpressionAsPattern(*prop->value);
                        }
                        node->properties.push_back(move(new_prop));
                    }
                }

                return node;
            }

            case SyntaxNodeType::AssignmentExpression: {
                auto that = dynamic_pointer_cast<AssignmentExpression>(expr);
                auto node = Alloc<AssignmentPattern>();
                node->right = that->right;
                node->left = ReinterpretExpressionAsPattern(that->left);
                return node;
            }

            default:
                Assert(false, "ReinterpretExpressionAsPattern: should not reach here.");
                return nullptr;
        }
    }

    std::optional<ParserCommon::FormalParameterOptions> Parser::ReinterpretAsCoverFormalsList(const Sp<SyntaxNode> &expr) {
        std::vector<Sp<SyntaxNode>> params;
        FormalParameterOptions options;

        bool async_arrow = false;
        switch (expr->type) {
            case SyntaxNodeType::Identifier:
                break;

            case SyntaxNodeType::ArrowParameterPlaceHolder: {
                auto node = dynamic_pointer_cast<ArrowParameterPlaceHolder>(expr);
                params = node->params;
                async_arrow = node->async;
                break;
            }

            default:
                return nullopt;

        }

        options.simple = true;

        for (auto& param : params) {
            if (param->type == SyntaxNodeType::AssignmentPattern) {
                auto node = dynamic_pointer_cast<AssignmentPattern>(param);
                if (node->right->type == SyntaxNodeType::YieldExpression) {
                    auto right = dynamic_pointer_cast<YieldExpression>(node->right);
                    if (right->argument) {
                        ThrowUnexpectedToken(ctx->lookahead_);
                    }
                    auto new_right = Alloc<Identifier>();
                    new_right->name = "yield";
                    node->right = new_right;
                }
            } else if (async_arrow && param->type == SyntaxNodeType::Identifier) {
                auto id = dynamic_pointer_cast<Identifier>(param);
                if (id->name == "await") {
                    ThrowUnexpectedToken(ctx->lookahead_);
                }
            }
            CheckPatternParam(options, param);
        }

        if (ctx->strict_ || !ctx->allow_yield_) {
            for (auto& param : params) {
                if (param->type == SyntaxNodeType::YieldExpression) {
                    ThrowUnexpectedToken(ctx->lookahead_);
                }
            }
        }

        if (options.message == ParseMessages::StrictParamDupe) {
            Token token = ctx->strict_
                ? *options.stricted
                : *options.first_restricted;
            ThrowUnexpectedToken(token, options.message);
        }

        options.params = move(params);
        return options;
    }

    void Parser::CheckPatternParam(parser::ParserCommon::FormalParameterOptions &options,
                                   const Sp<SyntaxNode> &param) {
        switch (param->type) {
            case SyntaxNodeType::Identifier: {
                auto id = dynamic_pointer_cast<Identifier>(param);
                ValidateParam(options, Token(), id->name);
                break;
            }
            case SyntaxNodeType::RestElement: {
                auto rest_element = dynamic_pointer_cast<RestElement>(param);
                CheckPatternParam(options, rest_element->argument);
                break;
            }
            case SyntaxNodeType::AssignmentPattern: {
                auto assignment = dynamic_pointer_cast<AssignmentPattern>(param);
                CheckPatternParam(options, assignment->left);
                break;
            }
            case SyntaxNodeType::ArrayPattern: {
                auto pattern = dynamic_pointer_cast<ArrayPattern>(param);
                for (auto& i : pattern->elements) {
                    if (i.has_value()) {
                        CheckPatternParam(options, *i);
                    }
                }
                break;
            }
            case SyntaxNodeType::ObjectPattern: {
                auto pattern = dynamic_pointer_cast<ObjectPattern>(param);
                for (auto& prop : pattern->properties) {
                    if (prop->type == SyntaxNodeType::RestElement) {
                        auto rest = dynamic_pointer_cast<RestElement>(prop);
                        CheckPatternParam(options, rest);
                    } else {
                        auto property = dynamic_pointer_cast<Property>(prop);
                        if (property->value.has_value()) {
                            CheckPatternParam(options, *property->value);
                        }
                    }
                }
            }
            default:
                break;
        }
    }

    bool Parser::IsLexicalDeclaration() {
        Scanner& scanner = *ctx->scanner_;

        auto state = scanner.SaveState();
        std::vector<Sp<Comment>> comments;
        scanner.ScanComments(comments);
        Token next = scanner.Lex();
        scanner.RestoreState(state);

        return (next.type == JsTokenType::Identifier) ||
               (next.type == JsTokenType::LeftBrace) ||
               (next.type == JsTokenType::LeftBracket) ||
               next.type == JsTokenType::K_Let ||
               next.type == JsTokenType::K_Yield;
    }

    bool Parser::MatchImportCall() {
        Scanner& scanner = *ctx->scanner_;

        bool match = Match(JsTokenType::K_Import);
        if (match) {
            auto state = scanner.SaveState();
            std::vector<Sp<Comment>> comments;
            scanner.ScanComments(comments);
            Token next = scanner.Lex();
            scanner.RestoreState(state);
            match = (next.type == JsTokenType::LeftParen);
        }
        return match;
    }

    bool Parser::IsStartOfExpression() {
        const auto& value = ctx->lookahead_.value;

        if (IsPunctuatorToken(ctx->lookahead_.type)) {
            switch (ctx->lookahead_.type) {
                case JsTokenType::LeftBrace:
                case JsTokenType::RightParen:
                case JsTokenType::LeftBracket:
                case JsTokenType::Plus:
                case JsTokenType::Minus:
                case JsTokenType::Not:
                case JsTokenType::Wave:
                case JsTokenType::Increase:
                case JsTokenType::Decrease:
                case JsTokenType::Div:
                case JsTokenType::DivAssign:
                    return true;

                default:
                    return false;

            }

        }

        if (IsKeywordToken(ctx->lookahead_.type)) {
            switch (ctx->lookahead_.type) {
                case JsTokenType::K_Class:
                case JsTokenType::K_Delete:
                case JsTokenType::K_Function:
                case JsTokenType::K_Let:
                case JsTokenType::K_New:
                case JsTokenType::K_Super:
                case JsTokenType::K_This:
                case JsTokenType::K_Typeof:
                case JsTokenType::K_Void:
                case JsTokenType::K_Yield:
                    return true;

                default:
                    return false;

            }
        }

        return true;
    }

    ParserCommon::FormalParameterOptions Parser::ParseFormalParameters(Scope& scope, optional<Token> first_restricted) {

        FormalParameterOptions options;
        options.simple = true;
        options.first_restricted = move(first_restricted);

        Expect(JsTokenType::LeftParen);
        if (!Match(JsTokenType::RightParen)) {
            while (ctx->lookahead_.type != JsTokenType::EOF_) {
                ParseFormalParameter(scope, options);
                if (Match(JsTokenType::RightParen)) {
                    break;
                }
                Expect(JsTokenType::Comma);
                if (Match(JsTokenType::RightParen)) {
                    break;
                }
            }
        }
        Expect(JsTokenType::RightParen);

        return options;
    }

    void Parser::ParseFormalParameter(Scope& scope, parser::Parser::FormalParameterOptions &option) {
        std::vector<Token> params;
        Sp<SyntaxNode> param;
        if (Match(JsTokenType::Spread)) {
            param = ParseRestElement(scope, params);
        } else {
            param = ParsePatternWithDefault(scope, params, VarKind::Invalid);
        }

        for (auto& i : params) {
            ValidateParam(option, i, i.value);
        }

        option.simple &= param->type == SyntaxNodeType::Identifier;
        option.params.push_back(move(param));
    }

    void Parser::ValidateParam(parser::ParserCommon::FormalParameterOptions &options, const Token &param,
                               const std::string &name) {
        Scanner& scanner = *ctx->scanner_;

        std::string key = "$" + name;
        if (ctx->strict_) {
            if (scanner.IsRestrictedWord(name)) {
                options.stricted = param;
                options.message = ParseMessages::StrictParamName;
            }
            if (options.param_set.find(key) != options.param_set.end()) {
                options.stricted = param;
                options.message = ParseMessages::StrictParamDupe;
            }
        } else if (!options.first_restricted) {
            if (scanner.IsRestrictedWord(name)) {
                options.first_restricted = param;
                options.message = ParseMessages::StrictParamName;
                // TOOD:
//            } else if (scanner_->IsStrictModeReservedWord(name)) {
//                options.first_restricted = param;
//                options.message = ParseMessages::StrictReservedWord;
            } else if (options.param_set.find(key) != options.param_set.end()) {
                options.stricted = param;
                options.message = ParseMessages::StrictParamDupe;
            }
        }

        options.param_set.insert(key);
    }

    Sp<Expression> Parser::ParsePrimaryExpression(Scope& scope) {
        auto& config = ctx->config_;

        if (config.jsx && Match(JsTokenType::LessThan)) {
            JSXParser jsxParser(*this, ctx);
            return jsxParser.ParseJSXRoot(scope);
        }
        auto marker = CreateStartMarker();
        Token token;

        if (IsKeywordToken(ctx->lookahead_.type)) {
            if (!ctx->strict_ && ctx->allow_yield_ && Match(JsTokenType::K_Yield)) {
                return ParseIdentifierName();
            } else if (!ctx->strict_ && Match(JsTokenType::K_Let)) {
                token = NextToken();
                auto id = Alloc<Identifier>();
                id->name = token.value;
                return Finalize(marker, id);
            } else {
                ctx->is_assignment_target_ = false;
                ctx->is_binding_element_ = false;
                if (Match(JsTokenType::K_Function)) {
                    return ParseFunctionExpression(scope);
                } else if (Match(JsTokenType::K_This)) {
                    token = NextToken();
                    auto th = Alloc<ThisExpression>();
                    return Finalize(marker, th);
                } else if (Match(JsTokenType::K_Class)) {
                    return ParseClassExpression(scope);
                } else if (MatchImportCall()) {
                    return ParseImportCall();
                } else {
                    ThrowUnexpectedToken(NextToken());
                    return nullptr;
                }
            }
        }

        switch (ctx->lookahead_.type) {
            case JsTokenType::Identifier: {
                if ((ctx->is_module_ || ctx->await_) && ctx->lookahead_.value == "await") {
                    ThrowUnexpectedToken(ctx->lookahead_);
                }
                if (MatchAsyncFunction()) {
                    return ParseFunctionExpression(scope);
                } else {
                    // reference to a
                    auto node = Alloc<Identifier>();
                    Token next = NextToken();
                    node->name = next.value;
                    scope.AddUnresolvedId(node);
                    return Finalize(marker, node);
                }
            }

            case JsTokenType::NumericLiteral:
            case JsTokenType::StringLiteral: {
                Literal::Ty lty = ctx->lookahead_.type == JsTokenType::NumericLiteral
                                                           ? Literal::Ty::Double
                                                           : Literal::Ty::String;
                if (ctx->strict_ && Lookahead().octal) {
                    ThrowUnexpectedToken(Lookahead());
//                this.tolerateUnexpectedToken(this.lookahead, Messages.StrictOctalLiteral);
                }
                ctx->is_assignment_target_ = false;
                ctx->is_binding_element_ = false;
                token = NextToken();
                auto node = Alloc<Literal>();
                node->ty = lty;
                node->str_ = token.value;
                node->raw = GetTokenRaw(token);
                return Finalize(marker, node);
            }

            case JsTokenType::TrueLiteral:
            case JsTokenType::FalseLiteral: {
                ctx->is_assignment_target_ = false;
                ctx->is_binding_element_ = false;
                token = NextToken();
                auto node = Alloc<Literal>();
                node->ty = Literal::Ty::Boolean;
                node->boolean_ = ctx->lookahead_.type == JsTokenType::TrueLiteral;
                node->raw = GetTokenRaw(token);
                return Finalize(marker, node);
            }

            case JsTokenType::NullLiteral: {
                ctx->is_assignment_target_ = false;
                ctx->is_binding_element_ = false;
                token = NextToken();
                auto node = Alloc<Literal>();
                node->ty = Literal::Ty::Null;
                node->str_ = token.value;
                node->raw = GetTokenRaw(token);
                return Finalize(marker, node);
            }

            case JsTokenType::RegularExpression: {
                ctx->is_assignment_target_ = false;
                ctx->is_binding_element_ = false;
                token = NextToken();
                auto node = Alloc<Literal>();
                node->ty = Literal::Ty::Regex;
                node->str_ = token.value;
                node->raw = GetTokenRaw(token);
                return Finalize(marker, node);
            }

            case JsTokenType::Template:
                return ParseTemplateLiteral(scope);

            case JsTokenType::LeftParen:
                ctx->is_binding_element_ = false;
                return InheritCoverGrammar<Expression>([this, &scope] {
                    return ParseGroupExpression(scope);
                });

            case JsTokenType::LeftBrace:
                return InheritCoverGrammar<Expression>([this, &scope] {
                    return ParseArrayInitializer(scope);
                });

            case JsTokenType::LeftBracket:
                return InheritCoverGrammar<Expression>([this, &scope] {
                    return ParseObjectInitializer(scope);
                });

            case JsTokenType::Div:
            case JsTokenType::DivAssign: {
                ctx->is_assignment_target_ = false;
                ctx->is_binding_element_ = false;
                ctx->scanner_->SetIndex(StartMarker().cursor);
                token = NextRegexToken();
                auto node = Alloc<RegexLiteral>();
                node->value = token.value;
                node->raw = GetTokenRaw(token);
                return Finalize(marker, node);
            }

            default:
                ThrowUnexpectedToken(NextToken());
                return nullptr;

        }

        Assert(false, "ParsePrimaryExpression: should not reach here");
        return nullptr;
    }

    Sp<SpreadElement> Parser::ParseSpreadElement(Scope& scope) {
        auto marker = CreateStartMarker();

        Expect(JsTokenType::Spread);

        auto node = Alloc<SpreadElement>();
        node->argument = InheritCoverGrammar<Expression>([this, &scope] {
            return ParseAssignmentExpression(scope);
        });

        return Finalize(marker, node);
    }

    Sp<Property> Parser::ParseObjectProperty(Scope& scope, bool &has_proto) {
        auto marker = CreateStartMarker();
        Token token = ctx->lookahead_;
        VarKind kind = VarKind::Invalid;
        bool computed = false;
        bool method = false;
        bool shorthand = false;
        bool is_async = false;

        optional<Sp<SyntaxNode>> key;
        optional<Sp<SyntaxNode>> value;

        if (token.type == JsTokenType::Identifier) {
            auto id = token.value;
            NextToken();
            computed = Match(JsTokenType::LeftBrace);
            is_async = !ctx->has_line_terminator_ && (id == "async") &&
                       !Match(JsTokenType::Colon) && !Match(JsTokenType::LeftParen) && !Match(JsTokenType::Mul) && !Match(JsTokenType::Comma);
            if (is_async) {
                key = ParseObjectPropertyKey(scope);
            } else {
                auto node = Alloc<Identifier>();
                node->name = id;
                key = Finalize(marker, node);
            }
        } else if (Match(JsTokenType::Mul)) {
            NextToken();
        } else {
            computed = Match(JsTokenType::LeftBrace);
            key = ParseObjectPropertyKey(scope);
        }

        auto lookahead_prop_key = QualifiedPropertyName(ctx->lookahead_);

        if (token.type == JsTokenType::Identifier && !is_async && token.value == "get" && lookahead_prop_key) {
            kind = VarKind::Get;
            computed = Match(JsTokenType::LeftBrace);
            key = ParseObjectPropertyKey(scope);
            ctx->allow_yield_ = false;
            value = ParseGetterMethod(scope);
        } else if (token.type == JsTokenType::Identifier && !is_async && token.value == "set" && lookahead_prop_key) {
            kind = VarKind::Set;
            computed = Match(JsTokenType::LeftBrace);
            key = ParseObjectPropertyKey(scope);
            ctx->allow_yield_ = false;
            value = ParseSetterMethod(scope);
        } else if (token.type == JsTokenType::Mul && !is_async && lookahead_prop_key) {
            kind = VarKind::Init;
            computed = Match(JsTokenType::LeftBrace);
            key = ParseObjectPropertyKey(scope);
            value = ParseGeneratorMethod(scope);
            method = true;
        } else {
            if (!key.has_value()) {
                ThrowUnexpectedToken(ctx->lookahead_);
            }

            kind = VarKind::Init;
            if (Match(JsTokenType::Colon) && !is_async) {
                if (!computed && IsPropertyKey(*key, "__proto__")) {
                    if (has_proto) {
                        TolerateError(ParseMessages::DuplicateProtoProperty);
                    }
                    has_proto = true;
                }
                NextToken();
                value = InheritCoverGrammar<Expression>([this, &scope] {
                    return ParseAssignmentExpression(scope);
                });
            } else if (Match(JsTokenType::LeftParen)) {
                if (is_async) {
                    value = ParsePropertyMethodAsyncFunction(scope);
                } else {
                    value = ParsePropertyMethodFunction(scope);
                }
                method = true;
            } else if (token.type == JsTokenType::Identifier) {
                auto id = Alloc<Identifier>();
                id->name = token.value;
                Finalize(marker, id);
                if (Match(JsTokenType::Assign)) {
                    ctx->first_cover_initialized_name_error_ = {ctx->lookahead_ };
                    NextToken();
                    shorthand = true;
                    auto node = Alloc<AssignmentPattern>();
                    node->left = move(id);
                    node->right = IsolateCoverGrammar<Expression>([this, &scope] {
                        return ParseAssignmentExpression(scope);
                    });
                } else {
                    shorthand = true;
                    value = move(id);
                }
            } else {
                ThrowUnexpectedToken(NextToken());
            }
        }

        Assert(key.has_value(), "Property: key should has value");
        auto node = Alloc<Property>();
        node->kind = kind;
        node->key = *key;
        node->value = move(value);
        node->computed = computed;
        node->method = method;
        node->shorthand = shorthand;
        return node;
    }

    Sp<SyntaxNode> Parser::ParseObjectPropertyKey(Scope& scope) {
        auto marker = CreateStartMarker();
        Token token = NextToken();

        if (IsKeywordToken(token.type)) {
            auto node = Alloc<Identifier>();
            node->name = token.value;
            return Finalize(marker, node);
        }

        if (IsPunctuatorToken(token.type)) {
            if (token.type == JsTokenType::LeftBrace) {
                auto result = IsolateCoverGrammar<Expression>([this, &scope] {
                    return ParseAssignmentExpression(scope);
                });
                Expect(JsTokenType::RightBrace);
                return result;
            } else {
                ThrowUnexpectedToken(token);
            }
        }

        switch (token.type) {
            case JsTokenType::StringLiteral: {
                if (ctx->strict_ && token.octal) {
                    TolerateUnexpectedToken(token, ParseMessages::StrictOctalLiteral);
                }
                auto node = Alloc<Literal>();
                node->ty = Literal::Ty::String;
                node->str_ = token.value;
                node->raw = GetTokenRaw(token);
                return Finalize(marker, node);
            }

            case JsTokenType::NumericLiteral: {
                if (ctx->strict_ && token.octal) {
                    TolerateUnexpectedToken(token, ParseMessages::StrictOctalLiteral);
                }
                auto node = Alloc<Literal>();
                node->ty = Literal::Ty::Double;
                node->str_ = token.value;
                node->raw = GetTokenRaw(token);
                return Finalize(marker, node);
            }

            case JsTokenType::Identifier:
            case JsTokenType::TrueLiteral:
            case JsTokenType::FalseLiteral:
            case JsTokenType::NullLiteral: {
                auto node = Alloc<Identifier>();
                node->name = token.value;
                return Finalize(marker, node);
            }

            default:
                ThrowUnexpectedToken(token);
                return nullptr;
        }

        return nullptr;
    }

    Sp<Expression> Parser::ParseObjectInitializer(Scope& scope) {
        auto marker = CreateStartMarker();

        Expect(JsTokenType::LeftBracket);
        auto node = Alloc<ObjectExpression>();
        bool has_proto = false;
        while (!Match(JsTokenType::RightBracket)) {
            Sp<SyntaxNode> prop;
            if (Match(JsTokenType::Spread)) {
                prop = ParseSpreadElement(scope);
            } else {
                prop = ParseObjectProperty(scope, has_proto);
            }
            node->properties.push_back(std::move(prop));
            if (!Match(JsTokenType::RightBracket)) {
                ExpectCommaSeparator();
            }
        }
        Expect(JsTokenType::RightBracket);

        return Finalize(marker, node);
    }

    Sp<FunctionExpression> Parser::ParseFunctionExpression(Scope& parent_scope) {
        auto marker = CreateStartMarker();

        auto fun_scope = std::make_unique<Scope>(ScopeType::Function);
        fun_scope->SetParent(&parent_scope);
        auto& scope = *fun_scope.get();

        bool is_async = MatchContextualKeyword("async");
        if (is_async) NextToken();

        Expect(JsTokenType::K_Function);

        bool is_generator = is_async ? false : Match(JsTokenType::Mul);
        if (is_generator) NextToken();

        optional<Sp<Identifier>> id;
        optional<Token> first_restricted;

        bool prev_allow_await = ctx->await_;
        bool prev_allow_yield = ctx->allow_yield_;
        ctx->await_ = is_async;
        ctx->allow_yield_ = !is_generator;

        std::string message;

        if (!Match(JsTokenType::LeftParen)) {
            Token token = ctx->lookahead_;
            auto& scanner = *ctx->scanner_;

            if (!ctx->strict_ && !is_generator && Match(JsTokenType::K_Yield)) {
                id = ParseIdentifierName();
            } else {
                id = ParseVariableIdentifier(scope, VarKind::Invalid);
            }

            if (ctx->strict_) {
                if (scanner.IsRestrictedWord(token.value)) {
                    TolerateUnexpectedToken(token, ParseMessages::StrictFunctionName);
                }
            } else {
                if (scanner.IsRestrictedWord(token.value)) {
                    first_restricted = token;
                    message = ParseMessages::StrictFunctionName;
                    // TODO: message
//                } else if (scanner_->IsStrictModeReservedWord(token.value_)) {
//                    first_restricted = token;
//                    message = ParseMessages::StrictReservedWord;
                }
            }
        }

        auto formal = ParseFormalParameters(scope, first_restricted);
        if (!formal.message.empty()) {
            message = formal.message;
        }

        bool prev_strict = ctx->strict_;
        bool prev_allow_strict_directive = ctx->allow_strict_directive_;
        ctx->allow_strict_directive_ = formal.simple;
        auto body = ParseFunctionSourceElements(scope);
        if (ctx->strict_ && first_restricted.has_value()) {
            ThrowUnexpectedToken(*first_restricted, message);
        }
        if (ctx->strict_ && formal.stricted.has_value()) {
            TolerateUnexpectedToken(*formal.stricted, message);
        }
        ctx->strict_ = prev_strict;
        ctx->allow_strict_directive_ = prev_allow_strict_directive;
        ctx->await_ = prev_allow_await;
        ctx->allow_yield_ = prev_allow_yield;

        if (is_async) {
            auto node = Alloc<FunctionExpression>();
            node->id = id;
            node->params = move(formal.params);
            node->body = move(body);
            node->async = true;
            node->scope = move(fun_scope);
            return Finalize(marker, node);
        } else {
            auto node = Alloc<FunctionExpression>();
            node->id = id;
            node->params = move(formal.params);
            node->body = move(body);
            node->generator = is_generator;
            node->async = false;
            node->scope = move(fun_scope);
            return Finalize(marker, node);
        }
    }

    Sp<Identifier> Parser::ParseIdentifierName() {
        auto marker = CreateStartMarker();
        auto node = Alloc<Identifier>();
        Token token = NextToken();
        if (!IsIdentifierName(token)) {
            ThrowUnexpectedToken(token);
            return nullptr;
        }
        node->name = token.value;
        return Finalize(marker, node);
    }

    Sp<Expression> Parser::ParseNewExpression(Scope& scope) {
        auto start_marker = CreateStartMarker();

        Sp<Identifier> id = ParseIdentifierName();
        Assert(id->name == "new", "New expression must start with `new`");

        Sp<Expression> expr;

        if (Match(JsTokenType::Dot)) {
            NextToken();
            if (
                    ctx->lookahead_.type == JsTokenType::Identifier &&
                    ctx->in_function_body_ &&
                            ctx->lookahead_.value == "target"
            ) {
                auto node = Alloc<MetaProperty>();
                node->property = ParseIdentifierName();
                node->meta = id;
                expr = node;
            } else {
                ThrowUnexpectedToken(ctx->lookahead_);
                return nullptr;
            }
        } else if (Match(JsTokenType::K_Import)) {
            ThrowUnexpectedToken(ctx->lookahead_);
            return nullptr;
        } else {
            auto node = Alloc<NewExpression>();
            node->callee = IsolateCoverGrammar<Expression>([this, &scope] {
                return ParseLeftHandSideExpression(scope);
            });
            if (Match(JsTokenType::LeftParen)) {
                node->arguments = ParseArguments(scope);
            }
            expr = node;
            ctx->is_assignment_target_ = false;
            ctx->is_binding_element_ = false;
        }

        return Finalize(start_marker, expr);
    }

    Sp<YieldExpression> Parser::ParseYieldExpression(Scope& scope) {
        auto marker = CreateStartMarker();
        Expect(JsTokenType::K_Yield);

        auto node = Alloc<YieldExpression>();
        if (!ctx->has_line_terminator_) {
            bool prev_allow_yield = ctx->allow_yield_;
            ctx->allow_yield_ = false;
            node->delegate = Match(JsTokenType::Mul);
            if (node->delegate) {
                NextToken();
                node->argument = ParseAssignmentExpression(scope);
            } else if (IsStartOfExpression()) {
                node->argument = ParseAssignmentExpression(scope);
            }
            ctx->allow_yield_ = prev_allow_yield;
        }

        return Finalize(marker, node);
    }

    std::vector<Sp<SyntaxNode>> Parser::ParseArguments(Scope& scope) {
        std::vector<Sp<SyntaxNode>> result;
        Expect(JsTokenType::LeftParen);
        if (!Match(JsTokenType::RightParen)) {
            Sp<SyntaxNode> expr;
            while (true) {
                if (Match(JsTokenType::Spread)) {
                    expr = ParseSpreadElement(scope);
                } else {
                    expr = IsolateCoverGrammar<Expression>([this, &scope] {
                        return ParseAssignmentExpression(scope);
                    });
                }
                result.push_back(expr);
                if (Match(JsTokenType::RightParen)) {
                    break;
                }
                ExpectCommaSeparator();
                if (Match(JsTokenType::RightParen)) {
                    break;
                }
            }
        }
        Expect(JsTokenType::RightParen);
        return result;
    }

    Sp<Import> Parser::ParseImportCall() {
        auto marker = CreateStartMarker();
        auto node = Alloc<Import>();
        Expect(JsTokenType::K_Import);
        return Finalize(marker, node);
    }

    Sp<Statement> Parser::ParseDirective(Scope& scope) {
        auto token = ctx->lookahead_;

        auto marker = CreateStartMarker();
        Sp<Expression> expr = ParseExpression(scope);
        std::string directive;
        if (expr->type == SyntaxNodeType::Literal) {
            directive = token.value.substr(1, token.value.size() - 1);
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

    Sp<Expression> Parser::ParseExpression(Scope& scope) {
        auto start_token = ctx->lookahead_;
        auto start_marker = CreateStartMarker();
        Sp<Expression> expr = IsolateCoverGrammar<Expression>([this, &scope] {
            return ParseAssignmentExpression(scope);
        });

        if (Match(JsTokenType::Comma)) {
            std::vector<Sp<Expression>> expressions;
            expressions.push_back(expr);
            while (ctx->lookahead_.type != JsTokenType::EOF_) {
                if (!Match(JsTokenType::Comma)) {
                    break;
                }
                NextToken();
                Sp<Expression> node = IsolateCoverGrammar<Expression>([this, &scope] {
                    return ParseAssignmentExpression(scope);
                });
                expressions.push_back(node);
            }

            auto node = Alloc<SequenceExpression>();
            node->expressions = expressions;
            expr = Finalize(start_marker, node);
        }

        return expr;
    }

    Sp<BlockStatement> Parser::ParseFunctionSourceElements(Scope& scope) {
        auto start_marker = CreateStartMarker();

        vector<Sp<SyntaxNode>> body = ParseDirectivePrologues(scope);
        Expect(JsTokenType::LeftBracket);

        auto prev_label_set = move(ctx->label_set_);
        bool prev_in_iteration = ctx->in_iteration_;
        bool prev_in_switch = ctx->in_switch_;
        bool prev_in_fun_body = ctx->in_function_body_;

        ctx->label_set_ = make_unique<HashSet<std::string>>();
        ctx->in_iteration_ = false;
        ctx->in_switch_ = false;
        ctx->in_function_body_ = true;

        while (ctx->lookahead_.type != JsTokenType::EOF_) {
            if (Match(JsTokenType::RightBracket)) {
                break;
            }

            Sp<SyntaxNode> temp = ParseStatementListItem(scope);
            body.push_back(move(temp));
        }

        Expect(JsTokenType::RightBracket);

        ctx->label_set_ = move(prev_label_set);
        ctx->in_iteration_ = prev_in_iteration;
        ctx->in_switch_ = prev_in_switch;
        ctx->in_function_body_ = prev_in_fun_body;

        auto node = Alloc<BlockStatement>();
        node->body = move(body);

        return Finalize(start_marker, node);
    }

    std::vector<Sp<SyntaxNode>> Parser::ParseDirectivePrologues(Scope& scope) {
        optional<Token> first_restrict;
        std::vector<Sp<SyntaxNode>> result;

        Token token;
        while (true) {
            token = ctx->lookahead_;
            if (token.type != JsTokenType::StringLiteral) {
                break;
            }

            Sp<Statement> statement = ParseDirective(scope);
            result.push_back(statement);
            if (statement->type != SyntaxNodeType::Directive) {
                break;
            }
            auto directive_ = reinterpret_cast<Directive*>(statement.get());

            if (directive_->directive == "use strict") {
                ctx->strict_ = true;
                if (first_restrict) {
                    TolerateUnexpectedToken(*first_restrict, ParseMessages::StrictOctalLiteral);
                }
                if (!ctx->allow_strict_directive_) {
                    TolerateUnexpectedToken(token, ParseMessages::IllegalLanguageModeDirective);
                }
            } else {
                if (!first_restrict && token.octal) {
                    first_restrict = token;
                }
            }
        }

        return result;
    }

    Sp<ClassBody> Parser::ParseClassBody(Scope& scope) {
        auto marker = CreateStartMarker();
        auto node = Alloc<ClassBody>();

        node->body = ParseClassElementList(scope);

        return Finalize(marker, node);
    }

    Sp<ClassDeclaration> Parser::ParseClassDeclaration(Scope& parent_scope, bool identifier_is_optional) {
        auto marker = CreateStartMarker();

        auto cls_scope = std::make_unique<Scope>(ScopeType::Class);
        cls_scope->SetParent(&parent_scope);
        auto& scope = *cls_scope;

        bool prev_strict = ctx->strict_;
        ctx->strict_ = true;
        Expect(JsTokenType::K_Class);

        auto node = Alloc<ClassDeclaration>();
        node->scope = move(cls_scope);
        if (identifier_is_optional && (ctx->lookahead_.type != JsTokenType::Identifier)) {
            // nothing
        } else {
            node->id = ParseVariableIdentifier(parent_scope, VarKind::Var);
        }

        if (Match(JsTokenType::K_Extends)) {
            NextToken();
            auto temp = IsolateCoverGrammar<Expression>([this, &scope] {
                return ParseLeftHandSideExpressionAllowCall(scope);
            });
            if (temp->type != SyntaxNodeType::Identifier) {
                ThrowUnexpectedToken(Token());
                return nullptr;
            }
            node->super_class = dynamic_pointer_cast<Identifier>(temp);
        }
        node->body = ParseClassBody(scope);
        ctx->strict_ = prev_strict;

        return Finalize(marker, node);
    }

    Sp<ClassExpression> Parser::ParseClassExpression(Scope& parent_scope) {
        auto marker = CreateStartMarker();

        auto cls_scope = std::make_unique<Scope>(ScopeType::Class);
        cls_scope->SetParent(&parent_scope);
        auto& scope = *cls_scope;

        auto node = Alloc<ClassExpression>();
        node->scope = move(cls_scope);

        bool prev_strict = ctx->strict_;
        ctx->strict_ = true;
        Expect(JsTokenType::K_Class);

        if (ctx->lookahead_.type == JsTokenType::Identifier) {
            node->id = ParseVariableIdentifier(parent_scope, VarKind::Invalid);
        }

        if (Match(JsTokenType::K_Extends)) {
            NextToken();
            auto temp = IsolateCoverGrammar<Expression>([this, &scope] {
                return ParseLeftHandSideExpressionAllowCall(scope);
            });
            if (temp->type != SyntaxNodeType::Identifier) {
                ThrowUnexpectedToken(Token());
                return nullptr;
            }
            node->super_class = dynamic_pointer_cast<Identifier>(temp);
        }

        node->body = ParseClassBody(scope);
        ctx->strict_ = prev_strict;

        return Finalize(marker, node);
    }

    Sp<Expression> Parser::ParseLeftHandSideExpressionAllowCall(Scope& scope) {
        Token start_token = ctx->lookahead_;
        auto start_marker = CreateStartMarker();
        bool maybe_async = MatchContextualKeyword("async");

        bool prev_allow_in = ctx->allow_in_;
        ctx->allow_in_ = true;

        Sp<Expression> expr;
        if (Match(JsTokenType::K_Super) && ctx->in_function_body_) {
            auto node = Alloc<Super>();
            auto marker = CreateStartMarker();
            NextToken();
            expr = Finalize(marker, node);
            if (!Match(JsTokenType::LeftParen) && !Match(JsTokenType::Dot) && !Match(JsTokenType::LeftBrace)) {
                ThrowUnexpectedToken(ctx->lookahead_);
                return nullptr;
            }
        } else {
            if (Match(JsTokenType::K_New)) {
                expr = InheritCoverGrammar<Expression>([this, &scope] {
                    return ParseNewExpression(scope);
                });
            } else {
                expr = InheritCoverGrammar<Expression>([this, &scope] {
                    return ParsePrimaryExpression(scope);
                });
            }
        }
        Assert(!!expr, "ParseLeftHandSideExpressionAllowCall: expr should not be nullptr");

        while (true) {
            if (Match(JsTokenType::Dot)) {
                ctx->is_binding_element_ = false;
                ctx->is_assignment_target_ = true;
                Expect(JsTokenType::Dot);
                auto node = Alloc<MemberExpression>();
                node->property = ParseIdentifierName();
                node->object = expr;
                node->computed = false;
                expr = Finalize(StartNode(start_token), node);
            } else if (Match(JsTokenType::LeftParen)) {
                bool async_arrow = maybe_async && (start_token.lineNumber == ctx->lookahead_.lineNumber);
                ctx->is_binding_element_ = false;
                ctx->is_assignment_target_ = false;
                auto node = Alloc<CallExpression>();
                if (async_arrow) {
                    node->arguments = ParseAsyncArguments(scope);
                } else {
                    node->arguments = ParseArguments(scope);
                }
                if (expr->type == SyntaxNodeType::Import && node->arguments.size() != 1) {
                    TolerateError(ParseMessages::BadImportCallArity);
                }
                node->callee = expr;
                expr = Finalize(StartNode(start_token), node);
                if (async_arrow && Match(JsTokenType::Arrow)) {
                    for (std::size_t i = 0; i < node->arguments.size(); i++) {
                        node->arguments[i] = ReinterpretExpressionAsPattern(node->arguments[i]);
                    }
                    auto placeholder = Alloc<ArrowParameterPlaceHolder>();
                    placeholder->params = node->arguments;
                    placeholder->async = true;
                    expr = move(placeholder);
                }
            } else if (Match(JsTokenType::LeftBrace)) {
                ctx->is_binding_element_ = false;
                ctx->is_assignment_target_ = true;
                Expect(JsTokenType::LeftBrace);
                auto node = Alloc<MemberExpression>();
                node->computed = true;
                node->property = IsolateCoverGrammar<Expression>([this, &scope] {
                    return ParseExpression(scope);
                });
                Expect(JsTokenType::RightBrace);
                node->object = expr;
                expr = Finalize(StartNode(start_token), node);
            } else if (ctx->lookahead_.type == JsTokenType::Template && ctx->lookahead_.head) {
                auto node = Alloc<TaggedTemplateExpression>();
                node->quasi = ParseTemplateLiteral(scope);
                node->tag = expr;
                expr = Finalize(StartNode(start_token), node);
            } else {
                break;
            }
        }
        ctx->allow_in_ = prev_allow_in;

        return expr;
    }

    Sp<Expression> Parser::ParseArrayInitializer(Scope& scope) {
        auto marker = CreateStartMarker();
        auto node = Alloc<ArrayExpression>();
        Sp<SyntaxNode> element;

        Expect(JsTokenType::LeftBrace);
        while (!Match(JsTokenType::RightBrace)) {
            if (Match(JsTokenType::Comma)) {
                NextToken();
                node->elements.emplace_back(nullopt);
            } else if (Match(JsTokenType::Spread)) {
                element = ParseSpreadElement(scope);
                if (!Match(JsTokenType::RightBrace)) {
                    ctx->is_assignment_target_ = false;
                    ctx->is_binding_element_ = false;
                    Expect(JsTokenType::Comma);
                }
                node->elements.emplace_back(element);
            } else {
                element = InheritCoverGrammar<SyntaxNode>([this, &scope] {
                    return ParseAssignmentExpression(scope);
                });
                node->elements.emplace_back(element);
                if (!Match(JsTokenType::RightBrace)) {
                    Expect(JsTokenType::Comma);
                }
            }
        }
        Expect(JsTokenType::RightBrace);

        return Finalize(marker, node);
    }

    Sp<Module> Parser::ParseModule() {
        ctx->strict_ = true;
        ctx->is_module_ = true;
        auto marker = CreateStartMarker();
        auto node = make_shared<Module>();
        node->body = ParseDirectivePrologues(*node->scope.get());
        node->source_type = u"module";
        while (ctx->lookahead_.type != JsTokenType::EOF_) {
            node->body.push_back(ParseStatementListItem(*node->scope.get()));
        }
        if (ctx->config_.comment) {
            node->comments = move(ctx->comments_);
        } else {
            ctx->comments_.clear();
            ctx->comments_.shrink_to_fit();
        }
        return Finalize(marker, node);
    }

    Sp<Script> Parser::ParseScript() {
        auto start_marker = CreateStartMarker();
        auto node = Alloc<Script>();
        node->body = ParseDirectivePrologues(*node->scope);
        node->source_type = u"script";
        while (ctx->lookahead_.type != JsTokenType::EOF_) {
            node->body.push_back(ParseStatementListItem(*node->scope));
        }
        if (ctx->config_.comment) {
            node->comments = move(ctx->comments_);
        } else {
            ctx->comments_.clear();
            ctx->comments_.shrink_to_fit();
        }
        return Finalize(start_marker, node);
    }

    Sp<SwitchCase> Parser::ParseSwitchCase(Scope& scope) {
        auto marker = CreateStartMarker();
        auto node = Alloc<SwitchCase>();

        if (Match(JsTokenType::K_Default)) {
            NextToken();
            node->test.reset();
        } else {
            Expect(JsTokenType::K_Case);
            node->test = ParseExpression(scope);
        }
        Expect(JsTokenType::Colon);

        while (true) {
            if (Match(JsTokenType::RightBracket) || Match(JsTokenType::K_Default) || Match(JsTokenType::K_Case)) {
                break;
            }
            Sp<Statement> con = ParseStatementListItem(scope);
            node->consequent.push_back(std::move(con));
        }

        return Finalize(marker, node);
    }

    Sp<IfStatement> Parser::ParseIfStatement(Scope& scope) {
        auto marker = CreateStartMarker();
        auto node = Alloc<IfStatement>();

        Expect(JsTokenType::K_If);
        Expect(JsTokenType::LeftParen);
        node->test = ParseExpression(scope);

        if (!Match(JsTokenType::RightParen) && ctx->config_.tolerant) {
            Token token = NextToken();
            ThrowUnexpectedToken(token);
            node->consequent = Finalize(CreateStartMarker(), Alloc<EmptyStatement>());
        } else {
            Expect(JsTokenType::RightParen);
            node->consequent = ParseIfClause(scope);
            if (Match(JsTokenType::K_Else)) {
                NextToken();
                node->alternate = ParseIfClause(scope);
            }
        }

        return Finalize(marker, node);
    }

    Sp<Statement> Parser::ParseIfClause(Scope& scope) {
        if (ctx->strict_ && Match(JsTokenType::K_Function)) {
            TolerateError(ParseMessages::StrictFunction);
        }
        return ParseStatement(scope);
    }

    Sp<Statement> Parser::ParseStatement(Scope& scope) {

        if (IsPunctuatorToken(ctx->lookahead_.type)) {
            switch (ctx->lookahead_.type) {
                case JsTokenType::LeftBracket:
                    return ParseBlock(scope, true);

                case JsTokenType::LeftParen:
                    return ParseExpressionStatement(scope);

                case JsTokenType::Semicolon:
                    return ParseEmptyStatement();

                default:
                    return ParseExpressionStatement(scope);

            }
        }

        switch (ctx->lookahead_.type) {
            case JsTokenType::TrueLiteral:
            case JsTokenType::FalseLiteral:
            case JsTokenType::NullLiteral:
            case JsTokenType::NumericLiteral:
            case JsTokenType::StringLiteral:
            case JsTokenType::Template:
            case JsTokenType::RegularExpression:
                return ParseExpressionStatement(scope);

            case JsTokenType::Identifier: {
                if (MatchAsyncFunction()) {
                    return ParseFunctionDeclaration(scope, false);
                } else {
                    return ParseLabelledStatement(scope);
                }
            }

            case JsTokenType::K_Break:
                return ParseBreakStatement();

            case JsTokenType::K_Continue:
                return ParseContinueStatement();

            case JsTokenType::K_Debugger:
                return ParseDebuggerStatement();

            case JsTokenType::K_Do:
                return ParseDoWhileStatement(scope);

            case JsTokenType::K_For:
                return ParseForStatement(scope);

            case JsTokenType::K_Function:
                return ParseFunctionDeclaration(scope, false);

            case JsTokenType::K_If:
                return ParseIfStatement(scope);

            case JsTokenType::K_Return:
                return ParseReturnStatement(scope);

            case JsTokenType::K_Switch:
                return ParseSwitchStatement(scope);

            case JsTokenType::K_Throw:
                return ParseThrowStatement(scope);

            case JsTokenType::K_Try:
                return ParseTryStatement(scope);

            case JsTokenType::K_Var:
                return ParseVariableStatement(scope);

            case JsTokenType::K_While:
                return ParseWhileStatement(scope);

            case JsTokenType::K_With:
                return ParseWithStatement(scope);

            default:
                if (IsKeywordToken(ctx->lookahead_.type)) {
                    return ParseExpressionStatement(scope);
                }
                ThrowUnexpectedToken(ctx->lookahead_);
                return nullptr;

        }
    }

    Sp<ExpressionStatement> Parser::ParseExpressionStatement(Scope& scope) {
        auto marker = CreateStartMarker();
        auto node = Alloc<ExpressionStatement>();

        node->expression  = ParseExpression(scope);
        ConsumeSemicolon();

        return Finalize(marker, node);
    }

    Sp<BlockStatement> Parser::ParseBlock(Scope& parent_scope, bool new_scope) {
        auto marker = CreateStartMarker();
        auto node = Alloc<BlockStatement>();
        if (new_scope) {
            auto new_scope_ins = std::make_unique<Scope>();
            new_scope_ins->type = ScopeType::Block;
            new_scope_ins->SetParent(&parent_scope);
            node->scope = { std::move(new_scope_ins) };
        }

        Expect(JsTokenType::LeftBracket);
        while (true) {
            if (Match(JsTokenType::RightBracket)) {
                break;
            }
            Sp<SyntaxNode> stmt;
            if (new_scope) {
                stmt = ParseStatementListItem(**node->scope);
            } else {
                stmt = ParseStatementListItem(parent_scope);
            }
            node->body.push_back(std::move(stmt));
        }
        Expect(JsTokenType::RightBracket);

        return Finalize(marker, node);
    }

    Sp<EmptyStatement> Parser::ParseEmptyStatement() {
        auto node = Alloc<EmptyStatement>();
        auto marker = CreateStartMarker();
        Expect(JsTokenType::Semicolon);
        return Finalize(marker, node);
    }

    Sp<FunctionDeclaration> Parser::ParseFunctionDeclaration(Scope& parent_scope, bool identifier_is_optional) {
        auto marker = CreateStartMarker();

        auto fun_scope = std::make_unique<Scope>(ScopeType::Function);
        fun_scope->SetParent(&parent_scope);
        auto& scope = *fun_scope;

        bool is_async = MatchContextualKeyword("async");
        if (is_async) {
            NextToken();
        }

        Expect(JsTokenType::K_Function);

        bool is_generator = is_async ? false : Match(JsTokenType::Mul);
        if (is_generator) {
            NextToken();
        }

        optional<Sp<Identifier>> id;
        optional<Token> first_restricted;
        string message;

        auto& scanner = *ctx->scanner_;

        if (!identifier_is_optional || !Match(JsTokenType::LeftParen)) {
            Token token = ctx->lookahead_;
            id = ParseVariableIdentifier(parent_scope, VarKind::Var);
            if (ctx->strict_) {
                if (scanner.IsRestrictedWord(token.value)) {
                    TolerateUnexpectedToken(token, ParseMessages::StrictFunctionName);
                }
            } else {
                if (scanner.IsRestrictedWord(token.value)) {
                    first_restricted = token;
                    message = ParseMessages::StrictFunctionName;
                } else {
                    first_restricted = token;
                    message = ParseMessages::StrictReservedWord;
                }
            }
        }

        bool prev_allow_await = ctx->await_;
        bool prev_allow_yield = ctx->allow_yield_;
        ctx->await_ = is_async;
        ctx->allow_yield_ = !is_generator;

        auto options = ParseFormalParameters(scope, first_restricted);
        first_restricted = options.first_restricted;
        if (!options.message.empty()) {
            message = move(options.message);
        }

        bool prev_strict = ctx->strict_;
        bool prev_allow_strict_directive = ctx->allow_strict_directive_;
        ctx->allow_strict_directive_ = options.simple;
        auto body = ParseFunctionSourceElements(scope);

        if (ctx->strict_ && first_restricted) {
            Token temp = *first_restricted;
            ThrowUnexpectedToken(temp, message);
        }
        if (ctx->strict_ && options.stricted) {
            Token temp = *options.stricted;
            TolerateUnexpectedToken(temp, message);
        }

        ctx->strict_ = prev_strict;
        ctx->allow_strict_directive_ = prev_allow_strict_directive;
        ctx->await_ = prev_allow_await;
        ctx->allow_yield_ = prev_allow_yield;

        if (is_async) {
            auto node = Alloc<FunctionDeclaration>();
            node->id = id;
            node->params = move(options.params);
            node->body = move(body);
            node->async = true;
            node->scope = move(fun_scope);
            return Finalize(marker, node);
        } else {
            auto node = Alloc<FunctionDeclaration>();
            node->id = id;
            node->generator = is_generator;
            node->params = move(options.params);
            node->body = move(body);
            node->async = false;
            node->scope = move(fun_scope);

            return Finalize(marker, node);
        }
    }

    Sp<Statement> Parser::ParseLabelledStatement(Scope& scope) {
        auto start_marker = CreateStartMarker();
        Sp<Expression> expr = ParseExpression(scope);

        Sp<Statement> statement;
        if ((expr->type == SyntaxNodeType::Identifier) && Match(JsTokenType::Colon)) {
            NextToken();

            auto id = dynamic_pointer_cast<Identifier>(expr);
            std::string key = "$" + id->name;

            if (ctx->label_set_->find(key) != ctx->label_set_->end()) {
                ThrowError(ParseMessages::Redeclaration, string("Label: ") + id->name);
            }
            ctx->label_set_->insert(key);

            Sp<Statement> body;
            if (Match(JsTokenType::K_Class)) {
                TolerateUnexpectedToken(ctx->lookahead_);
                body = ParseClassDeclaration(scope, false);
            } else if (Match(JsTokenType::K_Function)) {
                Token token = ctx->lookahead_;
                auto declaration = ParseFunctionDeclaration(scope, false);
                if (ctx->strict_) {
                    TolerateUnexpectedToken(token, ParseMessages::StrictFunction);
                } else if (declaration->generator) {
                    TolerateUnexpectedToken(token, ParseMessages::GeneratorInLegacyContext);
                }
                body = move(declaration);
            } else {
                body = ParseStatement(scope);
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
        Expect(JsTokenType::K_Break);

        std::optional<Sp<Identifier>> label;
        if (ctx->lookahead_.type == JsTokenType::Identifier && !ctx->has_line_terminator_) {
            Scope scope;
            Sp<Identifier> id = ParseVariableIdentifier(scope, VarKind::Invalid);

            std::string key = "$" + id->name;
            if (auto& label_set = *ctx->label_set_; label_set.find(key) == label_set.end()) {
                ThrowError(ParseMessages::UnknownLabel, id->name);
            }
            label = id;
        }

        ConsumeSemicolon();

        if (!label && !ctx->in_iteration_ && !ctx->in_switch_) {
            ThrowError(ParseMessages::IllegalBreak);
            return nullptr;
        }

        auto node = Alloc<BreakStatement>();
        node->label = label;
        return Finalize(marker, node);
    }

    Sp<ContinueStatement> Parser::ParseContinueStatement() {
        auto marker = CreateStartMarker();
        Expect(JsTokenType::K_Continue);
        auto node = Alloc<ContinueStatement>();

        if (ctx->lookahead_.type == JsTokenType::Identifier && !ctx->has_line_terminator_) {
            Scope scope;
            auto id = ParseVariableIdentifier(scope, VarKind::Invalid);
            node->label = id;

            std::string key = "$" + id->name;
            if (auto& label_set = *ctx->label_set_; label_set.find(key) == label_set.end()) {
                ThrowError(ParseMessages::UnknownLabel, id->name);
            }
        }

        ConsumeSemicolon();

        if (!node->label && !ctx->in_iteration_) {
            ThrowError(ParseMessages::IllegalContinue);
            return nullptr;
        }

        return Finalize(marker, node);
    }

    Sp<DebuggerStatement> Parser::ParseDebuggerStatement() {
        auto marker = CreateStartMarker();
        Expect(JsTokenType::K_Debugger);
        ConsumeSemicolon();
        auto node = Alloc<DebuggerStatement>();
        return Finalize(marker, node);
    }

    Sp<DoWhileStatement> Parser::ParseDoWhileStatement(Scope& scope) {
        auto marker = CreateStartMarker();
        auto node = Alloc<DoWhileStatement>();
        Expect(JsTokenType::K_Do);

        auto previous_in_interation = ctx->in_iteration_;
        ctx->in_iteration_ = true;
        node->body = ParseStatement(scope);
        ctx->in_iteration_ = previous_in_interation;

        Expect(JsTokenType::K_While);
        Expect(JsTokenType::LeftParen);
        node->test = ParseExpression(scope);

        if (!Match(JsTokenType::RightParen) && ctx->config_.tolerant) {
            ThrowUnexpectedToken(NextToken());
        } else {
            Expect(JsTokenType::RightParen);
            if (Match(JsTokenType::Semicolon)) {
                NextToken();
            }
        }

        return Finalize(marker, node);
    }

    Sp<Statement> Parser::ParseForStatement(Scope& parent_scope) {
        bool for_in = true;

        std::optional<Sp<SyntaxNode>> init;
        std::optional<Sp<Expression>> test;
        std::optional<Sp<Expression>> update;
        Sp<SyntaxNode> left;
        Sp<SyntaxNode> right;
        auto marker = CreateStartMarker();

        auto for_scope = std::make_unique<Scope>(ScopeType::For);
        for_scope->SetParent(&parent_scope);
        auto& scope = *for_scope;

        Expect(JsTokenType::K_For);
        Expect(JsTokenType::LeftParen);

        if (Match(JsTokenType::Semicolon)) {
            NextToken();
        } else {
            if (Match(JsTokenType::K_Var)) {
                auto marker = CreateStartMarker();
                NextToken();

                auto prev_allow_in = ctx->allow_in_;
                ctx->allow_in_ = true;
                auto declarations = ParseVariableDeclarationList(scope, for_in);
                ctx->allow_in_ = prev_allow_in;

                if (declarations.size() == 1 && Match(JsTokenType::K_In)) {
                    auto decl = declarations[0];
                    if (decl->init && (decl->id->type == SyntaxNodeType::ArrayPattern || decl->id->type == SyntaxNodeType::ObjectPattern || ctx->strict_)) {
                        TolerateError(ParseMessages::ForInOfLoopInitializer);
                    }
                    auto node = Alloc<VariableDeclaration>();
                    node->declarations = declarations;
                    node->kind = VarKind::Var;
                    init = Finalize(marker, node);
                    NextToken();
                    left = *init;
                    right = ParseExpression(scope);
                    init.reset();
                } else if (declarations.size() == 1 && !declarations[0]->init && MatchContextualKeyword("of")) { // of
                    auto node = Alloc<VariableDeclaration>();
                    node->declarations = declarations;
                    node->kind = VarKind::Var;
                    init = Finalize(marker, node);
                    NextToken();
                    left = *init;
                    right = ParseAssignmentExpression(scope);
                    init.reset();
                    for_in = false;
                } else {
                    auto node = Alloc<VariableDeclaration>();
                    node->declarations = declarations;
                    node->kind = VarKind::Var;
                    init = Finalize(marker, node);
                    Expect(JsTokenType::Semicolon);
                }
            } else if (Match(JsTokenType::K_Const) || Match(JsTokenType::K_Let)) {
                auto marker = CreateStartMarker();
                Token token = NextToken();
                VarKind kind;
                if (token.value == "const") {
                    kind = VarKind::Const;
                } else if (token.value == "let") {
                    kind = VarKind::Let;
                }

                if (!ctx->strict_ && ctx->lookahead_.value == "in") {
                    auto node = Alloc<Identifier>();
                    node->name = token.value;
                    init = Finalize(marker, node);
                    NextToken();
                    left = *init;
                    right = ParseExpression(scope);
                    init.reset();
                } else {
                    auto prev_allow_in = ctx->allow_in_;
                    ctx->allow_in_ = false;
                    bool in_for = true;
                    auto declarations = ParseBindingList(scope, VarKind::Const, in_for);
                    ctx->allow_in_ = prev_allow_in;

                    if (declarations.size() == 1 && !declarations[0]->init && Match(JsTokenType::K_In)) {
                        auto node = Alloc<VariableDeclaration>();
                        node->declarations = declarations;
                        node->kind = kind;
                        init = Finalize(marker, node);
                        NextToken();
                        left = *init;
                        right = ParseExpression(scope);
                        init.reset();
                    } else if (declarations.size() == 1 && !declarations[0]->init && MatchContextualKeyword("of")) {
                        auto node = Alloc<VariableDeclaration>();
                        node->declarations = declarations;
                        node->kind = kind;
                        init = Finalize(marker, node);
                        NextToken();
                        left = *init;
                        right = ParseAssignmentExpression(scope);
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
                auto init_start_token = ctx->lookahead_;
                auto start_marker = CreateStartMarker();
                auto prev_allow_in = ctx->allow_in_;
                ctx->allow_in_ = false;
                init = InheritCoverGrammar<Expression>([this, &scope] {
                    return ParseAssignmentExpression(scope);
                });
                ctx->allow_in_ = prev_allow_in;

                if (Match(JsTokenType::K_In)) {
                    if (!ctx->is_assignment_target_ || (*init)->type == SyntaxNodeType::AssignmentExpression) {
                        TolerateError(ParseMessages::InvalidLHSInForIn);
                    }

                    NextToken();
                    init = ReinterpretExpressionAsPattern(*init);
                    left = *init;
                    right = ParseExpression(scope);
                    init.reset();
                } else if (MatchContextualKeyword("of")) {
                    if (!ctx->is_assignment_target_ || (*init)->type == SyntaxNodeType::AssignmentExpression) {
                        TolerateError(ParseMessages::InvalidLHSInForIn);
                    }

                    NextToken();
                    init = ReinterpretExpressionAsPattern(*init);
                    left = *init;
                    right = ParseAssignmentExpression(scope);
                    init.reset();
                    for_in = false;
                } else {
                    if (Match(JsTokenType::Comma)) {
                        std::vector<Sp<Expression>> init_seq;
                        Assert(init.has_value() && (*init)->IsExpression(), "init should be an expression");
                        init_seq.push_back(dynamic_pointer_cast<Expression>(*init));

                        while (Match(JsTokenType::Comma)) {
                            NextToken();
                            Sp<Expression> node = IsolateCoverGrammar<Expression>([this, &scope] {
                                return ParseAssignmentExpression(scope);
                            });
                            init_seq.push_back(node);
                        }

                        auto node = Alloc<SequenceExpression>();
                        node->expressions = move(init_seq);
                        init = Finalize(start_marker, node);
                    }
                    Expect(JsTokenType::Semicolon);
                }
            }
        }

        if (!left) {
            if (!Match(JsTokenType::Semicolon)) {
                test = ParseExpression(scope);
            }
            Expect(JsTokenType::Semicolon);
            if (!Match(JsTokenType::RightParen)) {
                update = ParseExpression(scope);
            }
        }

        Sp<Statement> body;
        if (!Match(JsTokenType::RightParen) && ctx->config_.tolerant) {
            TolerateUnexpectedToken(NextToken());
            auto node = Alloc<EmptyStatement>();
            body = Finalize(CreateStartMarker(), node);
        } else {
            Expect(JsTokenType::RightParen);

            auto prev_in_iter = ctx->in_iteration_;
            ctx->in_iteration_ = true;
            body = IsolateCoverGrammar<Statement>([this, &scope] {
                return ParseStatement(scope);
            });
            ctx->in_iteration_ = prev_in_iter;
        }

        if (!left) {
            auto node = Alloc<ForStatement>();
            node->init = init;
            node->test = test;
            node->update = update;
            node->body = body;
            node->scope = move(for_scope);
            return Finalize(marker, node);
        } else if (for_in) {
            auto node = Alloc<ForInStatement>();
            node->left = left;
            node->right = right;
            node->body = body;
            node->scope = move(for_scope);
            return Finalize(marker, node);
        } else {
            auto node = Alloc<ForOfStatement>();
            node->left = left;
            node->right = right;
            node->body = body;
            node->scope = move(for_scope);
            return Finalize(marker, node);
        }

    }

    Sp<ReturnStatement> Parser::ParseReturnStatement(Scope& scope) {
        if (!ctx->in_function_body_) {
            TolerateError(ParseMessages::IllegalReturn);
        }

        auto marker = CreateStartMarker();
        Expect(JsTokenType::K_Return);

        bool has_arg = (!Match(JsTokenType::Semicolon) && !Match(JsTokenType::RightBracket) &&
                        !ctx->has_line_terminator_ && ctx->lookahead_.type != JsTokenType::EOF_) ||
                       ctx->lookahead_.type == JsTokenType::StringLiteral ||
                       ctx->lookahead_.type == JsTokenType::Template;

        auto node = Alloc<ReturnStatement>();
        if (has_arg) {
            node->argument = ParseExpression(scope);
        }

        ConsumeSemicolon();

        return Finalize(marker, node);
    }

    Sp<SwitchStatement> Parser::ParseSwitchStatement(Scope& parent_scope) {
        auto marker = CreateStartMarker();

        auto switch_scope = std::make_unique<Scope>(ScopeType::Switch);
        switch_scope->SetParent(&parent_scope);
        auto& scope = *switch_scope;

        Expect(JsTokenType::K_Switch);
        auto node = Alloc<SwitchStatement>();
        node->scope = move(switch_scope);

        Expect(JsTokenType::LeftParen);
        node->discrimiant = ParseExpression(scope);
        Expect(JsTokenType::RightParen);

        auto prev_in_switch = ctx->in_switch_;
        ctx->in_switch_ = true;

        bool default_found = false;
        Expect(JsTokenType::LeftBracket);
        while (true) {
            if (Match(JsTokenType::RightBracket)) {
                break;
            }
            Sp<SwitchCase> clause = ParseSwitchCase(scope);
            if (!clause->test) {
                if (default_found) {
                    ThrowError(ParseMessages::MultipleDefaultsInSwitch);
                    return nullptr;
                }
                default_found = true;
            }
            node->cases.push_back(clause);
        }
        Expect(JsTokenType::RightBracket);

        ctx->in_switch_ = prev_in_switch;

        return Finalize(marker, node);
    }

    Sp<ThrowStatement> Parser::ParseThrowStatement(Scope& scope) {
        auto marker = CreateStartMarker();
        Expect(JsTokenType::K_Throw);

        if (ctx->has_line_terminator_) {
            ThrowError(ParseMessages::NewlineAfterThrow);
            return nullptr;
        }

        auto node = Alloc<ThrowStatement>();
        node->argument = ParseExpression(scope);
        ConsumeSemicolon();

        return Finalize(marker, node);
    }

    Sp<TryStatement> Parser::ParseTryStatement(Scope& scope) {
        auto marker = CreateStartMarker();
        Expect(JsTokenType::K_Try);
        auto node = Alloc<TryStatement>();

        node->block = ParseBlock(scope, true);
        if (Match(JsTokenType::K_Catch)) {
            node->handler = ParseCatchClause(scope);
        }
        if (Match(JsTokenType::K_Finally)) {
            node->finalizer = ParseFinallyClause(scope);
        }

        return Finalize(marker, node);
    }

    Sp<CatchClause> Parser::ParseCatchClause(Scope& parent_scope) {
        auto marker = CreateStartMarker();

        auto catch_scope = std::make_unique<Scope>(ScopeType::Catch);
        catch_scope->SetParent(&parent_scope);
        auto& scope = *catch_scope;

        Expect(JsTokenType::K_Catch);

        Expect(JsTokenType::LeftParen);
        if (Match(JsTokenType::RightParen)) {
            ThrowUnexpectedToken(ctx->lookahead_);
            return nullptr;
        }

        auto node = Alloc<CatchClause>();
        node->scope = move(catch_scope);

        std::vector<Token> params;
        node->param = ParsePattern(scope, params);

        std::unordered_set<std::string> param_set;
        for (auto& token : params) {
            std::string key = "$" + token.value;
            if (param_set.find(key) != param_set.end()) {
                TolerateError(string(ParseMessages::DuplicateBinding) + ": " + token.value);
            }
            param_set.insert(key);
        }

        if (ctx->strict_ && node->param->type == SyntaxNodeType::Identifier) {
            auto id = dynamic_pointer_cast<Identifier>(node->param);
            if (ctx->scanner_->IsRestrictedWord(id->name)) {
                TolerateError(ParseMessages::StrictCatchVariable);
            }
        }

        Expect(JsTokenType::RightParen);
        node->body = ParseBlock(scope, false);

        return Finalize(marker, node);
    }

    Sp<BlockStatement> Parser::ParseFinallyClause(Scope& scope) {
        Expect(JsTokenType::K_Finally);
        return ParseBlock(scope, true);
    }

    Sp<VariableDeclaration> Parser::ParseVariableStatement(Scope& scope) {
        auto marker = CreateStartMarker();
        Expect(JsTokenType::K_Var);

        auto node = Alloc<VariableDeclaration>();
        node->declarations = ParseVariableDeclarationList(scope, false);
        ConsumeSemicolon();

        node->kind = VarKind::Var;
        return Finalize(marker, node);
    }

    Sp<VariableDeclarator> Parser::ParseVariableDeclaration(Scope& parent_scope, bool in_for) {
        auto marker = CreateStartMarker();
        auto node = Alloc<VariableDeclarator>();
        node->scope->SetParent(&parent_scope);

        vector<Token> params;
        Sp<SyntaxNode> id = ParsePattern(parent_scope, params, VarKind::Var);

        if (ctx->strict_ && id->type == SyntaxNodeType::Identifier) {
            auto identifier = dynamic_pointer_cast<Identifier>(id);
            if (ctx->scanner_->IsRestrictedWord(identifier->name)) {
                TolerateError(ParseMessages::StrictVarName);
            }
        }

        optional<Sp<Expression>> init;
        if (Match(JsTokenType::Assign)) {
            NextToken();
            init = IsolateCoverGrammar<Expression>([this, &scope = *node->scope] {
                return ParseAssignmentExpression(scope);
            });
        } else if (id->type != SyntaxNodeType::Identifier && !in_for) {
            Expect(JsTokenType::Assign);
        }

        node->id = move(id);
        node->init = init;

        return Finalize(marker, node);
    }

    std::vector<Sp<VariableDeclarator>> Parser::ParseVariableDeclarationList(
            Scope& scope, bool in_for) {

        std::vector<Sp<VariableDeclarator>> list;
        list.push_back(ParseVariableDeclaration(scope, in_for));
        while (Match(JsTokenType::Comma)) {
            NextToken();
            list.push_back(ParseVariableDeclaration(scope, in_for));
        }

        return list;
    }

    Sp<WhileStatement> Parser::ParseWhileStatement(Scope& scope) {
        auto marker = CreateStartMarker();
        auto node = Alloc<WhileStatement>();

        Expect(JsTokenType::K_While);
        Expect(JsTokenType::LeftParen);
        node->test = ParseExpression(scope);

        if (!Match(JsTokenType::RightParen) && ctx->config_.tolerant) {
            TolerateUnexpectedToken(NextToken());
            node->body = Finalize(CreateStartMarker(), Alloc<EmptyStatement>());
        } else {
            Expect(JsTokenType::RightParen);

            auto prev_in_interation = ctx->in_iteration_;
            ctx->in_iteration_ = true;
            node->body = ParseStatement(scope);
            ctx->in_iteration_ = prev_in_interation;
        }

        return Finalize(marker, node);
    }

    Sp<WithStatement> Parser::ParseWithStatement(Scope& scope) {
        if (ctx->strict_) {
            TolerateError(ParseMessages::StrictModeWith);
        }

        auto marker = CreateStartMarker();
        auto node = Alloc<WithStatement>();

        Expect(JsTokenType::K_With);
        Expect(JsTokenType::LeftParen);

        node->object = ParseExpression(scope);

        if (!Match(JsTokenType::RightParen) && ctx->config_.tolerant) {
            TolerateUnexpectedToken(NextToken());
            auto empty = Alloc<EmptyStatement>();
            node->body = Finalize(marker, empty);
        } else {
            Expect(JsTokenType::RightParen);
            node->body = ParseStatement(scope);
        }

        return Finalize(marker, node);
    }

    Sp<VariableDeclarator> Parser::ParseLexicalBinding(Scope& scope, VarKind kind, bool &in_for) {
        auto start_marker = CreateStartMarker();
        auto node = Alloc<VariableDeclarator>();
        std::vector<Token> params;
        node->id = ParsePattern(scope, params, kind);

        if (ctx->strict_ && node->id->type == SyntaxNodeType::Identifier) {
            auto id = dynamic_pointer_cast<Identifier>(node->id);
            if (ctx->scanner_->IsRestrictedWord(id->name)) {
                TolerateError(ParseMessages::StrictVarName);
            }
        }

        if (kind == VarKind::Const) {
            if (!Match(JsTokenType::K_In) && !MatchContextualKeyword("of")) {
                if (Match(JsTokenType::Assign)) {
                    NextToken();
                    node->init = IsolateCoverGrammar<Expression>([this, &scope] {
                        return ParseAssignmentExpression(scope);
                    });
                } else {
                    ThrowError(ParseMessages::DeclarationMissingInitializer);
                }
            }
        } else if ((!in_for && node->id->type != SyntaxNodeType::Identifier) || Match(JsTokenType::Assign)) {
            Expect(JsTokenType::Assign);
            node->init = IsolateCoverGrammar<Expression>([this, &scope] {
                return ParseAssignmentExpression(scope);
            });
        }

        return Finalize(start_marker, node);
    }

    std::vector<Sp<VariableDeclarator>> Parser::ParseBindingList(Scope& scope, VarKind kind, bool& in_for) {
        std::vector<Sp<VariableDeclarator>> list;
        list.push_back(ParseLexicalBinding(scope, kind, in_for));

        while (Match(JsTokenType::Comma)) {
            NextToken();
            list.push_back(ParseLexicalBinding(scope, kind, in_for));
        }

        return list;
    }

    Sp<RestElement> Parser::ParseRestElement(Scope& scope, std::vector<Token> &params) {
        auto marker = CreateStartMarker();
        auto node = Alloc<RestElement>();

        Expect(JsTokenType::Spread);
        node->argument = ParsePattern(scope, params, VarKind::Invalid);
        if (Match(JsTokenType::Assign)) {
            ThrowError(ParseMessages::DefaultRestParameter);
            return nullptr;
        }
        if (!Match(JsTokenType::RightParen)) {
            ThrowError(ParseMessages::ParameterAfterRestParameter);
            return nullptr;
        }

        return Finalize(marker, node);
    }

    Sp<Statement> Parser::ParseStatementListItem(Scope& scope) {
        Sp<Statement> statement;
        ctx->is_assignment_target_ = true;
        ctx->is_binding_element_ = true;

        if (IsKeywordToken(ctx->lookahead_.type)) {
            if (ctx->lookahead_.value == "export") {
                if (ctx->is_module_) {
                    TolerateUnexpectedToken(ctx->lookahead_, ParseMessages::IllegalExportDeclaration);
                }
                statement = ParseExportDeclaration(scope);
            } else if (ctx->lookahead_.value == "import") {
                if (MatchImportCall()) {
                    statement = ParseExpressionStatement(scope);
                } else {
                    if (!ctx->is_module_) {
                        TolerateUnexpectedToken(ctx->lookahead_, ParseMessages::IllegalImportDeclaration);
                    }
                    statement = ParseImportDeclaration(scope);
                }
            } else if (ctx->lookahead_.value == "const") {
                bool in_for = false;
                statement = ParseLexicalDeclaration(scope, in_for);
            } else if (ctx->lookahead_.value == "function") {
                statement = ParseFunctionDeclaration(scope, false);
            } else if (ctx->lookahead_.value == "class") {
                statement = ParseClassDeclaration(scope, false);
            } else if (ctx->lookahead_.value == "let") {
                if (IsLexicalDeclaration()) {
                    bool in_for = false;
                    statement = ParseLexicalDeclaration(scope, in_for);
                } else {
                    statement = ParseStatement(scope);
                }
            } else {
                statement = ParseStatement(scope);
            }
        } else {
            statement = ParseStatement(scope);
        }

        return statement;
    }

    Sp<ExportSpecifier> Parser::ParseExportSpecifier(Scope& scope) {
        auto start_marker = CreateStartMarker();

        auto local = ParseIdentifierName();
        auto exported = std::make_shared<Identifier>();
        (*exported) = (*local);

        scope.AddUnresolvedId(local);

        if (MatchContextualKeyword("as")) {
            NextToken();
            exported = ParseIdentifierName();
        }

        auto node = Alloc<ExportSpecifier>();
        node->local = std::move(local);
        node->exported = std::move(exported);
        return Finalize(start_marker, node);
    }

    Sp<Declaration> Parser::ParseExportDeclaration(Scope& scope) {
        auto module_scope = scope.CastToMoudle();
        Assert(module_scope, "scope should be module scope");

        if (ctx->in_function_body_) {
            ThrowError(ParseMessages::IllegalExportDeclaration);
        }

        auto start_marker = CreateStartMarker();
        Expect(JsTokenType::K_Export);

        Sp<Declaration> export_decl;
        if (Match(JsTokenType::K_Default)) {
            NextToken();
            if (Match(JsTokenType::K_Function)) {
                auto node = Alloc<ExportDefaultDeclaration>();
                node->declaration = ParseFunctionDeclaration(scope, true);

                Assert(module_scope->export_manager.ResolveDefaultDecl(node) == ExportManager::Ok,
                       "resolve export failed");

                export_decl = Finalize(start_marker, node);
            } else if (Match(JsTokenType::K_Class)) {
                auto node = Alloc<ExportDefaultDeclaration>();
                node->declaration = ParseClassExpression(scope);

                Assert(module_scope->export_manager.ResolveDefaultDecl(node) == ExportManager::Ok,
                       "resolve export failed");

                export_decl = Finalize(start_marker, node);
            } else if (MatchContextualKeyword("async")) {
                auto node = Alloc<ExportDefaultDeclaration>();
                if (MatchAsyncFunction()) {
                    node->declaration = ParseFunctionDeclaration(scope, true);
                } else {
                    node->declaration = ParseAssignmentExpression(scope);
                }

                Assert(module_scope->export_manager.ResolveDefaultDecl(node) == ExportManager::Ok,
                       "resolve export failed");

                export_decl = Finalize(start_marker, node);
            } else {
                if (MatchContextualKeyword("from")) {
                    ThrowError(ParseMessages::UnexpectedToken, ctx->lookahead_.value);
                }
                Sp<SyntaxNode> decl;
                if (Match(JsTokenType::LeftBracket)) {
                    decl = ParseObjectInitializer(scope);
                } else if (Match(JsTokenType::LeftBrace)) {
                    decl = ParseArrayInitializer(scope);
                } else {
                    decl = ParseAssignmentExpression(scope);
                }
                ConsumeSemicolon();
                auto node = Alloc<ExportDefaultDeclaration>();
                node->declaration = move(decl);

                Assert(module_scope->export_manager.ResolveDefaultDecl(node) == ExportManager::Ok,
                       "resolve export failed");

                export_decl = Finalize(start_marker, node);
            }
        } else if (Match(JsTokenType::Mul)) {
            NextToken();
            if (!MatchContextualKeyword("from")) {
                string message;
                if (!ctx->lookahead_.value.empty()) {
                    message = ParseMessages::UnexpectedToken;
                } else {
                    message = ParseMessages::MissingFromClause;
                }
                ThrowError(message, ctx->lookahead_.value);
            }
            NextToken();
            auto node = Alloc<ExportAllDeclaration>();
            node->source = ParseModuleSpecifier();
            ConsumeSemicolon();

            // notify other thread to parse
            export_all_decl_created_listener.Emit(node);

            Assert(module_scope->export_manager.ResolveAllDecl(node) == ExportManager::Ok,
                    "resolve export failed");

            export_decl = Finalize(start_marker, node);
        } else if (IsKeywordToken(ctx->lookahead_.type)) {
            auto node = Alloc<ExportNamedDeclaration>();

            if (ctx->lookahead_.value == "let" || ctx->lookahead_.value == "const") {
                bool in_for = false;
                node->declaration = ParseLexicalDeclaration(scope, in_for);
            } else if (
                    ctx->lookahead_.value == "var" ||
                    ctx->lookahead_.value == "class" ||
                    ctx->lookahead_.value == "function"
            ) {
                node->declaration = ParseStatementListItem(scope);
            } else {
                ThrowUnexpectedToken(ctx->lookahead_);
            }

            Assert(module_scope->export_manager.ResolveNamedDecl(node) == ExportManager::Ok,
                   "resolve export failed");

            export_named_decl_created_listener.Emit(node);
            export_decl = Finalize(start_marker, node);
        } else if (MatchAsyncFunction()) {
            auto node = Alloc<ExportNamedDeclaration>();
            node->declaration = ParseFunctionDeclaration(scope, false);

            Assert(module_scope->export_manager.ResolveNamedDecl(node) == ExportManager::Ok,
                   "resolve export failed");

            export_named_decl_created_listener.Emit(node);
            export_decl = Finalize(start_marker, node);
        } else {
            auto node = Alloc<ExportNamedDeclaration>();
            bool is_export_from_id = false;

            Expect(JsTokenType::LeftBracket);
            while (!Match(JsTokenType::RightBracket)) {
                is_export_from_id = is_export_from_id || Match(JsTokenType::K_Default);
                node->specifiers.push_back(ParseExportSpecifier(scope));
                if (!Match(JsTokenType::RightBracket)) {
                    Expect(JsTokenType::Comma);
                }
            }
            Expect(JsTokenType::RightBracket);

            if (MatchContextualKeyword("from")) {
                NextToken();
                node->source = ParseModuleSpecifier();
                ConsumeSemicolon();
            } else if (is_export_from_id) {
                string message;
                if (!ctx->lookahead_.value.empty()) {
                    message = ParseMessages::UnexpectedToken;
                } else {
                    message = ParseMessages::MissingFromClause;
                }
                ThrowError(message, ctx->lookahead_.value);
            } else {
                ConsumeSemicolon();
            }

            Assert(module_scope->export_manager.ResolveNamedDecl(node) == ExportManager::Ok,
                   "resolve export failed");

            export_named_decl_created_listener.Emit(node);
            export_decl = Finalize(start_marker, node);
        }

        return export_decl;
    }

    Sp<Expression> Parser::ParseAssignmentExpression(Scope& scope) {
        Sp<Expression> expr;

        if (!ctx->allow_yield_ && Match(JsTokenType::K_Yield)) {
            expr = ParseYieldExpression(scope);
        } else {
            auto start_marker = CreateStartMarker();
            auto token = ctx->lookahead_;
            expr = ParseConditionalExpression(scope);

            if (
                    token.type == JsTokenType::Identifier &&
                    (token.lineNumber == ctx->lookahead_.lineNumber) &&
                            token.value == "async"
            ) {
                if (ctx->lookahead_.type == JsTokenType::Identifier || Match(JsTokenType::K_Yield)) {
                    Sp<SyntaxNode> arg = ParsePrimaryExpression(scope);
                    arg = ReinterpretExpressionAsPattern(arg);

                    auto node = Alloc<ArrowParameterPlaceHolder>();
                    node->params.push_back(arg);
                    node->async = true;
                    expr = move(node);

                }
            }

            if (expr->type == SyntaxNodeType::ArrowParameterPlaceHolder || Match(JsTokenType::Arrow)) {

                ctx->is_assignment_target_ = false;
                ctx->is_binding_element_ = false;
                bool is_async = false;
                if (auto placeholder = dynamic_pointer_cast<ArrowParameterPlaceHolder>(expr); placeholder) {
                    is_async = placeholder->async;
                }

                if (auto list = ReinterpretAsCoverFormalsList(expr); list.has_value()) {
                    if (ctx->has_line_terminator_) {
                        TolerateUnexpectedToken(ctx->lookahead_);
                    }
                    ctx->first_cover_initialized_name_error_.reset();

                    bool prev_strict = ctx->strict_;
                    bool prev_allow_strict_directive = ctx->allow_strict_directive_;
                    ctx->allow_strict_directive_ = list->simple;

                    bool prev_allow_yield = ctx->allow_yield_;
                    bool prev_await = ctx->await_;
                    ctx->allow_yield_ = true;
                    ctx->await_ = is_async;

                    auto marker = CreateStartMarker();
                    Expect(JsTokenType::Arrow);
                    Sp<SyntaxNode> body;

                    if (Match(JsTokenType::LeftBracket)) {
                        bool prev_allow_in = ctx->allow_in_;
                        ctx->allow_in_ = true;
                        body = ParseFunctionSourceElements(scope);
                        ctx->allow_in_ = prev_allow_in;
                    } else {
                        body = IsolateCoverGrammar<SyntaxNode>([this, &scope] {
                            return ParseAssignmentExpression(scope);
                        });
                    }

                    bool expression = body->type != SyntaxNodeType::BlockStatement;

                    if (ctx->strict_ && list->first_restricted) {
                        ThrowUnexpectedToken(*list->first_restricted, list->message);
                        return nullptr;
                    }
                    if (ctx->strict_ && list->stricted) {
                        TolerateUnexpectedToken(*list->stricted, list->message);
                    }

                    if (is_async) {
                        auto node = Alloc<ArrowFunctionExpression>();
                        node->params = list->params;
                        node->body = body;
                        node->expression = expression;
                        node->async = true;
                        expr = Finalize(marker, node);
                    } else {
                        auto node = Alloc<ArrowFunctionExpression>();
                        node->params = list->params;
                        node->body = body;
                        node->expression = expression;
                        expr = Finalize(marker, node);
                    }

                    ctx->strict_ = prev_strict;
                    ctx->allow_strict_directive_ = prev_allow_strict_directive;
                    ctx->allow_yield_ = prev_allow_yield;
                    ctx->await_ = prev_await;
                }
            } else {

                if (MatchAssign()) {
                    if (!ctx->is_assignment_target_) {
                        TolerateError(ParseMessages::InvalidLHSInAssignment);
                    }

                    if (ctx->strict_ && expr->type == SyntaxNodeType::Identifier) {
                        auto id = dynamic_pointer_cast<Identifier>(expr);
                        if (ctx->scanner_->IsRestrictedWord(id->name)) {
                            TolerateUnexpectedToken(token, ParseMessages::StrictLHSAssignment);
                        }
                    }

                    auto temp = Alloc<AssignmentExpression>();

                    if (!Match(JsTokenType::Assign)) {
                        ctx->is_assignment_target_ = false;
                        ctx->is_binding_element_ = false;
                    } else {
//                        Assert(!!temp->left, "left should not be null");
                    }
                    temp->left = ReinterpretExpressionAsPattern(expr);

                    token = NextToken();
//                    auto operator_ = token.value;
                    temp->right = IsolateCoverGrammar<Expression>([this, &scope] {
                        return ParseAssignmentExpression(scope);
                    });
                    auto tokenView = TokenTypeToLiteral(token.type);
                    temp->operator_ = std::string(tokenView.data(), tokenView.size());
                    expr = Finalize(start_marker, temp);
                    ctx->first_cover_initialized_name_error_.reset();
                }
            }

        }

        // TODO: wait to complete
        return expr;
    }

    Sp<Expression> Parser::ParseConditionalExpression(Scope& scope) {
        auto marker = CreateStartMarker();

        Sp<Expression> expr = InheritCoverGrammar<Expression>([this, &scope] {
            return ParseBinaryExpression(scope);
        });

        if (Match(JsTokenType::Ask)) {
            NextToken();
            auto node = Alloc<ConditionalExpression>();

            bool prev_allow_in = ctx->allow_in_;
            ctx->allow_in_ = true;

            node->consequent = IsolateCoverGrammar<Expression>([this, &scope] {
                return ParseAssignmentExpression(scope);
            });
            ctx->allow_in_ = prev_allow_in;

            Expect(JsTokenType::Colon);
            node->alternate = IsolateCoverGrammar<Expression>([this, &scope] {
                return ParseAssignmentExpression(scope);
            });

            node->test = move(expr);
            expr = move(node);

            ctx->is_assignment_target_ = false;
            ctx->is_binding_element_ = false;
        }

        return Finalize(marker, expr);
    }

    Sp<Expression> Parser::ParseBinaryExpression(Scope& scope) {
        Token start_token = ctx->lookahead_;

        auto expr = InheritCoverGrammar<Expression>([this, &scope] {
            return ParseExponentiationExpression(scope);
        });

        Token token = ctx->lookahead_;
        int prec = BinaryPrecedence(token);
        if (prec > 0) {
            NextToken();

            return ParseBinaryExpression(scope, expr, token);
        }

        return expr;
    }

    Sp<Expression> Parser::ParseBinaryExpression(Scope &scope,
                                                 const Sp<Expression>& left,
                                                 const Token &left_tk) {
        auto marker = CreateStartMarker();

        auto expr = IsolateCoverGrammar<Expression>([this, &scope] {
            return ParseExponentiationExpression(scope);
        });

        int left_prec = BinaryPrecedence(left_tk);
        while (true) {
            Token right_tk = ctx->lookahead_;
            int right_prec = BinaryPrecedence(right_tk);

            if (left_prec < right_prec) {
                NextToken();
                expr = ParseBinaryExpression(scope, expr, right_tk);
            } else if (left_prec == right_prec) {
                if (left_prec <= 0) {
                    return expr;
                }
                auto binary = Alloc<BinaryExpression>();
                binary->left = left;
                binary->right = expr;
                std::string_view tokenView = TokenTypeToLiteral(left_tk.type);
                binary->operator_ = std::string(tokenView.data(), tokenView.size());
                if (ctx->config_.constant_folding) {
                    expr = ContantFolding::TryBinaryExpression(binary);
                } else {
                    expr = binary;
                }
                NextToken();
                return Finalize(marker, ParseBinaryExpression(scope, expr, right_tk));
            } else {  // left_op > right_op
                auto binary = Alloc<BinaryExpression>();
                auto tokenView = TokenTypeToLiteral(left_tk.type);
                binary->operator_ = std::string(tokenView.data(), tokenView.size());
                binary->left = left;
                binary->right = expr;

                if (ctx->config_.constant_folding) {
                    expr = Finalize(marker, ContantFolding::TryBinaryExpression(binary));
                } else {
                    expr = Finalize(marker, binary);
                }

                if (right_prec <= 0) {
                    return expr;
                }
                NextToken();
                return ParseBinaryExpression(scope, expr, right_tk);
            }
        }
        Assert(false, "unreachable code");
        return nullptr;
    }

    Sp<Expression> Parser::ParseExponentiationExpression(Scope& scope) {
        auto start = CreateStartMarker();

        Sp<Expression> expr = InheritCoverGrammar<Expression>([this, &scope] {
            return ParseUnaryExpression(scope);
        });

        Assert(!!expr, "ParseExponentiationExpression: expr should not be null");

        if (expr->type != SyntaxNodeType::UnaryExpression && Match(JsTokenType::Pow)) {
            NextToken();
            ctx->is_assignment_target_ = false;
            ctx->is_binding_element_ = false;
            auto node = Alloc<BinaryExpression>();
            node->left = expr;
            node->right = IsolateCoverGrammar<Expression>([this, &scope, &node] {
                return ParseExponentiationExpression(scope);
            });
            node->operator_ = "**";
            expr = Finalize(start, node);
        }

        return expr;
    }

    Sp<Expression> Parser::ParseUnaryExpression(Scope& scope) {
        Sp<Expression> expr;

        if (
            Match(JsTokenType::Plus) || Match(JsTokenType::Minus) || Match(JsTokenType::Wave) || Match(JsTokenType::Not) ||
            Match(JsTokenType::K_Delete) || Match(JsTokenType::K_Void) || Match(JsTokenType::K_Typeof)
            ) {
            auto marker = CreateStartMarker();
            Token token = ctx->lookahead_;
            NextToken();
            expr = InheritCoverGrammar<Expression>([this, &scope] {
                return ParseUnaryExpression(scope);
            });
            auto node = Alloc<UnaryExpression>();
            auto tokenView = TokenTypeToLiteral(token.type);
            node->operator_ = std::string(tokenView.data(), tokenView.size());
            node->argument = expr;
            node->prefix = true;
            expr = Finalize(marker, node);
            if (ctx->strict_ && node->operator_ == "delete" && node->argument->type == SyntaxNodeType::Identifier) {
                TolerateError(ParseMessages::StrictDelete);
            }
            ctx->is_assignment_target_ = false;
            ctx->is_binding_element_ = false;
        } else if (ctx->await_ && MatchContextualKeyword("await")) {
            expr = ParseAwaitExpression(scope);
        } else {
            expr = ParseUpdateExpression(scope);
        }

        return expr;
    }

    Sp<AwaitExpression> Parser::ParseAwaitExpression(Scope& scope) {
        auto marker = CreateStartMarker();
        NextToken();
        auto node = Alloc<AwaitExpression>();
        node->argument = ParseUnaryExpression(scope);
        return Finalize(marker, node);
    }

    Sp<Expression> Parser::ParseUpdateExpression(Scope& scope) {
        Sp<Expression> expr;
        auto start_marker = CreateStartMarker();

        if (Match(JsTokenType::Increase) || Match(JsTokenType::Decrease)) {
            auto marker = CreateStartMarker();
            Token token = NextToken();
            expr = InheritCoverGrammar<Expression>([this, &scope] {
                return ParseUnaryExpression(scope);
            });
            if (ctx->strict_ && expr->type == SyntaxNodeType::Identifier) {
                auto id = dynamic_pointer_cast<Identifier>(expr);
                if (ctx->scanner_->IsRestrictedWord(id->name)) {
                    TolerateError(ParseMessages::StrictLHSPrefix);
                }
            }
            if (!ctx->is_assignment_target_) {
                TolerateError(ParseMessages::InvalidLHSInAssignment);
            }
            auto node = Alloc<UpdateExpression>();
            node->prefix = true;
            node->operator_ = token.value;
            node->argument = expr;
            expr = Finalize(marker, node);
            ctx->is_assignment_target_ = false;
            ctx->is_binding_element_ = false;
        } else {
            expr = InheritCoverGrammar<Expression>([this, &scope] {
                return ParseLeftHandSideExpressionAllowCall(scope);
            });
            if (!ctx->has_line_terminator_ && IsPunctuatorToken(ctx->lookahead_.type)) {
                if (Match(JsTokenType::Increase) || Match(JsTokenType::Decrease)) {
                    if (ctx->strict_ && expr->type == SyntaxNodeType::Identifier) {
                        auto id = dynamic_pointer_cast<Identifier>(expr);
                        if (ctx->scanner_->IsRestrictedWord(id->name)) {
                            TolerateError(ParseMessages::StrictLHSPostfix);
                        }
                    }
                    if (!ctx->is_assignment_target_) {
                        TolerateError(ParseMessages::InvalidLHSInAssignment);
                    }
                    ctx->is_assignment_target_ = false;
                    ctx->is_binding_element_ = false;
                    auto node = Alloc<UpdateExpression>();
                    node->prefix = false;
                    Token token = NextToken();
                    node->operator_ = token.value;
                    node->argument = expr;
                    expr = Finalize(start_marker, node);
                }
            }
        }

        return expr;
    }

    Sp<Expression> Parser::ParseLeftHandSideExpression(Scope& scope) {
        Assert(ctx->allow_in_, "callee of new expression always allow in keyword.");

        auto start_marker = CreateStartMarker();
        Sp<Expression> expr;
        if (Match(JsTokenType::K_Super) && ctx->in_function_body_) {
            expr = ParseSuper();
        } else {
            expr = InheritCoverGrammar<Expression>([this, &scope] {
                if (Match(JsTokenType::K_New)) {
                    return ParseNewExpression(scope);
                } else {
                    return ParsePrimaryExpression(scope);
                }
            });
        }

        while (true) {
            if (Match(JsTokenType::LeftBrace)) {
                ctx->is_binding_element_ = false;
                ctx->is_assignment_target_ = true;
                Expect(JsTokenType::LeftBrace);
                auto node = Alloc<MemberExpression>();
                node->computed = true;
                node->property = IsolateCoverGrammar<Expression>([this, &scope] {
                    return ParseExpression(scope);
                });
                Expect(JsTokenType::RightBrace);
                node->object = move(expr);
                expr = Finalize(start_marker, node);
            } else if (Match(JsTokenType::Dot)) {
                ctx->is_binding_element_ = false;
                ctx->is_assignment_target_ = true;
                Expect(JsTokenType::Dot);
                auto node = Alloc<MemberExpression>();
                node->computed = false;
                node->property = ParseIdentifierName();
                node->object = move(expr);
                expr = Finalize(start_marker, node);
            } else if (ctx->lookahead_.type == JsTokenType::Template && ctx->lookahead_.head) {
                auto node = Alloc<TaggedTemplateExpression>();
                node->quasi = ParseTemplateLiteral(scope);
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

        Expect(JsTokenType::K_Super);
        if (!Match(JsTokenType::LeftBrace) && !Match(JsTokenType::Dot)) {
            ThrowUnexpectedToken(ctx->lookahead_);
            return nullptr;
        }

        auto node = Alloc<Super>();
        return Finalize(start_marker, node);
    }

    Sp<ImportDeclaration> Parser::ParseImportDeclaration(Scope& scope) {
        auto module_scope = scope.CastToMoudle();
        Assert(module_scope, "import specifier is not in module scope");

        if (ctx->in_function_body_) {
            ThrowError(ParseMessages::IllegalImportDeclaration);
        }

        auto start_marker = CreateStartMarker();
        Expect(JsTokenType::K_Import);

        Sp<Literal> src;
        std::vector<Sp<SyntaxNode>> specifiers;

        if (ctx->lookahead_.type == JsTokenType::StringLiteral) {
            src = ParseModuleSpecifier();
        } else {
            if (Match(JsTokenType::LeftBracket)) {
                auto children = ParseNamedImports(scope);
                specifiers.insert(specifiers.end(), children.begin(), children.end());
            } else if (Match(JsTokenType::Mul)) {
                specifiers.push_back(ParseImportNamespaceSpecifier(scope));
            } else if (IsIdentifierName(ctx->lookahead_) && !Match(JsTokenType::K_Default)) {
                auto default_ = ParseImportDefaultSpecifier(scope);
                specifiers.push_back(move(default_));

                if (Match(JsTokenType::Comma)) {
                    NextToken();
                    if (Match(JsTokenType::Mul)) {
                        specifiers.push_back(ParseImportNamespaceSpecifier(scope));
                    } else if (Match(JsTokenType::LeftBracket)) {
                        auto children = ParseNamedImports(scope);
                        specifiers.insert(specifiers.end(), children.begin(), children.end());
                    } else {
                        ThrowUnexpectedToken(ctx->lookahead_);
                    }
                }
            } else {
                ThrowUnexpectedToken(NextToken());
            }

            if (!MatchContextualKeyword("from")) {
                string message;
                if (!ctx->lookahead_.value.empty()) {
                    message = ParseMessages::UnexpectedToken;
                } else {
                    message = ParseMessages::MissingFromClause;
                }
                ThrowError(message, ctx->lookahead_.value);
            }
            NextToken();
            src = ParseModuleSpecifier();
        }
        ConsumeSemicolon();

        auto node = Alloc<ImportDeclaration>();
        node->specifiers = move(specifiers);
        node->source = move(src);

        {
            auto finalized_node = Finalize(start_marker, node);

            // notify callback to analyze another module
            import_decl_created_listener.Emit(finalized_node);

            // notify module scope to analyze variable ref
            if (module_scope->import_manager.ResolveImportDecl(finalized_node) != ImportManager::EC::Ok) {
                ThrowError("resolver import relation error");
            }

            return finalized_node;
        }
    }

    std::vector<Sp<SyntaxNode>> Parser::ParseNamedImports(Scope& scope) {
        Expect(JsTokenType::LeftBracket);
        std::vector<Sp<SyntaxNode>> specifiers;
        while (!Match(JsTokenType::RightBracket)) {
            specifiers.push_back(ParseImportSpecifier(scope));
            if (!Match(JsTokenType::RightBracket)) {
                Expect(JsTokenType::Comma);
            }
        }
        Expect(JsTokenType::RightBracket);

        return specifiers;
    }

    Sp<ImportSpecifier> Parser::ParseImportSpecifier(Scope& scope) {
        auto start_marker = CreateStartMarker();

        Sp<Identifier> imported;
        Sp<Identifier> local;
        if (ctx->lookahead_.type == JsTokenType::Identifier) {
            imported = ParseVariableIdentifier(LeftValueScope::default_, VarKind::Invalid);
            local = std::make_shared<Identifier>();
            (*local) = (*imported);
            if (MatchContextualKeyword("as")) {
                NextToken();
                local = ParseVariableIdentifier(scope, VarKind::Invalid);
            }
            scope.CreateVariable(local, VarKind::Var);
        } else {  // maybe keywords
            imported = ParseIdentifierName();
            local = std::make_shared<Identifier>();
            (*local) = (*imported);
            if (MatchContextualKeyword("as")) {
                NextToken();
                local = ParseVariableIdentifier(scope, VarKind::Invalid);
            } else {
                ThrowUnexpectedToken(NextToken());
            }
            scope.CreateVariable(local, VarKind::Var);
        }

        auto node = Alloc<ImportSpecifier>();
        node->local = local;
        node->imported = imported;
        return Finalize(start_marker, node);
    }

    Sp<Literal> Parser::ParseModuleSpecifier() {
        auto start_marker = CreateStartMarker();

        if (ctx->lookahead_.type != JsTokenType::StringLiteral) {
            ThrowError(ParseMessages::InvalidModuleSpecifier);
        }

        Token token = NextToken();
        auto node = Alloc<Literal>();
        node->ty = Literal::Ty::String;
        node->str_ = token.value;
        node->raw = GetTokenRaw(token);
        return Finalize(start_marker, node);
    }

    Sp<ImportDefaultSpecifier> Parser::ParseImportDefaultSpecifier(Scope& scope) {
        auto start_marker = CreateStartMarker();
        auto local = ParseIdentifierName();
        auto node = Alloc<ImportDefaultSpecifier>();
        node->local = local;
        scope.CreateVariable(local, VarKind::Var);
        return Finalize(start_marker, node);
    }

    Sp<ImportNamespaceSpecifier> Parser::ParseImportNamespaceSpecifier(Scope& scope) {
        auto start_marker = CreateStartMarker();

        Expect(JsTokenType::Mul);
        if (!MatchContextualKeyword("as")) {
            ThrowError(ParseMessages::NoAsAfterImportNamespace);
        }
        NextToken();
        auto local = ParseIdentifierName();

        auto node = Alloc<ImportNamespaceSpecifier>();
        node->local = move(local);

        scope.CreateVariable(node->local, VarKind::Var);
        return Finalize(start_marker, node);
    }

    Sp<Declaration> Parser::ParseLexicalDeclaration(Scope& scope, bool &in_for) {
        auto start_marker = CreateStartMarker();
        auto node = Alloc<VariableDeclaration>();
        Token next = NextToken();
        if (next.value == "let") {
            node->kind = VarKind::Let;
        } else if (next.value == "const") {
            node->kind = VarKind::Const;
        } else {
            ThrowUnexpectedToken(next);
        }

        node->declarations = ParseBindingList(scope, node->kind, in_for);
        ConsumeSemicolon();

        return Finalize(start_marker, node);
    }

    Sp<Identifier> Parser::ParseVariableIdentifier(Scope& scope, VarKind kind) {
        auto marker = CreateStartMarker();

        Token token = NextToken();
        if (Match(JsTokenType::K_Yield)) {
            if (ctx->strict_) {
                TolerateUnexpectedToken(token, ParseMessages::StrictReservedWord);
            } else if (!ctx->allow_yield_) {
                ThrowUnexpectedToken(token);
                return nullptr;
            }
        } else if (token.type != JsTokenType::Identifier) {
            ThrowUnexpectedToken(token);
            return nullptr;
        } else if ((ctx->is_module_ || ctx->await_) && token.type == JsTokenType::Identifier && token.value == "await") {
            TolerateUnexpectedToken(token);
        }

        auto node = Alloc<Identifier>();
        node->name = token.value;
        scope.CreateVariable(node, kind);
        return Finalize(marker, node);
    }

    Sp<SyntaxNode> Parser::ParsePattern(Scope& scope, std::vector<Token> &params, VarKind kind) {
        Sp<SyntaxNode> pattern;

        if (Match(JsTokenType::LeftBrace)) {
            pattern = ParseArrayPattern(scope, params, kind);
        } else if (Match(JsTokenType::LeftBracket)) {
            pattern = ParseObjectPattern(scope, params, kind);
        } else {
            if (Match(JsTokenType::K_Let) && (kind == VarKind::Const || kind == VarKind::Let)) {
                TolerateUnexpectedToken(ctx->lookahead_, ParseMessages::LetInLexicalBinding);
            }
            params.push_back(ctx->lookahead_);
            pattern = ParseVariableIdentifier(scope, kind);
        }

        return pattern;
    }

    Sp<SyntaxNode> Parser::ParsePatternWithDefault(Scope& scope, std::vector<Token> &params, VarKind kind) {
        Token start_token_ = ctx->lookahead_;

        auto pattern = ParsePattern(scope, params, kind);
        if (Match(JsTokenType::Assign)) {
            NextToken();
            bool prev_allow_yield = ctx->allow_yield_;
            ctx->allow_yield_ = true;
            auto right = IsolateCoverGrammar<Expression>([this, &scope] {
                return ParseAssignmentExpression(scope);
            });
            ctx->allow_yield_ = prev_allow_yield;
            auto node = Alloc<AssignmentPattern>();
            node->left = move(pattern);
            node->right = move(right);
            pattern = node;
        }

        return pattern;
    }

    Sp<RestElement> Parser::ParseBindingRestElement(Scope& scope, std::vector<Token> &params, VarKind kind) {
        auto start_marker = CreateStartMarker();

        Expect(JsTokenType::Spread);
        auto node = Alloc<RestElement>();
        node->argument = ParsePattern(scope, params, kind);

        return Finalize(start_marker, node);
    }

    Sp<ArrayPattern> Parser::ParseArrayPattern(Scope& scope, std::vector<Token> &params, VarKind kind) {
        auto start_marker = CreateStartMarker();

        Expect(JsTokenType::LeftBrace);
        std::vector<optional<Sp<SyntaxNode>>> elements;
        while (!Match(JsTokenType::RightBrace)) {
            if (Match(JsTokenType::Comma)) {
                NextToken();
                elements.emplace_back(nullopt);
            } else {
                if (Match(JsTokenType::Spread)) {
                    elements.emplace_back(ParseBindingRestElement(scope, params, kind));
                    break;
                } else {
                    elements.emplace_back(ParsePatternWithDefault(scope, params, kind));
                }
                if (!Match(JsTokenType::RightBrace)) {
                    Expect(JsTokenType::Comma);
                }
            }
        }
        Expect(JsTokenType::RightBrace);

        auto node = Alloc<ArrayPattern>();
        node->elements = move(elements);
        return Finalize(start_marker, node);
    }

    Sp<Property> Parser::ParsePropertyPattern(Scope& scope, std::vector<Token> &params, VarKind kind) {
        auto start_marker = CreateStartMarker();

        auto node = Alloc<Property>();
        node->computed = false;
        node->shorthand = false;
        node->method = false;

        if (ctx->lookahead_.type == JsTokenType::Identifier) {
            Token keyToken = ctx->lookahead_;
            node->key = ParseVariableIdentifier(LeftValueScope::default_, kind);

            auto id_ = Alloc<Identifier>();
            id_->name = keyToken.value;
            auto init = Finalize(start_marker, id_);

            if (Match(JsTokenType::Assign)) {
                params.push_back(keyToken);
                node->shorthand = true;
                NextToken();
                auto expr = ParseAssignmentExpression(scope);
                auto assign = Alloc<AssignmentPattern>();
                assign->left = move(init);
                assign->right = move(expr);
                node->value = Finalize(StartNode(keyToken), assign);
            } else if (!Match(JsTokenType::Colon)) {  // shorthand!
                params.push_back(keyToken);

                scope.CreateVariable(init, kind);

                node->shorthand = true;
                node->value = move(init);
            } else {
                Expect(JsTokenType::Colon);
                node->value = ParsePatternWithDefault(scope, params, kind);
            }
        } else {
            node->computed = Match(JsTokenType::LeftBrace);
            node->key = ParseObjectPropertyKey(scope);
            Expect(JsTokenType::Colon);
            node->value = ParsePatternWithDefault(scope, params, kind);
        }

        return Finalize(start_marker, node);
    }

    Sp<RestElement> Parser::ParseRestProperty(Scope& scope, std::vector<Token> &params, VarKind kind) {
        auto start_marker = CreateStartMarker();
        Expect(JsTokenType::Spread);
        auto arg = ParsePattern(scope, params, VarKind::Invalid);
        if (Match(JsTokenType::Assign)) {
            ThrowError(ParseMessages::DefaultRestProperty);
        }
        if (!Match(JsTokenType::RightBracket)) {
            ThrowError(ParseMessages::PropertyAfterRestProperty);
        }
        auto node = Alloc<RestElement>();
        node->argument = move(arg);
        return Finalize(start_marker, node);
    }

    Sp<ObjectPattern> Parser::ParseObjectPattern(Scope& scope, std::vector<Token> &params, VarKind kind) {
        auto start_marker = CreateStartMarker();
        std::vector<Sp<SyntaxNode>> props;

        Expect(JsTokenType::LeftBracket);
        while (!Match(JsTokenType::RightBracket)) {
            if (Match(JsTokenType::Spread)) {
                props.push_back(ParseRestProperty(scope, params, kind));
            } else {
                props.push_back(ParsePropertyPattern(scope, params, kind));
            }

            if (!Match(JsTokenType::RightBracket)) {
                Expect(JsTokenType::Comma);
            }
        }
        Expect(JsTokenType::RightBracket);

        auto node = Alloc<ObjectPattern>();
        node->properties = move(props);
        return Finalize(start_marker, node);
    }

    Sp<SyntaxNode> Parser::ParseAsyncArgument(Scope& scope) {
        auto arg = ParseAssignmentExpression(scope);
        ctx->first_cover_initialized_name_error_.reset();
        return arg;
    }

    std::vector<Sp<SyntaxNode>> Parser::ParseAsyncArguments(Scope& scope) {
        Expect(JsTokenType::LeftParen);
        std::vector<Sp<SyntaxNode>> result;
        if (!Match(JsTokenType::RightParen)) {
            while (true) {
                if (Match(JsTokenType::Spread)) {
                    result.push_back(ParseSpreadElement(scope));
                } else {
                    result.push_back(IsolateCoverGrammar<SyntaxNode>([this, &scope] {
                        return ParseAsyncArgument(scope);
                    }));
                }
                if (Match(JsTokenType::RightParen)) {
                    break;
                }
                ExpectCommaSeparator();
                if (Match(JsTokenType::RightParen)) {
                    break;
                }
            }
        }
        Expect(JsTokenType::RightParen);

        return result;
    }

    Sp<Expression> Parser::ParseGroupExpression(Scope& scope) {
        Sp<Expression> expr;

        Expect(JsTokenType::LeftParen);
        if (Match(JsTokenType::RightParen)) {
            NextToken();
            if (!Match(JsTokenType::Arrow)) {
                Expect(JsTokenType::Arrow);
            }
            auto node = Alloc<ArrowParameterPlaceHolder>();
            node->async = false;
            expr = move(node);
        } else {
            auto start_token = ctx->lookahead_;

            std::vector<Token> params;
            if (Match(JsTokenType::Spread)) {
                expr = ParseRestElement(scope, params);
                Expect(JsTokenType::RightParen);
                if (!Match(JsTokenType::Arrow)) {
                    Expect(JsTokenType::Arrow);
                }
                auto node = Alloc<ArrowParameterPlaceHolder>();
                node->params.push_back(move(expr));
                node->async = false;
                expr = move(node);
            } else {
                bool arrow = false;
                ctx->is_binding_element_ = true;

                expr = InheritCoverGrammar<Expression>([this, &scope] {
                    return ParseAssignmentExpression(scope);
                });

                if (Match(JsTokenType::Comma)) {
                    std::vector<Sp<Expression>> expressions;

                    ctx->is_assignment_target_ = false;
                    expressions.push_back(expr);
                    while (ctx->lookahead_.type != JsTokenType::EOF_) {
                        if (!Match(JsTokenType::Comma)) {
                            break;
                        }
                        NextToken();
                        if (Match(JsTokenType::RightParen)) {
                            NextToken();
                            for (auto& i : expressions) {
                                ReinterpretExpressionAsPattern(i);
                            }
                            arrow = true;
                            auto node = Alloc<ArrowParameterPlaceHolder>();
                            for (auto& i : expressions) {
                                node->params.push_back(i);
                            }
                            node->async = false;
                            expr = move(node);
                        } else if (Match(JsTokenType::Spread)) {
                            if (!ctx->is_binding_element_) {
                                ThrowUnexpectedToken(ctx->lookahead_);
                            }
                            expressions.push_back(ParseRestElement(scope, params));
                            Expect(JsTokenType::RightParen);
                            if (!Match(JsTokenType::Arrow)) {
                                Expect(JsTokenType::Arrow);
                            }
                            ctx->is_binding_element_ = false;
                            for (auto& i : expressions) {
                                ReinterpretExpressionAsPattern(i);
                            }
                            arrow = true;
                            auto node = Alloc<ArrowParameterPlaceHolder>();
                            for (auto& i : expressions) {
                                node->params.push_back(i);
                            }
                            node->async = false;
                            expr = move(node);
                        } else {
                            expressions.push_back(InheritCoverGrammar<Expression>([this, &scope] {
                                return ParseAssignmentExpression(scope);
                            }));
                        }
                        if (arrow) {
                            break;
                        }
                    }

                    if (!arrow) {
                        auto node = Alloc<SequenceExpression>();
                        node->expressions = move(expressions);
                        expr = Finalize(StartNode(start_token), node);
                    }
                }

                if (!arrow) {
                    Expect(JsTokenType::RightParen);
                    if (Match(JsTokenType::Arrow)) {
                        if (expr->type == SyntaxNodeType::Identifier && dynamic_pointer_cast<Identifier>(expr)->name == "yield") {
                            arrow = true;
                            auto node = Alloc<ArrowParameterPlaceHolder>();
                            node->params.push_back(move(expr));
                            node->async = false;
                            expr = move(node);
                        }

                        if (!arrow) {
                            if (!ctx->is_binding_element_) {
                                ThrowUnexpectedToken(ctx->lookahead_);
                            }

                            if (expr->type == SyntaxNodeType::SequenceExpression) {
                                auto node = dynamic_pointer_cast<SequenceExpression>(expr);
                                for (auto& i : node->expressions) {
                                    ReinterpretExpressionAsPattern(i);
                                }
                            } else {
                                ReinterpretExpressionAsPattern(expr);
                            }

                            auto placeholder = Alloc<ArrowParameterPlaceHolder>();

                            if (expr->type == SyntaxNodeType::SequenceExpression) {
                                auto seq = dynamic_pointer_cast<SequenceExpression>(expr);
                                for (auto& i : seq->expressions) {
                                    placeholder->params.push_back(i);
                                }
                            } else {
                                placeholder->params.push_back(expr);
                            }

                            expr = move(placeholder);
                        }
                    }

                    ctx->is_binding_element_ = false;
                }
            }
        }

        return expr;
    }

    Sp<TemplateElement> Parser::ParseTemplateHead() {
        Assert(ctx->lookahead_.head, "Template literal must start with a template head");
        auto start_marker = CreateStartMarker();
        Token token = NextToken();

        auto node = Alloc<TemplateElement>();
        node->raw = token.value;
        node->cooked = token.cooked;
        node->tail = token.tail;

        return Finalize(start_marker, node);
    }

    Sp<TemplateElement> Parser::ParseTemplateElement() {
        if (ctx->lookahead_.type != JsTokenType::Template) {
            ThrowUnexpectedToken(ctx->lookahead_);
        }

        auto start_marker = CreateStartMarker();
        Token token = NextToken();

        auto node = Alloc<TemplateElement>();
        node->raw = token.value;
        node->cooked = token.cooked;
        node->tail = token.tail;

        return Finalize(start_marker, node);
    }

    Sp<TemplateLiteral> Parser::ParseTemplateLiteral(Scope& scope) {
        auto start_marker = CreateStartMarker();
        auto node = Alloc<TemplateLiteral>();

        Sp<TemplateElement> quasi = ParseTemplateHead();
        node->quasis.push_back(quasi);
        while (!quasi->tail) {
            node->expressions.push_back(ParseExpression(scope));
            quasi = ParseTemplateElement();
            node->quasis.push_back(quasi);
        }

        return Finalize(start_marker, node);
    }

    Sp<MethodDefinition> Parser::ParseClassElement(Scope& scope, bool &has_ctor) {
        Token token = ctx->lookahead_;
        auto start_marker = CreateStartMarker();

        optional<Sp<SyntaxNode>> key;
        optional<Sp<Expression>> value;
        bool computed = false;
        bool method = false;
        bool is_static = false;
        bool is_async = false;
        VarKind kind = VarKind::Invalid;

        if (Match(JsTokenType::Mul)) {
            NextToken();
        } else {
            computed = Match(JsTokenType::LeftBrace);
            key = ParseObjectPropertyKey(scope);
            auto id = dynamic_pointer_cast<Identifier>(*key);
            if (id && id->name == "static" && (QualifiedPropertyName(ctx->lookahead_) || Match(JsTokenType::Mul))) {
                token = ctx->lookahead_;
                is_static = true;
                computed = Match(JsTokenType::LeftBrace);
                if (Match(JsTokenType::Mul)) {
                    NextToken();
                } else {
                    key = ParseObjectPropertyKey(scope);
                }
            }
            if (
                    (token.type == JsTokenType::Identifier) &&
                    !ctx->has_line_terminator_ &&
                (token.value == "async")
            ) {
                if (token.type != JsTokenType::Colon && token.type != JsTokenType::LeftParen && token.type != JsTokenType::Mul) {
                    is_async = true;
                    token = ctx->lookahead_;
                    key = ParseObjectPropertyKey(scope);
                    if (token.type == JsTokenType::Identifier && token.value == "constructor") {
                        TolerateUnexpectedToken(token, ParseMessages::ConstructorIsAsync);
                    }
                }
            }
        }

        bool lookahead_prop_key = QualifiedPropertyName(ctx->lookahead_);
        if (token.type == JsTokenType::Identifier) {
            if (token.value == "get" && lookahead_prop_key) {
                kind = VarKind::Get;
                computed = Match(JsTokenType::LeftBrace);
                key = ParseObjectPropertyKey(scope);
                ctx->allow_yield_ = false;
                value = ParseGetterMethod(scope);
            } else if (token.value == "set" && lookahead_prop_key) {
                kind = VarKind::Set;
                computed = Match(JsTokenType::LeftBrace);
                key = ParseObjectPropertyKey(scope);
                value = ParseSetterMethod(scope);
            }
        } else if (token.type == JsTokenType::Mul && lookahead_prop_key) {
            kind = VarKind::Init;
            computed = Match(JsTokenType::LeftBrace);
            key = ParseObjectPropertyKey(scope);
            value = ParseGeneratorMethod(scope);
            method = true;
        }

        if (kind == VarKind::Invalid && key.has_value() && Match(JsTokenType::LeftParen)) {
            kind = VarKind::Init;
            if (is_async) {
                value = ParsePropertyMethodAsyncFunction(scope);
            } else {
                value = ParsePropertyMethodFunction(scope);
            }
            method = true;
        }

        if (kind == VarKind::Invalid) {
            ThrowUnexpectedToken(ctx->lookahead_);
        }

        if (kind == VarKind::Init) {
            kind = VarKind::Method;
        }

        if (!computed) {
            if (is_static && IsPropertyKey(*key, "prototype")) {
                ThrowUnexpectedToken(token, ParseMessages::StaticPrototype);
            }
            if (!is_static && IsPropertyKey(*key, "constructor")) {
//                TODO: generator
//                if (kind != VarKind::Method || !method || (value.has_value() &&))
                if (has_ctor) {
                    ThrowUnexpectedToken(token, ParseMessages::ConstructorSpecialMethod);
                } else {
                    has_ctor = true;
                }
                kind = VarKind::Ctor;
            }
        }

        auto node = Alloc<MethodDefinition>();
        node->key = move(key);
        node->computed = computed;
        node->value = move(value);
        node->kind = kind;
        node->static_ = is_static;
        return Finalize(start_marker, node);
    }

    std::vector<Sp<MethodDefinition>> Parser::ParseClassElementList(Scope& scope) {
        std::vector<Sp<MethodDefinition>> body;
        bool has_ctor = false;

        Expect(JsTokenType::LeftBracket);
        while (!Match(JsTokenType::RightBracket)) {
            if (Match(JsTokenType::Semicolon)) {
                NextToken();
            } else {
                body.push_back(ParseClassElement(scope, has_ctor));
            }
        }
        Expect(JsTokenType::RightBracket);

        return body;
    }

    Sp<FunctionExpression> Parser::ParseGetterMethod(Scope& scope) {
        auto start_marker = CreateStartMarker();
        auto node = Alloc<FunctionExpression>();

        node->generator = false;
        bool prev_allow_yield = ctx->allow_yield_;
        ctx->allow_yield_ = !node->generator;

        auto formal = ParseFormalParameters(scope);
        if (!formal.params.empty()) {
            TolerateError(ParseMessages::BadGetterArity);
        }

        node->params = formal.params;
        node->body = ParsePropertyMethod(scope, formal);
        ctx->allow_yield_ = prev_allow_yield;

        return Finalize(start_marker, node);
    }

    Sp<FunctionExpression> Parser::ParseSetterMethod(Scope& scope) {
        auto start_marker = CreateStartMarker();
        auto node = Alloc<FunctionExpression>();

        node->generator = false;
        bool prev_allow_yield = ctx->allow_yield_;
        ctx->allow_yield_ = !node->generator;
        auto options = ParseFormalParameters(scope);
        if (options.params.size() != 1) {
            TolerateError(ParseMessages::BadSetterArity);
        } else if (options.params[0]->type == SyntaxNodeType::RestElement) {
            TolerateError(ParseMessages::BadSetterRestParameter);
        }
        node->params = options.params;
        node->body = ParsePropertyMethod(scope, options);
        ctx->allow_yield_ = prev_allow_yield;

        return Finalize(start_marker, node);
    }

    Sp<BlockStatement> Parser::ParsePropertyMethod(Scope& scope, parser::ParserCommon::FormalParameterOptions &options) {
        ctx->is_assignment_target_ = false;
        ctx->is_binding_element_ = false;

        bool prev_strict = ctx->strict_;
        bool prev_allow_strict_directive = ctx->allow_strict_directive_;
        ctx->allow_strict_directive_ = options.simple;

        auto body = IsolateCoverGrammar<BlockStatement>([this, &scope] {
            return ParseFunctionSourceElements(scope);
        });
        if (ctx->strict_ && options.first_restricted.has_value()) {
            TolerateUnexpectedToken(*options.first_restricted, options.message);
        }
        if (ctx->strict_ && options.stricted.has_value()) {
            TolerateUnexpectedToken(*options.stricted, options.message);
        }
        ctx->strict_ = prev_strict;
        ctx->allow_strict_directive_ = prev_allow_strict_directive;

        return body;
    }

    Sp<FunctionExpression> Parser::ParseGeneratorMethod(Scope& scope) {
        auto start_marker = CreateStartMarker();
        auto node = Alloc<FunctionExpression>();

        node->generator = true;
        bool prev_allow_yield = ctx->allow_yield_;

        ctx->allow_yield_ = true;
        auto params = ParseFormalParameters(scope);
        ctx->allow_yield_ = false;
        auto method = ParsePropertyMethod(scope, params);
        ctx->allow_yield_ = prev_allow_yield;

        node->params = params.params;
        node->body = move(method);

        return Finalize(start_marker, node);
    }

    Sp<FunctionExpression> Parser::ParsePropertyMethodFunction(Scope& scope) {
        auto start_marker = CreateStartMarker();
        auto node = Alloc<FunctionExpression>();

        node->generator = false;
        bool prev_allow_yield = ctx->allow_yield_;
        ctx->allow_yield_ = true;
        auto params = ParseFormalParameters(scope);
        auto method = ParsePropertyMethod(scope, params);
        ctx->allow_yield_ = prev_allow_yield;

        node->params = params.params;
        node->body = move(method);

        return Finalize(start_marker, node);
    }

    Sp<FunctionExpression> Parser::ParsePropertyMethodAsyncFunction(Scope& scope) {
        auto start_marker = CreateStartMarker();
        auto node = Alloc<FunctionExpression>();

        node->generator = false;
        bool prev_allow_yield = ctx->allow_yield_;
        bool prev_await = ctx->await_;
        ctx->allow_yield_ = false;
        ctx->await_ = true;
        auto params = ParseFormalParameters(scope);
        auto method = ParsePropertyMethod(scope, params);
        ctx->allow_yield_ = prev_allow_yield;
        ctx->await_ = prev_await;

        node->params = params.params;
        node->body = move(method);
        node->async = true;

        return Finalize(start_marker, node);
    }

    bool Parser::QualifiedPropertyName(const Token &token) {
        if (IsKeywordToken(token.type)) return true;

        switch (token.type) {
            case JsTokenType::Identifier:
            case JsTokenType::StringLiteral:
            case JsTokenType::TrueLiteral:
            case JsTokenType::FalseLiteral:
            case JsTokenType::NullLiteral:
            case JsTokenType::NumericLiteral:
            case JsTokenType::LeftBrace:
                return true;

            default:
                break;
        }

        return false;
    }

    bool Parser::IsPropertyKey(const Sp<SyntaxNode> &key, const std::string &name) {
        if (key->type == SyntaxNodeType::Identifier) {
            return dynamic_pointer_cast<Identifier>(key)->name == name;
        }
        if (key->type == SyntaxNodeType::Literal) {
            auto lit = dynamic_pointer_cast<Literal>(key);
            return lit->ty == Literal::Ty::String && lit->str_ == name;
        }
        return false;
    }

}
