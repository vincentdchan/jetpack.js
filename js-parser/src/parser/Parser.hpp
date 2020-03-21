//
// Created by Duzhong Chen on 2019/9/3.
//
#pragma once

#include <vector>
#include <iostream>
#include "../tokenizer/Token.h"
#include "ParserContext.h"
#include "ParserCommon.h"
#include "ErrorMessage.h"
#include "NodesSize.h"

#define ASSERT_NOT_NULL(EXPR, MSG) if ((EXPR) == nullptr) { \
        LogError(std::string(#EXPR) + " should not be nullptr " + MSG); \
        return false; \
    }

namespace rocket_bundle::parser {

    class JSXParser;
    class TypeScriptParser;

    class Parser final: private ParserCommon {
    public:
        friend class JSXParser;
        friend class TypeScriptParser;

        typedef std::function<void (const UString&)> NewImportLocationAddedCallback;

        Parser(std::shared_ptr<ParserContext> state):
        ParserCommon(state) {}
//            nodes_pool(node_size::max_node_size) {}

        void OnNewImportLocationAdded(NewImportLocationAddedCallback callback);

        template <typename T>
        Sp<T> IsolateCoverGrammar(std::function<Sp<T>()> cb) {
            auto previousIsBindingElement = ctx->is_binding_element_;
            auto previousIsAssignmentTarget = ctx->is_assignment_target_;
            auto previousFirstCoverInitializedNameError = ctx->first_cover_initialized_name_error_;

            ctx->is_binding_element_ = true;
            ctx->is_assignment_target_ = true;
            ctx->first_cover_initialized_name_error_.reset();

            Sp<T> result = cb();
            if (ctx->first_cover_initialized_name_error_) {
                ThrowUnexpectedToken(Token(), "firstCoverInitializedNameError");
                return nullptr;
            }

            ctx->is_binding_element_ = previousIsBindingElement;
            ctx->is_assignment_target_ = previousIsAssignmentTarget;
            ctx->first_cover_initialized_name_error_ = previousFirstCoverInitializedNameError;

            return result;
        }

        template <typename T>
        Sp<T> InheritCoverGrammar(std::function<Sp<T>()> cb) {
            auto previousIsBindingElement = ctx->is_binding_element_;
            auto previousIsAssignmentTarget = ctx->is_assignment_target_;
            auto previousFirstCoverInitializedNameError = ctx->first_cover_initialized_name_error_;

            ctx->is_binding_element_ = true;
            ctx->is_assignment_target_ = true;
            ctx->first_cover_initialized_name_error_.reset();

            Sp<T> result = cb();

            ctx->is_binding_element_ &= previousIsBindingElement;
            ctx->is_assignment_target_ &= previousIsAssignmentTarget;
            ctx->first_cover_initialized_name_error_ =
                previousFirstCoverInitializedNameError ?
                previousFirstCoverInitializedNameError : ctx->first_cover_initialized_name_error_;

            return result;
        }

        Sp<Expression> ParsePrimaryExpression(Scope& scope);

        Sp<SpreadElement> ParseSpreadElement(Scope& scope);

        Sp<BlockStatement> ParsePropertyMethod(Scope& scope, FormalParameterOptions& options);

        Sp<FunctionExpression> ParsePropertyMethodFunction(Scope& scope);

        Sp<FunctionExpression> ParsePropertyMethodAsyncFunction(Scope& scope);

        Sp<Property> ParseObjectProperty(Scope& scope, bool& has_proto);

        Sp<SyntaxNode> ParseObjectPropertyKey(Scope& scope);

        Sp<Expression> ParseObjectInitializer(Scope& scope);

        Sp<TemplateElement> ParseTemplateHead();

        Sp<TemplateElement> ParseTemplateElement();

        Sp<TemplateLiteral> ParseTemplateLiteral(Scope& scope);

        Sp<Pattern> ReinterpretExpressionAsPattern(const Sp<SyntaxNode>& expr);

        std::optional<FormalParameterOptions> ReinterpretAsCoverFormalsList(const Sp<SyntaxNode>& ptr);

        void CheckPatternParam(FormalParameterOptions& options, const Sp<SyntaxNode>& param);

        Sp<Expression> ParseGroupExpression(Scope& scope);

        std::vector<Sp<SyntaxNode>> ParseArguments(Scope& scope);

        Sp<Identifier> ParseIdentifierName();

        Sp<Expression> ParseNewExpression(Scope& scope);

        Sp<SyntaxNode> ParseAsyncArgument(Scope& scope);

        std::vector<Sp<SyntaxNode>> ParseAsyncArguments(Scope& scope);

        Sp<Import> ParseImportCall();

        Sp<Expression> ParseLeftHandSideExpressionAllowCall(Scope& scope);

        Sp<Super> ParseSuper();

        Sp<Expression> ParseLeftHandSideExpression(Scope& scope);

        Sp<Expression> ParseUpdateExpression(Scope& scope);

        Sp<AwaitExpression> ParseAwaitExpression(Scope& scope);

        Sp<Expression> ParseUnaryExpression(Scope& scope);

        Sp<Expression> ParseExponentiationExpression(Scope& scope);

        Sp<Expression> ParseBinaryExpression(Scope& scope);

        Sp<Expression> ParseConditionalExpression(Scope& scope);

        Sp<Expression>
        ParseAssignmentExpression(Scope& scope);

        Sp<Statement> ParseStatementListItem(Scope& scope);

        Sp<BlockStatement> ParseBlock(Scope& scope);

        Sp<VariableDeclarator> ParseLexicalBinding(Scope& scope, VarKind kind, bool &in_for);

        std::vector<Sp<VariableDeclarator>> ParseBindingList(Scope& scope, VarKind kind, bool& in_for);

        bool IsLexicalDeclaration();

        Sp<Declaration> ParseLexicalDeclaration(Scope& scope, bool& in_for);

        Sp<RestElement> ParseBindingRestElement(Scope& scope, std::vector<Token> &params, VarKind kind);

        Sp<ArrayPattern> ParseArrayPattern(Scope& scope, std::vector<Token> &params, VarKind kind);

        Sp<Property> ParsePropertyPattern(Scope& scope, std::vector<Token> &params, VarKind kind);

        Sp<RestElement> ParseRestProperty(Scope& scope, std::vector<Token> &params, VarKind kind);

        Sp<ObjectPattern> ParseObjectPattern(Scope& scope, std::vector<Token> &params, VarKind kind);

        Sp<Expression> ParseArrayInitializer(Scope& scope);

        FormalParameterOptions ParseFormalParameters(Scope& scope, std::optional<Token> first_restricted = std::nullopt);
        void ParseFormalParameter(Scope& scope, FormalParameterOptions& option);
        void ValidateParam(FormalParameterOptions& option, const Token& param, const UString& name);
        bool IsStartOfExpression();

        Sp<RestElement> ParseRestElement(Scope& scope, std::vector<Token>& params);

        Sp<SyntaxNode> ParsePattern(Scope& scope, std::vector<Token>& params, VarKind kind = VarKind::Invalid);

        Sp<SyntaxNode> ParsePatternWithDefault(Scope& scope, std::vector<Token> &params, VarKind kind);

        Sp<Identifier> ParseVariableIdentifier(Scope& scope, VarKind kind);

        Sp<VariableDeclarator> ParseVariableDeclaration(Scope& scope, bool in_for);

        std::vector<Sp<VariableDeclarator>> ParseVariableDeclarationList(Scope& scope, bool in_for);

        Sp<VariableDeclaration> ParseVariableStatement(Scope& scope);

        Sp<EmptyStatement> ParseEmptyStatement();

        Sp<ExpressionStatement> ParseExpressionStatement(Scope& scope);

        Sp<Expression> ParseExpression(Scope& scope);

        Sp<Statement> ParseIfClause(Scope& scope);

        Sp<IfStatement> ParseIfStatement(Scope& scope);

        Sp<DoWhileStatement> ParseDoWhileStatement(Scope& scope);

        Sp<WhileStatement> ParseWhileStatement(Scope& scope);

        Sp<Statement> ParseForStatement(Scope& scope);

        Sp<ContinueStatement> ParseContinueStatement();

        Sp<BreakStatement> ParseBreakStatement();

        Sp<ReturnStatement> ParseReturnStatement(Scope& scope);

        Sp<WithStatement> ParseWithStatement(Scope& scope);

        Sp<SwitchCase> ParseSwitchCase(Scope& scope);

        Sp<SwitchStatement> ParseSwitchStatement(Scope& scope);

        Sp<Statement> ParseLabelledStatement(Scope& scope);

        Sp<ThrowStatement> ParseThrowStatement(Scope& scope);

        Sp<CatchClause> ParseCatchClause(Scope& scope);

        Sp<BlockStatement> ParseFinallyClause(Scope& scope);

        Sp<TryStatement> ParseTryStatement(Scope& scope);

        Sp<DebuggerStatement> ParseDebuggerStatement();

        Sp<Statement> ParseStatement(Scope& scope);

        Sp<BlockStatement> ParseFunctionSourceElements(Scope& scope);

        Sp<FunctionDeclaration> ParseFunctionDeclaration(Scope& scope, bool identifier_is_optional);

        Sp<FunctionExpression> ParseFunctionExpression(Scope& scope);

        Sp<Statement> ParseDirective(Scope& scope);

        std::vector<Sp<SyntaxNode>> ParseDirectivePrologues(Scope& scope);

        Sp<FunctionExpression> ParseGetterMethod(Scope& scope);

        Sp<FunctionExpression> ParseSetterMethod(Scope& scope);

        Sp<FunctionExpression> ParseGeneratorMethod(Scope& scope);

        Sp<YieldExpression> ParseYieldExpression(Scope& scope);

        bool QualifiedPropertyName(const Token& token);

        Sp<MethodDefinition> ParseClassElement(Scope& scope, bool& has_ctor);

        std::vector<Sp<MethodDefinition>> ParseClassElementList(Scope& scope);

        bool IsPropertyKey(const Sp<SyntaxNode>& key, const UString& name);

        Sp<ClassBody> ParseClassBody(Scope& scope);

        Sp<ClassDeclaration> ParseClassDeclaration(Scope& scope, bool identifier_is_optional);

        Sp<ClassExpression> ParseClassExpression(Scope& scope);

        Sp<Module> ParseModule();

        Sp<Script> ParseScript();

        Sp<Literal> ParseModuleSpecifier();

        Sp<ImportSpecifier> ParseImportSpecifier(Scope& scope);

        std::vector<Sp<SyntaxNode>> ParseNamedImports(Scope& scope);

        Sp<ImportDefaultSpecifier> ParseImportDefaultSpecifier();

        Sp<ImportNamespaceSpecifier> ParseImportNamespaceSpecifier();

        Sp<ImportDeclaration> ParseImportDeclaration(Scope& scope);

        Sp<ExportSpecifier> ParseExportSpecifier();

        Sp<Declaration> ParseExportDeclaration(Scope& scope);

        bool MatchImportCall();

        inline bool MatchAsyncFunction();

        inline std::vector<Sp<Comment>>& Comments() {
            return ctx->comments_;
        }

        ~Parser() = default;

    private:
        std::vector<NewImportLocationAddedCallback> import_decl_handlers_;

    };

    inline bool Parser::MatchAsyncFunction() {
        bool match = MatchContextualKeyword(u"async");
        if (match) {
            auto state = ctx->scanner_->SaveState();
            std::vector<Sp<Comment>> comments;
            ctx->scanner_->ScanComments(comments);
            Token next = ctx->scanner_->Lex();
            ctx->scanner_->RestoreState(state);

            match = (state.line_number_ == next.line_number_) && IsKeywordToken(next.type_) && (next.value_ == u"function");
        }

        return match;
    }

}
