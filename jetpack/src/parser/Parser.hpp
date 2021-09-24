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

namespace jetpack::parser {

    class JSXParser;
    class TypeScriptParser;

    class Parser final: private ParserCommon {
    public:
        friend class JSXParser;
        friend class TypeScriptParser;

        Parser(Sp<ParserContext> state);

        template <typename T>
        T* IsolateCoverGrammar(std::function<T*()> cb) {
            auto previousIsBindingElement = ctx->is_binding_element_;
            auto previousIsAssignmentTarget = ctx->is_assignment_target_;
            auto previousFirstCoverInitializedNameError = ctx->first_cover_initialized_name_error_;

            ctx->is_binding_element_ = true;
            ctx->is_assignment_target_ = true;
            ctx->first_cover_initialized_name_error_.reset();

            T* result = cb();
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
        T* InheritCoverGrammar(std::function<T*()> cb) {
            auto previousIsBindingElement = ctx->is_binding_element_;
            auto previousIsAssignmentTarget = ctx->is_assignment_target_;
            auto previousFirstCoverInitializedNameError = ctx->first_cover_initialized_name_error_;

            ctx->is_binding_element_ = true;
            ctx->is_assignment_target_ = true;
            ctx->first_cover_initialized_name_error_.reset();

            T* result = cb();

            ctx->is_binding_element_ &= previousIsBindingElement;
            ctx->is_assignment_target_ &= previousIsAssignmentTarget;
            ctx->first_cover_initialized_name_error_ =
                previousFirstCoverInitializedNameError ?
                previousFirstCoverInitializedNameError : ctx->first_cover_initialized_name_error_;

            return result;
        }

        Expression* ParsePrimaryExpression(Scope& scope);

        SpreadElement* ParseSpreadElement(Scope& scope);

        BlockStatement* ParsePropertyMethod(Scope& scope, FormalParameterOptions& options);

        FunctionExpression* ParsePropertyMethodFunction(Scope& scope);

        FunctionExpression* ParsePropertyMethodAsyncFunction(Scope& scope);

        Property* ParseObjectProperty(Scope& scope, bool& has_proto);

        SyntaxNode* ParseObjectPropertyKey(Scope& scope);

        Expression* ParseObjectInitializer(Scope& scope);

        TemplateElement* ParseTemplateHead();

        TemplateElement* ParseTemplateElement();

        TemplateLiteral* ParseTemplateLiteral(Scope& scope);

        Pattern* ReinterpretExpressionAsPattern(SyntaxNode* expr);

        std::optional<FormalParameterOptions> ReinterpretAsCoverFormalsList(SyntaxNode* ptr);

        void CheckPatternParam(FormalParameterOptions& options, SyntaxNode* param);

        Expression* ParseGroupExpression(Scope& scope);

        std::vector<SyntaxNode*> ParseArguments(Scope& scope);

        Identifier* ParseIdentifierName();

        Expression* ParseNewExpression(Scope& scope);

        SyntaxNode* ParseAsyncArgument(Scope& scope);

        std::vector<SyntaxNode*> ParseAsyncArguments(Scope& scope);

        Import* ParseImportCall();

        Expression* ParseLeftHandSideExpressionAllowCall(Scope& scope);

        Super* ParseSuper();

        Expression* ParseLeftHandSideExpression(Scope& scope);

        Expression* ParseUpdateExpression(Scope& scope);

        AwaitExpression* ParseAwaitExpression(Scope& scope);

        Expression* ParseUnaryExpression(Scope& scope);

        Expression* ParseExponentiationExpression(Scope& scope);

        Expression* ParseBinaryExpression(Scope& scope);
        Expression* ParseBinaryExpression(Scope& scope, Expression* left, const Token& left_tk);

        Expression* ParseConditionalExpression(Scope& scope);

        Expression*
        ParseAssignmentExpression(Scope& scope);

        Statement* ParseStatementListItem(Scope& scope);

        BlockStatement* ParseBlock(Scope& scope, bool new_scope);

        VariableDeclarator* ParseLexicalBinding(Scope& scope, VarKind kind, bool &in_for);

        std::vector<VariableDeclarator*> ParseBindingList(Scope& scope, VarKind kind, bool& in_for);

        bool IsLexicalDeclaration();

        Declaration* ParseLexicalDeclaration(Scope& scope, bool& in_for);

        RestElement* ParseBindingRestElement(Scope& scope, std::vector<Token> &params, VarKind kind);

        ArrayPattern* ParseArrayPattern(Scope& scope, std::vector<Token> &params, VarKind kind);

        Property* ParsePropertyPattern(Scope& scope, std::vector<Token> &params, VarKind kind);

        RestElement* ParseRestProperty(Scope& scope, std::vector<Token> &params, VarKind kind);

        ObjectPattern* ParseObjectPattern(Scope& scope, std::vector<Token> &params, VarKind kind);

        Expression* ParseArrayInitializer(Scope& scope);

        FormalParameterOptions ParseFormalParameters(Scope& scope, std::optional<Token> first_restricted = std::nullopt);
        void ParseFormalParameter(Scope& scope, FormalParameterOptions& option);
        void ValidateParam(FormalParameterOptions& option, const Token& param, const std::string& name);
        bool IsStartOfExpression();

        RestElement* ParseRestElement(Scope& scope, std::vector<Token>& params);

        SyntaxNode* ParsePattern(Scope& scope, std::vector<Token>& params, VarKind kind = VarKind::Invalid);

        SyntaxNode* ParsePatternWithDefault(Scope& scope, std::vector<Token> &params, VarKind kind);

        Identifier* ParseVariableIdentifier(Scope& scope, VarKind kind);

        VariableDeclarator* ParseVariableDeclaration(Scope& scope, bool in_for);

        std::vector<VariableDeclarator*> ParseVariableDeclarationList(Scope& scope, bool in_for);

        VariableDeclaration* ParseVariableStatement(Scope& scope);

        EmptyStatement* ParseEmptyStatement();

        ExpressionStatement* ParseExpressionStatement(Scope& scope);

        Expression* ParseExpression(Scope& scope);

        Statement* ParseIfClause(Scope& scope);

        IfStatement* ParseIfStatement(Scope& scope);

        DoWhileStatement* ParseDoWhileStatement(Scope& scope);

        WhileStatement* ParseWhileStatement(Scope& scope);

        Statement* ParseForStatement(Scope& scope);

        ContinueStatement* ParseContinueStatement();

        BreakStatement* ParseBreakStatement();

        ReturnStatement* ParseReturnStatement(Scope& scope);

        WithStatement* ParseWithStatement(Scope& scope);

        SwitchCase* ParseSwitchCase(Scope& scope);

        SwitchStatement* ParseSwitchStatement(Scope& scope);

        Statement* ParseLabelledStatement(Scope& scope);

        ThrowStatement* ParseThrowStatement(Scope& scope);

        CatchClause* ParseCatchClause(Scope& scope);

        BlockStatement* ParseFinallyClause(Scope& scope);

        TryStatement* ParseTryStatement(Scope& scope);

        DebuggerStatement* ParseDebuggerStatement();

        Statement* ParseStatement(Scope& scope);

        BlockStatement* ParseFunctionSourceElements(Scope& scope);

        FunctionDeclaration* ParseFunctionDeclaration(Scope& scope, bool identifier_is_optional);

        FunctionExpression* ParseFunctionExpression(Scope& scope);

        Statement* ParseDirective(Scope& scope);

        std::vector<SyntaxNode*> ParseDirectivePrologues(Scope& scope);

        FunctionExpression* ParseGetterMethod(Scope& scope);

        FunctionExpression* ParseSetterMethod(Scope& scope);

        FunctionExpression* ParseGeneratorMethod(Scope& scope);

        YieldExpression* ParseYieldExpression(Scope& scope);

        bool QualifiedPropertyName(const Token& token);

        MethodDefinition* ParseClassElement(Scope& scope, bool& has_ctor);

        std::vector<MethodDefinition*> ParseClassElementList(Scope& scope);

        bool IsPropertyKey(SyntaxNode* key, const std::string& name);

        ClassBody* ParseClassBody(Scope& scope);

        ClassDeclaration* ParseClassDeclaration(Scope& scope, bool identifier_is_optional);

        ClassExpression* ParseClassExpression(Scope& scope);

        Module* ParseModule();

        Script* ParseScript();

        Literal* ParseModuleSpecifier();

        ImportSpecifier* ParseImportSpecifier(Scope& scope);

        std::vector<SyntaxNode*> ParseNamedImports(Scope& scope);

        ImportDefaultSpecifier* ParseImportDefaultSpecifier(Scope& scope);

        ImportNamespaceSpecifier* ParseImportNamespaceSpecifier(Scope& scope);

        ImportDeclaration* ParseImportDeclaration(Scope& scope);

        ExportSpecifier* ParseExportSpecifier(Scope& scope);

        Declaration* ParseExportDeclaration(Scope& scope);

        bool MatchImportCall();

        inline bool MatchAsyncFunction();

        inline std::vector<Sp<Comment>>& Comments() {
            return ctx->comments_;
        }

        std::optional<SyntaxNode*> CheckRequireCall(Scope& scope, CallExpression* call);

        ~Parser() = default;

        NodeCreatedEventEmitter<ImportDeclaration> import_decl_created_listener;
        NodeCreatedEventEmitter<ExportNamedDeclaration> export_named_decl_created_listener;
        NodeCreatedEventEmitter<ExportAllDeclaration> export_all_decl_created_listener;
        NodeCreatedEventEmitterRet<std::optional<SyntaxNode*>, CallExpression> require_call_created_listener;

    };

    inline bool Parser::MatchAsyncFunction() {
        bool match = MatchContextualKeyword("async");
        if (match) {
            auto state = ctx->scanner_->SaveState();
            std::vector<Sp<Comment>> comments;
            ctx->scanner_->ScanComments(comments);
            Token next = ctx->scanner_->Lex();
            ctx->scanner_->RestoreState(state);

            match = (state.line_number_ == next.lineNumber) && IsKeywordToken(next.type) && (next.value == "function");
        }

        return match;
    }

}
