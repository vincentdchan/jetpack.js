//
// Created by Duzhong Chen on 2019/9/20.
//

#include "parser.hpp"
#include <jemalloc/jemalloc.h>
#include "../tokenizer/token.h"

namespace parser {

    char* Parser::tc_allocator::malloc(const parser::Parser::tc_allocator::size_type size) {
        return reinterpret_cast<char*>(::malloc(size));
    }

    void Parser::tc_allocator::free(char *const ptr) {
        ::free(ptr);
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
                        ThrowUnexpectedToken(lookahead_);
                    }
                    auto new_right = Alloc<Identifier>();
                    new_right->name = u"yield";
                    node->right = new_right;
                }
            } else if (async_arrow && param->type == SyntaxNodeType::Identifier) {
                auto id = dynamic_pointer_cast<Identifier>(param);
                if (id->name == u"await") {
                    ThrowUnexpectedToken(lookahead_);
                }
            }
            CheckPatternParam(options, param);
        }

        if (context_.strict_ || !context_.allow_yield) {
            for (auto& param : params) {
                if (param->type == SyntaxNodeType::YieldExpression) {
                    ThrowUnexpectedToken(lookahead_);
                }
            }
        }

        if (options.message == ParseMessages::StrictParamDupe) {
            Token token = context_.strict_ ? *options.stricted : *options.first_restricted;
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
        auto state = scanner_->SaveState();
        std::vector<Comment> comments;
        scanner_->ScanComments(comments);
        Token next = scanner_->Lex();
        scanner_->RestoreState(state);

        return (next.type_ == JsTokenType::Identifier) ||
               (next.type_ == JsTokenType::LeftBrace) ||
               (next.type_ == JsTokenType::LeftBracket) ||
               next.type_ == JsTokenType::K_Let ||
               next.type_ == JsTokenType::K_Yield;
    }

    bool Parser::MatchImportCall() {
        bool match = Match(JsTokenType::K_Import);
        if (match) {
            auto state = scanner_->SaveState();
            std::vector<Comment> comments;
            Token next = scanner_->Lex();
            scanner_->ScanComments(comments);
            scanner_->RestoreState(state);
            match = (next.type_ == JsTokenType::LeftParen);
        }
        return match;
    }

    bool Parser::IsStartOfExpression() {
        UString value = lookahead_.value_;

        if (IsPunctuatorToken(lookahead_.type_)) {
            switch (lookahead_.type_) {
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

        if (IsKeywordToken(lookahead_.type_)) {
            switch (lookahead_.type_) {
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

    ParserCommon::FormalParameterOptions Parser::ParseFormalParameters(optional<Token> first_restricted) {

        FormalParameterOptions options;
        options.simple = true;
        options.first_restricted = move(first_restricted);

        Expect(JsTokenType::LeftParen);
        if (!Match(JsTokenType::RightParen)) {
            while (lookahead_.type_ != JsTokenType::EOF_) {
                ParseFormalParameter(options);
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

    void Parser::ParseFormalParameter(parser::Parser::FormalParameterOptions &option) {
        std::vector<Token> params;
        Sp<SyntaxNode> param;
        if (Match(JsTokenType::Spread)) {
            param = ParseRestElement(params);
        } else {
            param = ParsePattern(params, VarKind::Invalid);
        }

        for (auto& i : params) {
            ValidateParam(option, i, i.value_);
        }

        option.simple &= param->type == SyntaxNodeType::Identifier;
        option.params.push_back(move(param));
    }

    void Parser::ValidateParam(parser::ParserCommon::FormalParameterOptions &options, const Token &param,
                               const UString &name) {
        UString key = UString(u"$") + name;
        if (context_.strict_) {
            if (scanner_->IsRestrictedWord(name)) {
                options.stricted = param;
                options.message = ParseMessages::StrictParamName;
            }
            if (options.param_set.find(key) != options.param_set.end()) {
                options.stricted = param;
                options.message = ParseMessages::StrictParamDupe;
            }
        } else if (!options.first_restricted) {
            if (scanner_->IsRestrictedWord(name)) {
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

    Sp<Expression> Parser::ParsePrimaryExpression() {
        auto marker = CreateStartMarker();
        Token token;

        if (IsKeywordToken(lookahead_.type_)) {
            if (!context_.strict_ && context_.allow_yield && Match(JsTokenType::K_Yield)) {
                return ParseIdentifierName();
            } else if (!context_.strict_ && Match(JsTokenType::K_Let)) {
                token = NextToken();
                auto id = Alloc<Identifier>();
                id->name = token.value_;
                return Finalize(marker, id);
            } else {
                context_.is_assignment_target = false;
                context_.is_binding_element = false;
                if (Match(JsTokenType::K_Function)) {
                    return ParseFunctionExpression();
                } else if (Match(JsTokenType::K_This)) {
                    token = NextToken();
                    auto th = Alloc<ThisExpression>();
                    return Finalize(marker, th);
                } else if (Match(JsTokenType::K_Class)) {
                    return ParseClassExpression();
                } else if (MatchImportCall()) {
                    return ParseImportCall();
                } else {
                    ThrowUnexpectedToken(NextToken());
                    return nullptr;
                }
            }
        }

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

            case JsTokenType::LeftParen:
                context_.is_binding_element = false;
                return InheritCoverGrammar<Expression>([this]() {
                    return ParseGroupExpression();
                });

            case JsTokenType::LeftBrace:
                return InheritCoverGrammar<Expression>([this]() {
                    return ParseArrayInitializer();
                });

            case JsTokenType::LeftBracket:
                return InheritCoverGrammar<Expression>([this]() {
                    return ParseObjectInitializer();
                });

            case JsTokenType::Div:
            case JsTokenType::DivAssign: {
                context_.is_assignment_target = false;
                context_.is_binding_element = false;
                scanner_->SetIndex(StartMarker().index);
                token = NextRegexToken();
                auto node = Alloc<RegexLiteral>();
                node->value = token.value_;
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

    Sp<SpreadElement> Parser::ParseSpreadElement() {
        auto marker = CreateStartMarker();

        Expect(JsTokenType::Spread);

        auto node = Alloc<SpreadElement>();
        node->argument = InheritCoverGrammar<Expression>([this] {
            return ParseAssignmentExpression();
        });

        return Finalize(marker, node);
    }

    Sp<Property> Parser::ParseObjectProperty(bool &has_proto) {
        auto marker = CreateStartMarker();
        Token token = lookahead_;
        VarKind kind = VarKind::Invalid;
        bool computed = false;
        bool method = false;
        bool shorthand = false;
        bool is_async = false;

        optional<Sp<SyntaxNode>> key;
        optional<Sp<SyntaxNode>> value;

        if (token.type_ == JsTokenType::Identifier) {
            auto id = token.value_;
            NextToken();
            computed = Match(JsTokenType::LeftBrace);
            is_async = !has_line_terminator_ && (id == u"async") &&
                       !Match(JsTokenType::Colon) && !Match(JsTokenType::LeftParen) && !Match(JsTokenType::Mul) && !Match(JsTokenType::Comma);
            if (is_async) {
                key = ParseObjectPropertyKey();
            } else {
                auto node = Alloc<Identifier>();
                node->name = id;
                key = Finalize(marker, node);
            }
        } else if (Match(JsTokenType::Mul)) {
            NextToken();
        } else {
            computed = Match(JsTokenType::LeftBrace);
            key = ParseObjectPropertyKey();
        }

        auto lookahead_prop_key = QualifiedPropertyName(lookahead_);

        if (token.type_ == JsTokenType::Identifier && !is_async && token.value_ == u"get" && lookahead_prop_key) {
            kind = VarKind::Get;
            computed = Match(JsTokenType::LeftBrace);
            key = ParseObjectPropertyKey();
            context_.allow_yield = false;
            value = ParseGetterMethod();
        } else if (token.type_ == JsTokenType::Identifier && !is_async && token.value_ == u"set" && lookahead_prop_key) {
            kind = VarKind::Set;
            computed = Match(JsTokenType::LeftBrace);
            key = ParseObjectPropertyKey();
            context_.allow_yield = false;
            value = ParseGetterMethod();
        } else if (token.type_ == JsTokenType::Mul && !is_async && lookahead_prop_key) {
            kind = VarKind::Init;
            computed = Match(JsTokenType::LeftBrace);
            key = ParseObjectPropertyKey();
            value = ParseGeneratorMethod();
            method = true;
        } else {
            if (!key.has_value()) {
                ThrowUnexpectedToken(lookahead_);
            }

            kind = VarKind::Init;
            if (Match(JsTokenType::Colon) && !is_async) {
                if (!computed && IsPropertyKey(*key, u"__proto__")) {
                    if (has_proto) {
                        TolerateError(ParseMessages::DuplicateProtoProperty);
                    }
                    has_proto = true;
                }
                NextToken();
                value = InheritCoverGrammar<Expression>([this] {
                    return ParseAssignmentExpression();
                });
            } else if (Match(JsTokenType::LeftParen)) {
                if (is_async) {
                    value = ParsePropertyMethodAsyncFunction();
                } else {
                    value = ParsePropertyMethodFunction();
                }
                method = true;
            } else if (token.type_ == JsTokenType::Identifier) {
                auto id = Alloc<Identifier>();
                id->name = token.value_;
                Finalize(marker, id);
                if (Match(JsTokenType::Assign)) {
                    context_.first_cover_initialized_name_error = lookahead_;
                    NextToken();
                    shorthand = true;
                    auto node = Alloc<AssignmentPattern>();
                    node->left = move(id);
                    node->right = IsolateCoverGrammar<Expression>([this] {
                        return ParseAssignmentExpression();
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

    Sp<SyntaxNode> Parser::ParseObjectPropertyKey() {
        auto marker = CreateStartMarker();
        Token token = NextToken();

        if (IsKeywordToken(token.type_)) {
            auto node = Alloc<Identifier>();
            node->name = token.value_;
            return Finalize(marker, node);
        }

        if (IsPunctuatorToken(token.type_)) {
            if (token.type_ == JsTokenType::LeftBrace) {
                auto result = IsolateCoverGrammar<Expression>([this] {
                    return ParseAssignmentExpression();
                });
                Expect(JsTokenType::RightBrace);
                return result;
            } else {
                ThrowUnexpectedToken(token);
            }
        }

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
            case JsTokenType::NullLiteral: {
                auto node = Alloc<Identifier>();
                node->name = token.value_;
                return Finalize(marker, node);
            }

            default:
                ThrowUnexpectedToken(token);
                return nullptr;
        }

        return nullptr;
    }

    Sp<Expression> Parser::ParseObjectInitializer() {
        auto marker = CreateStartMarker();

        Expect(JsTokenType::LeftBracket);
        auto node = Alloc<ObjectExpression>();
        bool has_proto = false;
        while (!Match(JsTokenType::RightBracket)) {
            Sp<SyntaxNode> prop;
            if (Match(JsTokenType::Spread)) {
                prop = ParseSpreadElement();
            } else {
                prop = ParseObjectProperty(has_proto);
            }
            node->properties.push_back(std::move(prop));
            if (!Match(JsTokenType::RightBracket)) {
                ExpectCommaSeparator();
            }
        }
        Expect(JsTokenType::RightBracket);

        return Finalize(marker, node);
    }

    Sp<FunctionExpression> Parser::ParseFunctionExpression() {
        auto marker = CreateStartMarker();

        bool is_async = MatchContextualKeyword(u"async");
        if (is_async) NextToken();

        Expect(JsTokenType::K_Function);

        bool is_generator = is_async ? false : Match(JsTokenType::Mul);
        if (is_generator) NextToken();

        optional<Sp<Identifier>> id;
        optional<Token> first_restricted;

        bool prev_allow_await = context_.await;
        bool prev_allow_yield = context_.allow_yield;
        context_.await = is_async;
        context_.allow_yield = !is_generator;

        std::string message;

        if (!Match(JsTokenType::LeftParen)) {
            Token token = lookahead_;

            if (!context_.strict_ && !is_generator && Match(JsTokenType::K_Yield)) {
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
                    // TODO: message
//                } else if (scanner_->IsStrictModeReservedWord(token.value_)) {
//                    first_restricted = token;
//                    message = ParseMessages::StrictReservedWord;
                }
            }
        }

        auto formal = ParseFormalParameters(first_restricted);
        if (!formal.message.empty()) {
            message = formal.message;
        }

        bool prev_strict = context_.strict_;
        bool prev_allow_strict_directive = context_.allow_strict_directive;
        context_.allow_strict_directive = formal.simple;
        auto body = ParseFunctionSourceElements();
        if (context_.strict_ && first_restricted.has_value()) {
            ThrowUnexpectedToken(*first_restricted, message);
        }
        if (context_.strict_ && formal.stricted.has_value()) {
            TolerateUnexpectedToken(*formal.stricted, message);
        }
        context_.strict_ = prev_strict;
        context_.allow_strict_directive = prev_allow_strict_directive;
        context_.await = prev_allow_await;
        context_.allow_yield = prev_allow_yield;

        if (is_async) {
            auto node = Alloc<FunctionExpression>();
            node->id = id;
            node->params = move(formal.params);
            node->body = move(body);
            node->async = true;
            return Finalize(marker, node);
        } else {
            auto node = Alloc<FunctionExpression>();
            node->id = id;
            node->params = move(formal.params);
            node->body = move(body);
            node->generator = is_generator;
            node->async = false;
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
        node->name = token.value_;
        return Finalize(marker, node);
    }

    Sp<Expression> Parser::ParseNewExpression() {
        auto start_marker = CreateStartMarker();

        Sp<Identifier> id = ParseIdentifierName();
        Assert(id->name == u"new", "New expression must start with `new`");

        Sp<Expression> expr;

        if (Match(JsTokenType::Dot)) {
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
        } else if (Match(JsTokenType::K_Import)) {
            ThrowUnexpectedToken(lookahead_);
            return nullptr;
        } else {
            auto node = Alloc<NewExpression>();
            node->callee = IsolateCoverGrammar<Expression>([this] {
                return ParseLeftHandSideExpression();
            });
            if (Match(JsTokenType::LeftParen)) {
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
        Expect(JsTokenType::K_Yield);

        auto node = Alloc<YieldExpression>();
        if (has_line_terminator_) {
            bool prev_allow_yield = context_.allow_yield;
            context_.allow_yield = false;
            node->delegate = Match(JsTokenType::Mul);
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
        Expect(JsTokenType::LeftParen);
        if (!Match(JsTokenType::RightParen)) {
            Sp<SyntaxNode> expr;
            while (true) {
                if (Match(JsTokenType::Spread)) {
                    expr = ParseSpreadElement();
                } else {
                    expr = IsolateCoverGrammar<Expression>([this] {
                        return ParseAssignmentExpression();
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

        if (Match(JsTokenType::Comma)) {
            std::vector<Sp<Expression>> expressions;
            expressions.push_back(expr);
            while (lookahead_.type_ != JsTokenType::EOF_) {
                if (!Match(JsTokenType::Comma)) {
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
        Expect(JsTokenType::LeftBracket);

        auto prev_label_set = move(context_.label_set);
        bool prev_in_iteration = context_.in_iteration;
        bool prev_in_switch = context_.in_switch;
        bool prev_in_fun_body = context_.in_function_body;

        context_.label_set = make_unique<unordered_set<UString>>();
        context_.in_iteration = false;
        context_.in_switch = false;
        context_.in_function_body = true;

        while (lookahead_.type_ != JsTokenType::EOF_) {
            if (Match(JsTokenType::RightBracket)) {
                break;
            }

            Sp<SyntaxNode> temp = ParseStatementListItem();
            body.push_back(move(temp));
        }

        Expect(JsTokenType::RightBracket);

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
        Expect(JsTokenType::K_Class);

        auto node = Alloc<ClassDeclaration>();
        if (identifier_is_optional && (lookahead_.type_ != JsTokenType::Identifier)) {
            // nothing
        } else {
            node->id = ParseVariableIdentifier(VarKind::Invalid);
        }

        if (Match(JsTokenType::K_Extends)) {
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
        Expect(JsTokenType::K_Class);

        if (lookahead_.type_ == JsTokenType::Identifier) {
            node->id = ParseVariableIdentifier(VarKind::Invalid);
        }

        if (Match(JsTokenType::K_Extends)) {
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
        if (Match(JsTokenType::K_Super) && context_.in_function_body) {
            auto node = Alloc<Super>();
            auto marker = CreateStartMarker();
            NextToken();
            expr = Finalize(marker, node);
            if (!Match(JsTokenType::LeftParen) && !Match(JsTokenType::Dot) && !Match(JsTokenType::LeftBrace)) {
                ThrowUnexpectedToken(lookahead_);
                return nullptr;
            }
        } else {
            if (Match(JsTokenType::K_New)) {
                expr = InheritCoverGrammar<Expression>([this] {
                    return ParseNewExpression();
                });
            } else {
                expr = InheritCoverGrammar<Expression>([this] {
                    return ParsePrimaryExpression();
                });
            }
        }
        Assert(!!expr, "ParseLeftHandSideExpressionAllowCall: expr should not be nullptr");

        while (true) {
            if (Match(JsTokenType::Dot)) {
                context_.is_binding_element = false;
                context_.is_assignment_target = true;
                Expect(JsTokenType::Dot);
                auto node = Alloc<MemberExpression>();
                node->property = ParseIdentifierName();
                node->object = expr;
                node->computed = false;
                expr = Finalize(StartNode(start_token), node);
            } else if (Match(JsTokenType::LeftParen)) {
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
                context_.is_binding_element = false;
                context_.is_assignment_target = true;
                Expect(JsTokenType::LeftBrace);
                auto node = Alloc<MemberExpression>();
                node->computed = true;
                node->property = IsolateCoverGrammar<Expression>([this] {
                    return ParseExpression();
                });
                Expect(JsTokenType::RightBrace);
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

        Expect(JsTokenType::LeftBrace);
        while (!Match(JsTokenType::RightBrace)) {
            if (Match(JsTokenType::Comma)) {
                NextToken();
                node->elements.emplace_back(nullopt);
            } else if (Match(JsTokenType::Spread)) {
                element = ParseSpreadElement();
                if (!Match(JsTokenType::RightBrace)) {
                    context_.is_assignment_target = false;
                    context_.is_binding_element = false;
                    Expect(JsTokenType::Comma);
                }
                node->elements.emplace_back(element);
            } else {
                element = InheritCoverGrammar<SyntaxNode>([this] {
                    return ParseAssignmentExpression();
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
        context_.strict_ = true;
        context_.is_module = true;
        auto marker = CreateStartMarker();
        auto node = make_shared<Module>();
        node->body = ParseDirectivePrologues();
        while (lookahead_.type_ != JsTokenType::EOF_) {
            node->body.push_back(ParseStatementListItem());
        }
        return Finalize(marker, node);
    }

    Sp<Script> Parser::ParseScript() {
        auto start_marker = CreateStartMarker();
        auto node = Alloc<Script>();
        node->body = ParseDirectivePrologues();
        while (lookahead_.type_ != JsTokenType::EOF_) {
            node->body.push_back(ParseStatementListItem());
        }
        return Finalize(start_marker, node);
    }

    Sp<SwitchCase> Parser::ParseSwitchCase() {

        auto marker = CreateStartMarker();
        auto node = Alloc<SwitchCase>();

        if (Match(JsTokenType::K_Default)) {
            NextToken();
            node->test.reset();
        } else {
            Expect(JsTokenType::K_Case);
            node->test = ParseExpression();
        }
        Expect(JsTokenType::Colon);

        while (true) {
            if (Match(JsTokenType::RightBracket) || Match(JsTokenType::K_Default) || Match(JsTokenType::K_Case)) {
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

        Expect(JsTokenType::K_If);
        Expect(JsTokenType::LeftParen);
        node->test = ParseExpression();

        if (!Match(JsTokenType::RightParen) && config_.tolerant) {
            Token token = NextToken();
            ThrowUnexpectedToken(token);
            node->consequent = Finalize(CreateStartMarker(), Alloc<EmptyStatement>());
        } else {
            Expect(JsTokenType::RightParen);
            node->consequent = ParseIfClause();
            if (Match(JsTokenType::K_Else)) {
                NextToken();
                node->alternate = ParseIfClause();
            }
        }

        return Finalize(marker, node);
    }

    Sp<Statement> Parser::ParseIfClause() {
        if (context_.strict_ && Match(JsTokenType::K_Function)) {
            TolerateError(ParseMessages::StrictFunction);
        }
        return ParseStatement();
    }

    Sp<Statement> Parser::ParseStatement() {

        if (IsPunctuatorToken(lookahead_.type_)) {
            switch (lookahead_.type_) {
                case JsTokenType::LeftBracket:
                    return ParseBlock();

                case JsTokenType::LeftParen:
                    return ParseExpressionStatement();

                case JsTokenType::Semicolon:
                    return ParseEmptyStatement();

                default:
                    return ParseExpressionStatement();

            }
        }

        switch (lookahead_.type_) {
            case JsTokenType::BooleanLiteral:
            case JsTokenType::NullLiteral:
            case JsTokenType::NumericLiteral:
            case JsTokenType::StringLiteral:
            case JsTokenType::Template:
            case JsTokenType::RegularExpression:
                return ParseExpressionStatement();

            case JsTokenType::Identifier: {
                if (MatchAsyncFunction()) {
                    return ParseFunctionDeclaration(false);
                } else {
                    return ParseLabelledStatement();
                }
            }

            case JsTokenType::K_Break:
                return ParseBreakStatement();

            case JsTokenType::K_Continue:
                return ParseContinueStatement();

            case JsTokenType::K_Debugger:
                return ParseDebuggerStatement();

            case JsTokenType::K_Do:
                return ParseDoWhileStatement();

            case JsTokenType::K_For:
                return ParseForStatement();

            case JsTokenType::K_Function:
                return ParseFunctionDeclaration(false);

            case JsTokenType::K_If:
                return ParseIfStatement();

            case JsTokenType::K_Return:
                return ParseReturnStatement();

            case JsTokenType::K_Switch:
                return ParseSwitchStatement();

            case JsTokenType::K_Throw:
                return ParseThrowStatement();

            case JsTokenType::K_Try:
                return ParseTryStatement();

            case JsTokenType::K_Var:
                return ParseVariableStatement();

            case JsTokenType::K_While:
                return ParseWhileStatement();

            case JsTokenType::K_With:
                return ParseWithStatement();

            default:
                if (IsKeywordToken(lookahead_.type_)) {
                    return ParseExpressionStatement();
                }
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

        Expect(JsTokenType::LeftBracket);
        while (true) {
            if (Match(JsTokenType::RightBracket)) {
                break;
            }
            Sp<SyntaxNode> stmt = ParseStatementListItem();
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

    Sp<FunctionDeclaration> Parser::ParseFunctionDeclaration(bool identifier_is_optional) {
        auto marker = CreateStartMarker();

        bool is_async = MatchContextualKeyword(u"async");
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

        if (!identifier_is_optional || !Match(JsTokenType::LeftParen)) {
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

        auto options = ParseFormalParameters(first_restricted);
        first_restricted = options.first_restricted;
        if (!options.message.empty()) {
            message = move(options.message);
        }

        bool prev_strict = context_.strict_;
        bool prev_allow_strict_directive = context_.allow_strict_directive;
        context_.allow_strict_directive = options.simple;
        auto body = ParseFunctionSourceElements();

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
            auto node = Alloc<FunctionDeclaration>();
            node->id = id;
            node->params = move(options.params);
            node->body = move(body);
            node->async = true;
            return Finalize(marker, node);
        } else {
            auto node = Alloc<FunctionDeclaration>();
            node->id = id;
            node->generator = is_generator;
            node->params = move(options.params);
            node->body = move(body);
            node->async = false;

            return Finalize(marker, node);
        }
    }

    Sp<Statement> Parser::ParseLabelledStatement() {
        auto start_marker = CreateStartMarker();
        Sp<Expression> expr = ParseExpression();

        Sp<Statement> statement;
        if ((expr->type == SyntaxNodeType::Identifier) && Match(JsTokenType::Colon)) {
            NextToken();

            auto id = dynamic_pointer_cast<Identifier>(expr);
            UString key = UString(u"$") + id->name;

            if (context_.label_set->find(key) != context_.label_set->end()) {
                ThrowError(ParseMessages::Redeclaration, string("Label: ") + utils::To_UTF8(id->name));
            }
            context_.label_set->insert(key);

            Sp<Statement> body;
            if (Match(JsTokenType::K_Class)) {
                TolerateUnexpectedToken(lookahead_);
                body = ParseClassDeclaration(false);
            } else if (Match(JsTokenType::K_Function)) {
                Token token = lookahead_;
                auto declaration = ParseFunctionDeclaration(false);
                if (context_.strict_) {
                    TolerateUnexpectedToken(token, ParseMessages::StrictFunction);
                } else if (declaration->generator) {
                    TolerateUnexpectedToken(token, ParseMessages::GeneratorInLegacyContext);
                }
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
        Expect(JsTokenType::K_Break);

        std::optional<Sp<Identifier>> label;
        if (lookahead_.type_ == JsTokenType::Identifier && !has_line_terminator_) {
            Sp<Identifier> id = ParseVariableIdentifier(VarKind::Invalid);

            UString key = UString(u"$") + id->name;
            if (context_.label_set->find(key) == context_.label_set->end()) {
                ThrowError(ParseMessages::UnknownLabel, utils::To_UTF8(id->name));
            }
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
        Expect(JsTokenType::K_Continue);
        auto node = Alloc<ContinueStatement>();

        if (lookahead_.type_ == JsTokenType::Identifier && !has_line_terminator_) {
            auto id = ParseVariableIdentifier(VarKind::Invalid);
            node->label = id;

            UString key = UString(u"$") + id->name;
            if (context_.label_set->find(key) == context_.label_set->end()) {
                ThrowError(ParseMessages::UnknownLabel, utils::To_UTF8(id->name));
            }
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
        Expect(JsTokenType::K_Debugger);
        ConsumeSemicolon();
        auto node = Alloc<DebuggerStatement>();
        return Finalize(marker, node);
    }

    Sp<DoWhileStatement> Parser::ParseDoWhileStatement() {
        auto marker = CreateStartMarker();
        auto node = Alloc<DoWhileStatement>();
        Expect(JsTokenType::K_Do);

        auto previous_in_interation = context_.in_iteration;
        context_.in_iteration = true;
        node->body = ParseStatement();
        context_.in_iteration = previous_in_interation;

        Expect(JsTokenType::K_While);
        Expect(JsTokenType::LeftParen);
        node->test = ParseExpression();

        if (!Match(JsTokenType::RightParen) && config_.tolerant) {
            ThrowUnexpectedToken(NextToken());
        } else {
            Expect(JsTokenType::RightParen);
            if (Match(JsTokenType::Semicolon)) {
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

        Expect(JsTokenType::K_For);
        Expect(JsTokenType::LeftParen);

        if (Match(JsTokenType::Semicolon)) {
            NextToken();
        } else {
            if (Match(JsTokenType::K_Var)) {
                auto marker = CreateStartMarker();
                NextToken();

                auto prev_allow_in = context_.allow_in;
                context_.allow_in = true;
                std::vector<Sp<VariableDeclarator>> declarations = ParseVariableDeclarationList(for_in);
                context_.allow_in = prev_allow_in;

                if (declarations.size() == 1 && Match(JsTokenType::K_In)) {
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
                } else if (declarations.size() == 1 && !declarations[0]->init && Match(JsTokenType::Invalid)) { // of
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
                    Expect(JsTokenType::Semicolon);
                }
            } else if (Match(JsTokenType::K_Const) || Match(JsTokenType::K_Let)) {
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
                    bool in_for = true;
                    std::vector<Sp<VariableDeclarator>> declarations = ParseBindingList(VarKind::Const, in_for);
                    context_.allow_in = prev_allow_in;

                    if (declarations.size() == 1 && !declarations[0]->init && Match(JsTokenType::K_In)) {
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

                if (Match(JsTokenType::K_In)) {
                    if (!context_.is_assignment_target || (*init)->type == SyntaxNodeType::AssignmentExpression) {
                        TolerateError(ParseMessages::InvalidLHSInForIn);
                    }

                    NextToken();
                    init = ReinterpretExpressionAsPattern(*init);
                    left = *init;
                    right = ParseExpression();
                    init.reset();
                } else if (MatchContextualKeyword(u"of")) {
                    if (!context_.is_assignment_target || (*init)->type == SyntaxNodeType::AssignmentExpression) {
                        TolerateError(ParseMessages::InvalidLHSInForIn);
                    }

                    NextToken();
                    init = ReinterpretExpressionAsPattern(*init);
                    left = *init;
                    right = ParseAssignmentExpression();
                    init.reset();
                    for_in = false;
                } else {
                    if (Match(JsTokenType::Comma)) {
                        std::vector<Sp<Expression>> init_seq;
                        Assert(init.has_value() && (*init)->IsExpression(), "init should be an expression");
                        init_seq.push_back(dynamic_pointer_cast<Expression>(*init));

                        while (Match(JsTokenType::Comma)) {
                            NextToken();
                            Sp<Expression> node = IsolateCoverGrammar<Expression>([this] {
                                return ParseAssignmentExpression();
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
                test = ParseExpression();
            }
            Expect(JsTokenType::Semicolon);
            if (!Match(JsTokenType::RightParen)) {
                update = ParseExpression();
            }
        }

        Sp<Statement> body;
        if (!Match(JsTokenType::RightParen) && config_.tolerant) {
            TolerateUnexpectedToken(NextToken());
            auto node = Alloc<EmptyStatement>();
            body = Finalize(CreateStartMarker(), node);
        } else {
            Expect(JsTokenType::RightParen);

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
        Expect(JsTokenType::K_Return);

        bool has_arg = (!Match(JsTokenType::Semicolon) && !Match(JsTokenType::RightBracket) &&
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
        Expect(JsTokenType::K_Switch);
        auto node = Alloc<SwitchStatement>();

        Expect(JsTokenType::LeftParen);
        node->discrimiant = ParseExpression();
        Expect(JsTokenType::RightParen);

        auto prev_in_switch = context_.in_switch;
        context_.in_switch = true;

        bool default_found = false;
        Expect(JsTokenType::LeftBracket);
        while (true) {
            if (Match(JsTokenType::RightBracket)) {
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
        Expect(JsTokenType::RightBracket);

        context_.in_switch = prev_in_switch;

        return Finalize(marker, node);
    }

    Sp<ThrowStatement> Parser::ParseThrowStatement() {
        auto marker = CreateStartMarker();
        Expect(JsTokenType::K_Throw);

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
        Expect(JsTokenType::K_Try);
        auto node = Alloc<TryStatement>();

        node->block = ParseBlock();
        if (Match(JsTokenType::K_Catch)) {
            node->handler = ParseCatchClause();
        }
        if (Match(JsTokenType::K_Finally)) {
            node->finalizer = ParseFinallyClause();
        }

        return Finalize(marker, node);
    }

    Sp<CatchClause> Parser::ParseCatchClause() {
        auto marker = CreateStartMarker();

        Expect(JsTokenType::K_Catch);

        Expect(JsTokenType::LeftParen);
        if (Match(JsTokenType::RightParen)) {
            ThrowUnexpectedToken(lookahead_);
            return nullptr;
        }

        auto node = Alloc<CatchClause>();

        std::vector<Token> params;
        node->param = ParsePattern(params);

        std::unordered_set<UString> param_set;
        for (auto& token : params) {
            UString key = UString(u"$") + token.value_;
            if (param_set.find(key) != param_set.end()) {
                TolerateError(string(ParseMessages::DuplicateBinding) + ": " + utils::To_UTF8(token.value_));
            }
            param_set.insert(key);
        }

        if (context_.strict_ && node->param->type == SyntaxNodeType::Identifier) {
            auto id = dynamic_pointer_cast<Identifier>(node->param);
            if (scanner_->IsRestrictedWord(id->name)) {
                TolerateError(ParseMessages::StrictCatchVariable);
            }
        }

        Expect(JsTokenType::RightParen);
        node->body = ParseBlock();

        return Finalize(marker, node);
    }

    Sp<BlockStatement> Parser::ParseFinallyClause() {
        Expect(JsTokenType::K_Finally);
        return ParseBlock();
    }

    Sp<VariableDeclaration> Parser::ParseVariableStatement() {
        auto marker = CreateStartMarker();
        Expect(JsTokenType::K_Var);

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
        if (Match(JsTokenType::Assign)) {
            NextToken();
            init = IsolateCoverGrammar<Expression>([this] {
                return ParseAssignmentExpression();
            });
        } else if (id->type != SyntaxNodeType::Identifier && !in_for) {
            Expect(JsTokenType::Assign);
        }

        node->id = move(id);
        node->init = init;

        return Finalize(marker, node);
    }

    std::vector<Sp<VariableDeclarator>> Parser::ParseVariableDeclarationList(bool in_for) {
        std::vector<Sp<VariableDeclarator>> list;
        list.push_back(ParseVariableDeclaration(in_for));
        while (Match(JsTokenType::Comma)) {
            NextToken();
            list.push_back(ParseVariableDeclaration(in_for));
        }

        return list;
    }

    Sp<WhileStatement> Parser::ParseWhileStatement() {
        auto marker = CreateStartMarker();
        auto node = Alloc<WhileStatement>();

        Expect(JsTokenType::K_While);
        Expect(JsTokenType::LeftParen);
        node->test = ParseExpression();

        if (!Match(JsTokenType::RightParen) && config_.tolerant) {
            TolerateUnexpectedToken(NextToken());
            node->body = Finalize(CreateStartMarker(), Alloc<EmptyStatement>());
        } else {
            Expect(JsTokenType::RightParen);

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

        Expect(JsTokenType::K_With);
        Expect(JsTokenType::LeftParen);

        node->object = ParseExpression();

        if (!Match(JsTokenType::RightParen) && config_.tolerant) {
            TolerateUnexpectedToken(NextToken());
            auto empty = Alloc<EmptyStatement>();
            node->body = Finalize(marker, empty);
        } else {
            Expect(JsTokenType::RightParen);
            node->body = ParseStatement();
        }

        return Finalize(marker, node);
    }

    Sp<VariableDeclarator> Parser::ParseLexicalBinding(VarKind kind, bool &in_for) {
        auto start_marker = CreateStartMarker();
        auto node = Alloc<VariableDeclarator>();
        std::vector<Token> params;
        node->id = ParsePattern(params, kind);

        if (context_.strict_ && node->id->type == SyntaxNodeType::Identifier) {
            auto id = dynamic_pointer_cast<Identifier>(node->id);
            if (scanner_->IsRestrictedWord(id->name)) {
                TolerateError(ParseMessages::StrictVarName);
            }
        }

        if (kind == VarKind::Const) {
            if (!Match(JsTokenType::K_In) && !MatchContextualKeyword(u"of")) {
                if (Match(JsTokenType::Assign)) {
                    NextToken();
                    node->init = IsolateCoverGrammar<Expression>([this] {
                        return ParseAssignmentExpression();
                    });
                } else {
                    ThrowError(ParseMessages::DeclarationMissingInitializer);
                }
            }
        } else if ((!in_for && node->id->type != SyntaxNodeType::Identifier) || Match(JsTokenType::Assign)) {
            Expect(JsTokenType::Assign);
            node->init = IsolateCoverGrammar<Expression>([this] {
                return ParseAssignmentExpression();
            });
        }

        return Finalize(start_marker, node);
    }

    std::vector<Sp<VariableDeclarator>> Parser::ParseBindingList(VarKind kind, bool& in_for) {
        std::vector<Sp<VariableDeclarator>> list;
        list.push_back(ParseLexicalBinding(kind, in_for));

        while (Match(JsTokenType::Comma)) {
            NextToken();
            list.push_back(ParseLexicalBinding(kind, in_for));
        }

        return list;
    }

    Sp<RestElement> Parser::ParseRestElement(std::vector<Token> &params) {
        auto marker = CreateStartMarker();
        auto node = Alloc<RestElement>();

        Expect(JsTokenType::Spread);
        node->argument = ParsePattern(params, VarKind::Invalid);
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

    Sp<Statement> Parser::ParseStatementListItem() {
        Sp<Statement> statement;
        context_.is_assignment_target = true;
        context_.is_binding_element = true;

        if (IsKeywordToken(lookahead_.type_)) {
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
        Expect(JsTokenType::K_Export);

        Sp<Declaration> export_decl;
        if (Match(JsTokenType::K_Default)) {
            NextToken();
            if (Match(JsTokenType::K_Function)) {
                auto node = Alloc<ExportDefaultDeclaration>();
                node->declaration = ParseFunctionDeclaration(true);
                export_decl = Finalize(start_marker, node);
            } else if (Match(JsTokenType::K_Class)) {
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
                if (Match(JsTokenType::LeftBracket)) {
                    decl = ParseObjectInitializer();
                } else if (Match(JsTokenType::LeftBrace)) {
                    decl = ParseArrayInitializer();
                } else {
                    decl = ParseAssignmentExpression();
                }
                ConsumeSemicolon();
                auto node = Alloc<ExportDefaultDeclaration>();
                node->declaration = move(decl);
                export_decl = Finalize(start_marker, node);
            }

        } else if (Match(JsTokenType::Mul)) {
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
        } else if (IsKeywordToken(lookahead_.type_)) {
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

            Expect(JsTokenType::LeftBracket);
            while (!Match(JsTokenType::RightBracket)) {
                is_export_from_id = is_export_from_id || Match(JsTokenType::K_Default);
                node->specifiers.push_back(ParseExportSpecifier());
                if (!Match(JsTokenType::RightBracket)) {
                    Expect(JsTokenType::Comma);
                }
            }
            Expect(JsTokenType::RightBracket);

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

        if (context_.allow_yield && Match(JsTokenType::K_Yield)) {
            expr = ParseYieldExpression();
        } else {
            auto start_marker = CreateStartMarker();
            auto token = lookahead_;
            expr = ParseConditionalExpression();

            if (token.type_ == JsTokenType::Identifier && (token.line_number_ == lookahead_.line_number_) && token.value_ == u"async") {
                if (lookahead_.type_ == JsTokenType::Identifier || Match(JsTokenType::K_Yield)) {
                    Sp<SyntaxNode> arg = ParsePrimaryExpression();
                    arg = ReinterpretExpressionAsPattern(arg);

                    auto node = Alloc<ArrowParameterPlaceHolder>();
                    node->params.push_back(arg);
                    node->async = true;
                    expr = move(node);

                }
            }

            if (expr->type == SyntaxNodeType::ArrowParameterPlaceHolder || Match(JsTokenType::Arrow)) {

                context_.is_assignment_target = false;
                context_.is_binding_element = false;
                bool is_async = false;
                if (auto placeholder = dynamic_pointer_cast<ArrowParameterPlaceHolder>(expr); placeholder) {
                    is_async = placeholder->async;
                }

                if (auto list = ReinterpretAsCoverFormalsList(expr); list.has_value()) {
                    if (has_line_terminator_) {
                        TolerateUnexpectedToken(lookahead_);
                    }
                    context_.first_cover_initialized_name_error.reset();

                    bool prev_strict = context_.strict_;
                    bool prev_allow_strict_directive = context_.allow_strict_directive;
                    context_.allow_strict_directive = list->simple;

                    bool prev_allow_yield = context_.allow_yield;
                    bool prev_await = context_.await;
                    context_.allow_yield = true;
                    context_.await = is_async;

                    auto marker = CreateStartMarker();
                    Expect(JsTokenType::Arrow);
                    Sp<SyntaxNode> body;

                    if (Match(JsTokenType::LeftBracket)) {
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

                    if (context_.strict_ && list->first_restricted) {
                        ThrowUnexpectedToken(*list->first_restricted, list->message);
                        return nullptr;
                    }
                    if (context_.strict_ && list->stricted) {
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
                    }

                    auto temp = Alloc<AssignmentExpression>();

                    if (!Match(JsTokenType::Assign)) {
                        context_.is_assignment_target = false;
                        context_.is_binding_element = false;
                    } else {
//                        Assert(!!temp->left, "left should not be null");
                    }
                    temp->left = ReinterpretExpressionAsPattern(expr);

                    token = NextToken();
                    auto operator_ = token.value_;
                    temp->right = IsolateCoverGrammar<Expression>([this] {
                        return ParseAssignmentExpression();
                    });
                    temp->operator_ = operator_;
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

        if (Match(JsTokenType::Ask)) {
            NextToken();
            auto node = Alloc<ConditionalExpression>();

            bool prev_allow_in = context_.allow_in;
            context_.allow_in = true;

            node->consequent = IsolateCoverGrammar<Expression>([this] {
                return ParseAssignmentExpression();
            });
            context_.allow_in = prev_allow_in;

            Expect(JsTokenType::Colon);
            node->alternate = IsolateCoverGrammar<Expression>([this] {
                return ParseAssignmentExpression();
            });

            node->test = move(expr);
            expr = move(node);

            context_.is_assignment_target = false;
            context_.is_binding_element = false;
        }

        return Finalize(marker, expr);
    }

    Sp<Expression> Parser::ParseBinaryExpression() {
        Token start_token = lookahead_;

        auto expr = InheritCoverGrammar<Expression>([this] {
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

                    precedences.pop();

                    left = expr_stack.top();
                    expr_stack.pop();

                    markers.pop();

                    auto top_marker = markers.top();
                    auto node = Alloc<BinaryExpression>();
                    node->operator_ = operator_;
                    node->left = left;
                    node->right = right;
                    expr_stack.push(Finalize(StartNode(top_marker), node));
                }

                // shift
                Token next_ = NextToken();
                op_stack.push(next_.value_);
                precedences.push(prec);
                markers.push(lookahead_);
                auto temp_expr = IsolateCoverGrammar<Expression>([this] {
                    return ParseExponentiationExpression();
                });
                expr_stack.push(temp_expr);
            }

            expr = expr_stack.top();

            auto last_marker = markers.top();
            markers.pop();
            while (expr_stack.size() >= 2) {
                Assert(!markers.empty(), "ParseBinaryExpression: markers should not be mepty");
                auto marker = markers.top();
                markers.pop();
                auto last_line_start = last_marker.line_start_;
                auto operator_ = move(op_stack.top());
                op_stack.pop();
                auto start_marker = StartNode(last_marker, last_line_start);
                auto node = Alloc<BinaryExpression>();
                node->operator_ = operator_;
                node->right = expr;
                expr_stack.pop();
                node->left = expr_stack.top();
                expr_stack.pop();
                expr = Finalize(start_marker, node);
                last_marker = last_marker;
            }
        }

        return expr;
    }

    Sp<Expression> Parser::ParseExponentiationExpression() {
        auto start = CreateStartMarker();

        Sp<Expression> expr = InheritCoverGrammar<Expression>([this] {
            return ParseUnaryExpression();
        });

        Assert(!!expr, "ParseExponentiationExpression: expr should not be null");

        if (expr->type != SyntaxNodeType::UnaryExpression && Match(JsTokenType::Pow)) {
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
            Match(JsTokenType::Plus) || Match(JsTokenType::Minus) || Match(JsTokenType::Wave) || Match(JsTokenType::Not) ||
            Match(JsTokenType::K_Delete) || Match(JsTokenType::K_Void) || Match(JsTokenType::K_Typeof)
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
            node->prefix = true;
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

        if (Match(JsTokenType::Increase) || Match(JsTokenType::Decrease)) {
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
            if (!has_line_terminator_ && IsPunctuatorToken(lookahead_.type_)) {
                if (Match(JsTokenType::Increase) || Match(JsTokenType::Decrease)) {
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
        if (Match(JsTokenType::K_Super) && context_.in_function_body) {
            expr = ParseSuper();
        } else {
            expr = InheritCoverGrammar<Expression>([this] {
                if (Match(JsTokenType::K_New)) {
                    return ParseNewExpression();
                } else {
                    return ParsePrimaryExpression();
                }
            });
        }

        while (true) {
            if (Match(JsTokenType::LeftBrace)) {
                context_.is_binding_element = false;
                context_.is_assignment_target = true;
                Expect(JsTokenType::LeftBrace);
                auto node = Alloc<MemberExpression>();
                node->computed = true;
                node->property = IsolateCoverGrammar<Expression>([this] {
                    return ParseExpression();
                });
                Expect(JsTokenType::RightBrace);
                node->object = move(expr);
                expr = Finalize(start_marker, node);
            } else if (Match(JsTokenType::Dot)) {
                context_.is_binding_element = false;
                context_.is_assignment_target = true;
                Expect(JsTokenType::Dot);
                auto node = Alloc<MemberExpression>();
                node->computed = false;
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

        Expect(JsTokenType::K_Super);
        if (!Match(JsTokenType::LeftBrace) && !Match(JsTokenType::Dot)) {
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
        Expect(JsTokenType::K_Import);

        Sp<Literal> src;
        std::vector<Sp<SyntaxNode>> specifiers;

        if (lookahead_.type_ == JsTokenType::StringLiteral) {
            src = ParseModuleSpecifier();
        } else {
            if (Match(JsTokenType::LeftBracket)) {
                auto children = ParseNamedImports();
                specifiers.insert(specifiers.end(), children.begin(), children.end());
            } else if (Match(JsTokenType::Mul)) {
                specifiers.push_back(ParseImportNamespaceSpecifier());
            } else if (IsIdentifierName(lookahead_) && !Match(JsTokenType::K_Default)) {
                auto default_ = ParseImportDefaultSpecifier();
                specifiers.push_back(move(default_));

                if (Match(JsTokenType::Comma)) {
                    NextToken();
                    if (Match(JsTokenType::Mul)) {
                        specifiers.push_back(ParseImportNamespaceSpecifier());
                    } else if (Match(JsTokenType::LeftBracket)) {
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
        Expect(JsTokenType::LeftBracket);
        std::vector<Sp<SyntaxNode>> specifiers;
        while (!Match(JsTokenType::RightBracket)) {
            specifiers.push_back(ParseImportSpecifier());
            if (!Match(JsTokenType::RightBracket)) {
                Expect(JsTokenType::Comma);
            }
        }
        Expect(JsTokenType::RightBracket);

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

        Expect(JsTokenType::Mul);
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
        auto start_marker = CreateStartMarker();
        auto node = Alloc<VariableDeclaration>();
        Token next = NextToken();
        if (next.value_ == u"let") {
            node->kind = VarKind::Let;
        } else if (next.value_ == u"const") {
            node->kind = VarKind::Const;
        } else {
            ThrowUnexpectedToken(next);
        }

        node->declarations = ParseBindingList(node->kind, in_for);
        ConsumeSemicolon();

        return Finalize(start_marker, node);
    }

    Sp<Identifier> Parser::ParseVariableIdentifier(VarKind kind) {
        auto marker = CreateStartMarker();

        Token token = NextToken();
        if (Match(JsTokenType::K_Yield)) {
            if (context_.strict_) {
                TolerateUnexpectedToken(token, ParseMessages::StrictReservedWord);
            } else if (!context_.allow_yield) {
                ThrowUnexpectedToken(token);
                return nullptr;
            }
        } else if (token.type_ != JsTokenType::Identifier) {
            ThrowUnexpectedToken(token);
            return nullptr;
        } else if ((context_.is_module || context_.await) && token.type_ == JsTokenType::Identifier && token.value_ == u"await") {
            TolerateUnexpectedToken(token);
        }

        auto node = Alloc<Identifier>();
        node->name = token.value_;
        return Finalize(marker, node);
    }

    Sp<SyntaxNode> Parser::ParsePattern(std::vector<Token> &params, VarKind kind) {
        Sp<SyntaxNode> pattern;

        if (Match(JsTokenType::LeftBrace)) {
            pattern = ParseArrayPattern(params, kind);
        } else if (Match(JsTokenType::LeftBracket)) {
            pattern = ParseObjectPattern(params, kind);
        } else {
            if (Match(JsTokenType::K_Let) && (kind == VarKind::Const || kind == VarKind::Let)) {
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
        if (Match(JsTokenType::Assign)) {
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

        Expect(JsTokenType::Spread);
        auto node = Alloc<RestElement>();
        node->argument = ParsePattern(params, kind);

        return Finalize(start_marker, node);
    }

    Sp<ArrayPattern> Parser::ParseArrayPattern(std::vector<Token> &params, VarKind kind) {
        auto start_marker = CreateStartMarker();

        Expect(JsTokenType::LeftBrace);
        std::vector<optional<Sp<SyntaxNode>>> elements;
        while (!Match(JsTokenType::RightBrace)) {
            if (Match(JsTokenType::Comma)) {
                NextToken();
                elements.emplace_back(nullopt);
            } else {
                if (Match(JsTokenType::Spread)) {
                    elements.emplace_back(ParseBindingRestElement(params, kind));
                    break;
                } else {
                    elements.emplace_back(ParsePatternWithDefault(params, kind));
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
            if (Match(JsTokenType::Assign)) {
                params.push_back(keyToken);
                node->shorthand = true;
                NextToken();
                auto expr = ParseAssignmentExpression();
                auto assign = Alloc<AssignmentPattern>();
                assign->left = move(init);
                assign->right = move(expr);
                node->value = Finalize(StartNode(keyToken), assign);
            } else if (!Match(JsTokenType::Colon)) {
                params.push_back(keyToken);
                node->shorthand = true;
                node->value = move(init);
            } else {
                Expect(JsTokenType::Colon);
                node->value = ParsePatternWithDefault(params, kind);
            }
        } else {
            node->computed = Match(JsTokenType::LeftBrace);
            node->key = ParseObjectPropertyKey();
            Expect(JsTokenType::Colon);
            node->value = ParsePatternWithDefault(params, kind);
        }

        return Finalize(start_marker, node);
    }

    Sp<RestElement> Parser::ParseRestProperty(std::vector<Token> &params, VarKind kind) {
        auto start_marker = CreateStartMarker();
        Expect(JsTokenType::Spread);
        auto arg = ParsePattern(params, VarKind::Invalid);
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

    Sp<ObjectPattern> Parser::ParseObjectPattern(std::vector<Token> &params, VarKind kind) {
        auto start_marker = CreateStartMarker();
        std::vector<Sp<SyntaxNode>> props;

        Expect(JsTokenType::LeftBracket);
        while (!Match(JsTokenType::RightBracket)) {
            if (Match(JsTokenType::Spread)) {
                props.push_back(ParseRestProperty(params, kind));
            } else {
                props.push_back(ParsePropertyPattern(params, kind));
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

    Sp<SyntaxNode> Parser::ParseAsyncArgument() {
        auto arg = ParseAssignmentExpression();
        context_.first_cover_initialized_name_error.reset();
        return arg;
    }

    std::vector<Sp<SyntaxNode>> Parser::ParseAsyncArguments() {
        Expect(JsTokenType::LeftParen);
        std::vector<Sp<SyntaxNode>> result;
        if (!Match(JsTokenType::RightParen)) {
            while (true) {
                if (Match(JsTokenType::Spread)) {
                    result.push_back(ParseSpreadElement());
                } else {
                    result.push_back(IsolateCoverGrammar<SyntaxNode>([this] {
                        return ParseAsyncArgument();
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

    Sp<Expression> Parser::ParseGroupExpression() {
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
            auto start_token = lookahead_;

            std::vector<Token> params;
            if (Match(JsTokenType::Spread)) {
                expr = ParseRestElement(params);
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
                context_.is_binding_element = true;

                expr = InheritCoverGrammar<Expression>([this] {
                    return ParseAssignmentExpression();
                });

                if (Match(JsTokenType::Comma)) {
                    std::vector<Sp<Expression>> expressions;

                    context_.is_assignment_target = false;
                    expressions.push_back(expr);
                    while (lookahead_.type_ != JsTokenType::EOF_) {
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
                            if (!context_.is_binding_element) {
                                ThrowUnexpectedToken(lookahead_);
                            }
                            expressions.push_back(ParseRestElement(params));
                            Expect(JsTokenType::RightParen);
                            if (!Match(JsTokenType::Arrow)) {
                                Expect(JsTokenType::Arrow);
                            }
                            context_.is_binding_element = false;
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
                            expressions.push_back(InheritCoverGrammar<Expression>([this] {
                                return ParseAssignmentExpression();
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
                        if (expr->type == SyntaxNodeType::Identifier && dynamic_pointer_cast<Identifier>(expr)->name == u"yield") {
                            arrow = true;
                            auto node = Alloc<ArrowParameterPlaceHolder>();
                            node->params.push_back(move(expr));
                            node->async = false;
                            expr = move(node);
                        }

                        if (!arrow) {
                            if (!context_.is_binding_element) {
                                ThrowUnexpectedToken(lookahead_);
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

                    context_.is_binding_element = false;
                }
            }
        }

        return expr;
    }

    Sp<TemplateElement> Parser::ParseTemplateHead() {
        Assert(lookahead_.head_, "Template literal must start with a template head");
        auto start_marker = CreateStartMarker();
        Token token = NextToken();

        auto node = Alloc<TemplateElement>();
        node->raw = token.value_;
        node->cooked = token.cooked_;
        node->tail = token.tail_;

        return Finalize(start_marker, node);
    }

    Sp<TemplateElement> Parser::ParseTemplateElement() {
        if (lookahead_.type_ != JsTokenType::Template) {
            ThrowUnexpectedToken(lookahead_);
        }

        auto start_marker = CreateStartMarker();
        Token token = NextToken();

        auto node = Alloc<TemplateElement>();
        node->raw = token.value_;
        node->cooked = token.cooked_;
        node->tail = token.tail_;

        return Finalize(start_marker, node);
    }

    Sp<TemplateLiteral> Parser::ParseTemplateLiteral() {
        auto start_marker = CreateStartMarker();
        auto node = Alloc<TemplateLiteral>();

        Sp<TemplateElement> quasi = ParseTemplateHead();
        node->quasis.push_back(quasi);
        while (!quasi->tail) {
            node->expressions.push_back(ParseExpression());
            quasi = ParseTemplateElement();
            node->quasis.push_back(quasi);
        }

        return Finalize(start_marker, node);
    }

    Sp<MethodDefinition> Parser::ParseClassElement(bool &has_ctor) {
        Token token = lookahead_;
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
            key = ParseObjectPropertyKey();
            auto id = dynamic_pointer_cast<Identifier>(*key);
            if (id && id->name == u"static" && (QualifiedPropertyName(lookahead_) || Match(JsTokenType::Mul))) {
                token = lookahead_;
                is_static = true;
                computed = Match(JsTokenType::LeftBrace);
                if (Match(JsTokenType::Mul)) {
                    NextToken();
                } else {
                    key = ParseObjectPropertyKey();
                }
            }
            if ((token.type_ == JsTokenType::Identifier) && !has_line_terminator_ && (token.value_ == u"async")) {
                if (token.type_ != JsTokenType::Colon && token.type_ != JsTokenType::LeftParen && token.type_ != JsTokenType::Mul) {
                    is_async = true;
                    token = lookahead_;
                    key = ParseObjectPropertyKey();
                    if (token.type_ == JsTokenType::Identifier && token.value_ == u"constructor") {
                        TolerateUnexpectedToken(token, ParseMessages::ConstructorIsAsync);
                    }
                }
            }
        }

        bool lookahead_prop_key = QualifiedPropertyName(lookahead_);
        if (token.type_ == JsTokenType::Identifier) {
            if (token.value_ == u"get" && lookahead_prop_key) {
                kind = VarKind::Get;
                computed = Match(JsTokenType::LeftBrace);
                key = ParseObjectPropertyKey();
                context_.allow_yield = false;
                value = ParseGetterMethod();
            } else if (token.value_ == u"set" && lookahead_prop_key) {
                kind = VarKind::Set;
                computed = Match(JsTokenType::LeftBrace);
                key = ParseObjectPropertyKey();
                value = ParseSetterMethod();
            }
        } else if (token.type_ == JsTokenType::Mul && lookahead_prop_key) {
            kind = VarKind::Init;
            computed = Match(JsTokenType::LeftBrace);
            key = ParseObjectPropertyKey();
            value = ParseGeneratorMethod();
            method = true;
        }

        if (kind == VarKind::Invalid && key.has_value() && Match(JsTokenType::LeftParen)) {
            kind = VarKind::Init;
            if (is_async) {
                value = ParsePropertyMethodAsyncFunction();
            } else {
                value = ParsePropertyMethodFunction();
            }
            method = true;
        }

        if (kind == VarKind::Invalid) {
            ThrowUnexpectedToken(lookahead_);
        }

        if (kind == VarKind::Init) {
            kind = VarKind::Method;
        }

        if (!computed) {
            if (is_static && IsPropertyKey(*key, u"prototype")) {
                ThrowUnexpectedToken(token, ParseMessages::StaticPrototype);
            }
            if (!is_static && IsPropertyKey(*key, u"constructor")) {
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

    std::vector<Sp<MethodDefinition>> Parser::ParseClassElementList() {
        std::vector<Sp<MethodDefinition>> body;
        bool has_ctor = false;

        Expect(JsTokenType::LeftBracket);
        while (!Match(JsTokenType::RightBracket)) {
            if (Match(JsTokenType::Semicolon)) {
                NextToken();
            } else {
                body.push_back(ParseClassElement(has_ctor));
            }
        }
        Expect(JsTokenType::RightBracket);

        return body;
    }

    Sp<FunctionExpression> Parser::ParseGetterMethod() {
        auto start_marker = CreateStartMarker();
        auto node = Alloc<FunctionExpression>();

        node->generator = false;
        bool prev_allow_yield = context_.allow_yield;
        context_.allow_yield = !node->generator;

        auto formal = ParseFormalParameters();
        if (!formal.params.empty()) {
            TolerateError(ParseMessages::BadGetterArity);
        }

        node->params = formal.params;
        node->body = ParsePropertyMethod(formal);
        context_.allow_yield = prev_allow_yield;

        return Finalize(start_marker, node);
    }

    Sp<FunctionExpression> Parser::ParseSetterMethod() {
        auto start_marker = CreateStartMarker();
        auto node = Alloc<FunctionExpression>();

        node->generator = false;
        bool prev_allow_yield = context_.allow_yield;
        context_.allow_yield = !node->generator;
        auto options = ParseFormalParameters();
        if (options.params.size() != 1) {
            TolerateError(ParseMessages::BadSetterArity);
        } else if (options.params[0]->type == SyntaxNodeType::RestElement) {
            TolerateError(ParseMessages::BadSetterRestParameter);
        }
        node->params = options.params;
        node->body = ParsePropertyMethod(options);
        context_.allow_yield = prev_allow_yield;

        return Finalize(start_marker, node);
    }

    Sp<BlockStatement> Parser::ParsePropertyMethod(parser::ParserCommon::FormalParameterOptions &options) {
        context_.is_assignment_target = false;
        context_.is_binding_element = false;

        bool prev_strict = context_.strict_;
        bool prev_allow_strict_directive = context_.allow_strict_directive;
        context_.allow_strict_directive = options.simple;

        auto body = IsolateCoverGrammar<BlockStatement>([this] {
            return ParseFunctionSourceElements();
        });
        if (context_.strict_ && options.first_restricted.has_value()) {
            TolerateUnexpectedToken(*options.first_restricted, options.message);
        }
        if (context_.strict_ && options.stricted.has_value()) {
            TolerateUnexpectedToken(*options.stricted, options.message);
        }
        context_.strict_ = prev_strict;
        context_.allow_strict_directive = prev_allow_strict_directive;

        return body;
    }

    Sp<FunctionExpression> Parser::ParseGeneratorMethod() {
        auto start_marker = CreateStartMarker();
        auto node = Alloc<FunctionExpression>();

        node->generator = true;
        bool prev_allow_yield = context_.allow_yield;

        context_.allow_yield = true;
        auto params = ParseFormalParameters();
        context_.allow_yield = false;
        auto method = ParsePropertyMethod(params);
        context_.allow_yield = prev_allow_yield;

        node->params = params.params;
        node->body = move(method);

        return Finalize(start_marker, node);
    }

    Sp<FunctionExpression> Parser::ParsePropertyMethodFunction() {
        auto start_marker = CreateStartMarker();
        auto node = Alloc<FunctionExpression>();

        node->generator = false;
        bool prev_allow_yield = context_.allow_yield;
        context_.allow_yield = true;
        auto params = ParseFormalParameters();
        auto method = ParsePropertyMethod(params);
        context_.allow_yield = prev_allow_yield;

        node->params = params.params;
        node->body = move(method);

        return Finalize(start_marker, node);
    }

    Sp<FunctionExpression> Parser::ParsePropertyMethodAsyncFunction() {
        auto start_marker = CreateStartMarker();
        auto node = Alloc<FunctionExpression>();

        node->generator = false;
        bool prev_allow_yield = context_.allow_yield;
        bool prev_await = context_.await;
        context_.allow_yield = false;
        context_.await = true;
        auto params = ParseFormalParameters();
        auto method = ParsePropertyMethod(params);
        context_.allow_yield = prev_allow_yield;
        context_.await = prev_await;

        node->params = params.params;
        node->body = move(method);
        node->async = true;

        return Finalize(start_marker, node);
    }

    bool Parser::QualifiedPropertyName(const Token &token) {
        if (IsKeywordToken(token.type_)) return true;

        switch (token.type_) {
            case JsTokenType::Identifier:
            case JsTokenType::StringLiteral:
            case JsTokenType::BooleanLiteral:
            case JsTokenType::NullLiteral:
            case JsTokenType::NumericLiteral:
            case JsTokenType::LeftBrace:
                return true;

            default:
                break;
        }

        return false;
    }

    bool Parser::IsPropertyKey(const Sp<SyntaxNode> &key, const UString &name) {
        if (key->type == SyntaxNodeType::Identifier) {
            return dynamic_pointer_cast<Identifier>(key)->name == name;
        }
        if (key->type == SyntaxNodeType::Literal) {
            auto lit = dynamic_pointer_cast<Literal>(key);
            return std::holds_alternative<UString>(lit->value) && std::get<UString>(lit->value) == name;
        }
        return false;
    }

}

