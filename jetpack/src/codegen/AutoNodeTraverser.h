//
// Created by Duzhong Chen on 2021/9/24.
//

#pragma once
#include "parser/SyntaxNodes.h"

namespace jetpack {

    class AutoNodeTraverser {
    public:

        AutoNodeTraverser() = default;

        void TraverseNode(SyntaxNode* node);

        virtual bool TraverseBefore(ArrayExpression* node) { return true; }
        virtual void TraverseAfter(ArrayExpression* node) {}
        virtual bool TraverseBefore(ArrayPattern* node) { return true; }
        virtual void TraverseAfter(ArrayPattern* node) {}
        virtual bool TraverseBefore(ArrowFunctionExpression* node) { return true; }
        virtual void TraverseAfter(ArrowFunctionExpression* node) {}
        virtual bool TraverseBefore(AssignmentExpression* node) { return true; }
        virtual void TraverseAfter(AssignmentExpression* node) {}
        virtual bool TraverseBefore(AssignmentPattern* node) { return true; }
        virtual void TraverseAfter(AssignmentPattern* node) {}
        virtual bool TraverseBefore(AwaitExpression* node) { return true; }
        virtual void TraverseAfter(AwaitExpression* node) {}
        virtual bool TraverseBefore(BinaryExpression* node) { return true; }
        virtual void TraverseAfter(BinaryExpression* node) {}
        virtual bool TraverseBefore(BlockStatement* node) { return true; }
        virtual void TraverseAfter(BlockStatement* node) {}
        virtual bool TraverseBefore(BreakStatement* node) { return true; }
        virtual void TraverseAfter(BreakStatement* node) {}
        virtual bool TraverseBefore(CallExpression* node) { return true; }
        virtual void TraverseAfter(CallExpression* node) {}
        virtual bool TraverseBefore(CatchClause* node) { return true; }
        virtual void TraverseAfter(CatchClause* node) {}
        virtual bool TraverseBefore(ClassBody* node) { return true; }
        virtual void TraverseAfter(ClassBody* node) {}
        virtual bool TraverseBefore(ClassDeclaration* node) { return true; }
        virtual void TraverseAfter(ClassDeclaration* node) {}
        virtual bool TraverseBefore(ClassExpression* node) { return true; }
        virtual void TraverseAfter(ClassExpression* node) {}
        virtual bool TraverseBefore(ConditionalExpression* node) { return true; }
        virtual void TraverseAfter(ConditionalExpression* node) {}
        virtual bool TraverseBefore(ContinueStatement* node) { return true; }
        virtual void TraverseAfter(ContinueStatement* node) {}
        virtual bool TraverseBefore(DebuggerStatement* node) { return true; }
        virtual void TraverseAfter(DebuggerStatement* node) {}
        virtual bool TraverseBefore(Directive* node) { return true; }
        virtual void TraverseAfter(Directive* node) {}
        virtual bool TraverseBefore(DoWhileStatement* node) { return true; }
        virtual void TraverseAfter(DoWhileStatement* node) {}
        virtual bool TraverseBefore(EmptyStatement* node) { return true; }
        virtual void TraverseAfter(EmptyStatement* node) {}
        virtual bool TraverseBefore(ExportAllDeclaration* node) { return true; }
        virtual void TraverseAfter(ExportAllDeclaration* node) {}
        virtual bool TraverseBefore(ExportDefaultDeclaration* node) { return true; }
        virtual void TraverseAfter(ExportDefaultDeclaration* node) {}
        virtual bool TraverseBefore(ExportNamedDeclaration* node) { return true; }
        virtual void TraverseAfter(ExportNamedDeclaration* node) {}
        virtual bool TraverseBefore(ExportSpecifier* node) { return true; }
        virtual void TraverseAfter(ExportSpecifier* node) {}
        virtual bool TraverseBefore(ExpressionStatement* node) { return true; }
        virtual void TraverseAfter(ExpressionStatement* node) {}
        virtual bool TraverseBefore(ForInStatement* node) { return true; }
        virtual void TraverseAfter(ForInStatement* node) {}
        virtual bool TraverseBefore(ForOfStatement* node) { return true; }
        virtual void TraverseAfter(ForOfStatement* node) {}
        virtual bool TraverseBefore(ForStatement* node) { return true; }
        virtual void TraverseAfter(ForStatement* node) {}
        virtual bool TraverseBefore(FunctionDeclaration* node) { return true; }
        virtual void TraverseAfter(FunctionDeclaration* node) {}
        virtual bool TraverseBefore(FunctionExpression* node) { return true; }
        virtual void TraverseAfter(FunctionExpression* node) {}
        virtual bool TraverseBefore(Identifier* node) { return true; }
        virtual void TraverseAfter(Identifier* node) {}
        virtual bool TraverseBefore(IfStatement* node) { return true; }
        virtual void TraverseAfter(IfStatement* node) {}
        virtual bool TraverseBefore(Import* node) { return true; }
        virtual void TraverseAfter(Import* node) {}
        virtual bool TraverseBefore(ImportDeclaration* node) { return true; }
        virtual void TraverseAfter(ImportDeclaration* node) {}
        virtual bool TraverseBefore(ImportDefaultSpecifier* node) { return true; }
        virtual void TraverseAfter(ImportDefaultSpecifier* node) {}
        virtual bool TraverseBefore(ImportNamespaceSpecifier* node) { return true; }
        virtual void TraverseAfter(ImportNamespaceSpecifier* node) {}
        virtual bool TraverseBefore(ImportSpecifier* node) { return true; }
        virtual void TraverseAfter(ImportSpecifier* node) {}
        virtual bool TraverseBefore(LabeledStatement* node) { return true; }
        virtual void TraverseAfter(LabeledStatement* node) {}
        virtual bool TraverseBefore(Literal* node) { return true; }
        virtual void TraverseAfter(Literal* node) {}
        virtual bool TraverseBefore(MetaProperty* node) { return true; }
        virtual void TraverseAfter(MetaProperty* node) {}
        virtual bool TraverseBefore(MethodDefinition* node) { return true; }
        virtual void TraverseAfter(MethodDefinition* node) {}
        virtual bool TraverseBefore(Module* node) { return true; }
        virtual void TraverseAfter(Module* node) {}
        virtual bool TraverseBefore(NewExpression* node) { return true; }
        virtual void TraverseAfter(NewExpression* node) {}
        virtual bool TraverseBefore(ObjectExpression* node) { return true; }
        virtual void TraverseAfter(ObjectExpression* node) {}
        virtual bool TraverseBefore(ObjectPattern* node) { return true; }
        virtual void TraverseAfter(ObjectPattern* node) {}
        virtual bool TraverseBefore(Property* node) { return true; }
        virtual void TraverseAfter(Property* node) {}
        virtual bool TraverseBefore(RegexLiteral* node) { return true; }
        virtual void TraverseAfter(RegexLiteral* node) {}
        virtual bool TraverseBefore(RestElement* node) { return true; }
        virtual void TraverseAfter(RestElement* node) {}
        virtual bool TraverseBefore(ReturnStatement* node) { return true; }
        virtual void TraverseAfter(ReturnStatement* node) {}
        virtual bool TraverseBefore(Script* node) { return true; }
        virtual void TraverseAfter(Script* node) {}
        virtual bool TraverseBefore(SequenceExpression* node) { return true; }
        virtual void TraverseAfter(SequenceExpression* node) {}
        virtual bool TraverseBefore(SpreadElement* node) { return true; }
        virtual void TraverseAfter(SpreadElement* node) {}
        virtual bool TraverseBefore(MemberExpression* node) { return true; }
        virtual void TraverseAfter(MemberExpression* node) {}
        virtual bool TraverseBefore(Super* node) { return true; }
        virtual void TraverseAfter(Super* node) {}
        virtual bool TraverseBefore(SwitchCase* node) { return true; }
        virtual void TraverseAfter(SwitchCase* node) {}
        virtual bool TraverseBefore(SwitchStatement* node) { return true; }
        virtual void TraverseAfter(SwitchStatement* node) {}
        virtual bool TraverseBefore(TaggedTemplateExpression* node) { return true; }
        virtual void TraverseAfter(TaggedTemplateExpression* node) {}
        virtual bool TraverseBefore(TemplateElement* node) { return true; }
        virtual void TraverseAfter(TemplateElement* node) {}
        virtual bool TraverseBefore(TemplateLiteral* node) { return true; }
        virtual void TraverseAfter(TemplateLiteral* node) {}
        virtual bool TraverseBefore(ThisExpression* node) { return true; }
        virtual void TraverseAfter(ThisExpression* node) {}
        virtual bool TraverseBefore(ThrowStatement* node) { return true; }
        virtual void TraverseAfter(ThrowStatement* node) {}
        virtual bool TraverseBefore(TryStatement* node) { return true; }
        virtual void TraverseAfter(TryStatement* node) {}
        virtual bool TraverseBefore(UnaryExpression* node) { return true; }
        virtual void TraverseAfter(UnaryExpression* node) {}
        virtual bool TraverseBefore(UpdateExpression* node) { return true; }
        virtual void TraverseAfter(UpdateExpression* node) {}
        virtual bool TraverseBefore(VariableDeclaration* node) { return true; }
        virtual void TraverseAfter(VariableDeclaration* node) {}
        virtual bool TraverseBefore(VariableDeclarator* node) { return true; }
        virtual void TraverseAfter(VariableDeclarator* node) {}
        virtual bool TraverseBefore(WhileStatement* node) { return true; }
        virtual void TraverseAfter(WhileStatement* node) {}
        virtual bool TraverseBefore(WithStatement* node) { return true; }
        virtual void TraverseAfter(WithStatement* node) {}
        virtual bool TraverseBefore(YieldExpression* node) { return true; }
        virtual void TraverseAfter(YieldExpression* node) {}
        virtual bool TraverseBefore(ArrowParameterPlaceHolder* node) { return true; }
        virtual void TraverseAfter(ArrowParameterPlaceHolder* node) {}
        virtual bool TraverseBefore(JSXClosingElement* node) { return true; }
        virtual void TraverseAfter(JSXClosingElement* node) {}
        virtual bool TraverseBefore(JSXElement* node) { return true; }
        virtual void TraverseAfter(JSXElement* node) {}
        virtual bool TraverseBefore(JSXEmptyExpression* node) { return true; }
        virtual void TraverseAfter(JSXEmptyExpression* node) {}
        virtual bool TraverseBefore(JSXExpressionContainer* node) { return true; }
        virtual void TraverseAfter(JSXExpressionContainer* node) {}
        virtual bool TraverseBefore(JSXIdentifier* node) { return true; }
        virtual void TraverseAfter(JSXIdentifier* node) {}
        virtual bool TraverseBefore(JSXMemberExpression* node) { return true; }
        virtual void TraverseAfter(JSXMemberExpression* node) {}
        virtual bool TraverseBefore(JSXAttribute* node) { return true; }
        virtual void TraverseAfter(JSXAttribute* node) {}
        virtual bool TraverseBefore(JSXNamespacedName* node) { return true; }
        virtual void TraverseAfter(JSXNamespacedName* node) {}
        virtual bool TraverseBefore(JSXOpeningElement* node) { return true; }
        virtual void TraverseAfter(JSXOpeningElement* node) {}
        virtual bool TraverseBefore(JSXSpreadAttribute* node) { return true; }
        virtual void TraverseAfter(JSXSpreadAttribute* node) {}
        virtual bool TraverseBefore(JSXText* node) { return true; }
        virtual void TraverseAfter(JSXText* node) {}
        virtual bool TraverseBefore(TSParameterProperty* node) { return true; }
        virtual void TraverseAfter(TSParameterProperty* node) {}
        virtual bool TraverseBefore(TSDeclareFunction* node) { return true; }
        virtual void TraverseAfter(TSDeclareFunction* node) {}
        virtual bool TraverseBefore(TSDeclareMethod* node) { return true; }
        virtual void TraverseAfter(TSDeclareMethod* node) {}
        virtual bool TraverseBefore(TSQualifiedName* node) { return true; }
        virtual void TraverseAfter(TSQualifiedName* node) {}
        virtual bool TraverseBefore(TSCallSignatureDeclaration* node) { return true; }
        virtual void TraverseAfter(TSCallSignatureDeclaration* node) {}
        virtual bool TraverseBefore(TSConstructSignatureDeclaration* node) { return true; }
        virtual void TraverseAfter(TSConstructSignatureDeclaration* node) {}
        virtual bool TraverseBefore(TSPropertySignature* node) { return true; }
        virtual void TraverseAfter(TSPropertySignature* node) {}
        virtual bool TraverseBefore(TSMethodSignature* node) { return true; }
        virtual void TraverseAfter(TSMethodSignature* node) {}
        virtual bool TraverseBefore(TSIndexSignature* node) { return true; }
        virtual void TraverseAfter(TSIndexSignature* node) {}
        virtual bool TraverseBefore(TSAnyKeyword* node) { return true; }
        virtual void TraverseAfter(TSAnyKeyword* node) {}
        virtual bool TraverseBefore(TSBooleanKeyword* node) { return true; }
        virtual void TraverseAfter(TSBooleanKeyword* node) {}
        virtual bool TraverseBefore(TSBigIntKeyword* node) { return true; }
        virtual void TraverseAfter(TSBigIntKeyword* node) {}
        virtual bool TraverseBefore(TSNeverKeyword* node) { return true; }
        virtual void TraverseAfter(TSNeverKeyword* node) {}
        virtual bool TraverseBefore(TSNullKeyword* node) { return true; }
        virtual void TraverseAfter(TSNullKeyword* node) {}
        virtual bool TraverseBefore(TSNumberKeyword* node) { return true; }
        virtual void TraverseAfter(TSNumberKeyword* node) {}
        virtual bool TraverseBefore(TSObjectKeyword* node) { return true; }
        virtual void TraverseAfter(TSObjectKeyword* node) {}
        virtual bool TraverseBefore(TSStringKeyword* node) { return true; }
        virtual void TraverseAfter(TSStringKeyword* node) {}
        virtual bool TraverseBefore(TSSymbolKeyword* node) { return true; }
        virtual void TraverseAfter(TSSymbolKeyword* node) {}
        virtual bool TraverseBefore(TSUndefinedKeyword* node) { return true; }
        virtual void TraverseAfter(TSUndefinedKeyword* node) {}
        virtual bool TraverseBefore(TSUnknownKeyword* node) { return true; }
        virtual void TraverseAfter(TSUnknownKeyword* node) {}
        virtual bool TraverseBefore(TSVoidKeyword* node) { return true; }
        virtual void TraverseAfter(TSVoidKeyword* node) {}
        virtual bool TraverseBefore(TSThisType* node) { return true; }
        virtual void TraverseAfter(TSThisType* node) {}
        virtual bool TraverseBefore(TSFunctionType* node) { return true; }
        virtual void TraverseAfter(TSFunctionType* node) {}
        virtual bool TraverseBefore(TSConstructorType* node) { return true; }
        virtual void TraverseAfter(TSConstructorType* node) {}
        virtual bool TraverseBefore(TSTypeReference* node) { return true; }
        virtual void TraverseAfter(TSTypeReference* node) {}
        virtual bool TraverseBefore(TSTypePredicate* node) { return true; }
        virtual void TraverseAfter(TSTypePredicate* node) {}
        virtual bool TraverseBefore(TSTypeQuery* node) { return true; }
        virtual void TraverseAfter(TSTypeQuery* node) {}
        virtual bool TraverseBefore(TSTypeLiteral* node) { return true; }
        virtual void TraverseAfter(TSTypeLiteral* node) {}
        virtual bool TraverseBefore(TSArrayType* node) { return true; }
        virtual void TraverseAfter(TSArrayType* node) {}
        virtual bool TraverseBefore(TSTupleType* node) { return true; }
        virtual void TraverseAfter(TSTupleType* node) {}
        virtual bool TraverseBefore(TSOptionalType* node) { return true; }
        virtual void TraverseAfter(TSOptionalType* node) {}
        virtual bool TraverseBefore(TSRestType* node) { return true; }
        virtual void TraverseAfter(TSRestType* node) {}
        virtual bool TraverseBefore(TSUnionType* node) { return true; }
        virtual void TraverseAfter(TSUnionType* node) {}
        virtual bool TraverseBefore(TSIntersectionType* node) { return true; }
        virtual void TraverseAfter(TSIntersectionType* node) {}
        virtual bool TraverseBefore(TSConditionalType* node) { return true; }
        virtual void TraverseAfter(TSConditionalType* node) {}
        virtual bool TraverseBefore(TSInferType* node) { return true; }
        virtual void TraverseAfter(TSInferType* node) {}
        virtual bool TraverseBefore(TSParenthesizedType* node) { return true; }
        virtual void TraverseAfter(TSParenthesizedType* node) {}
        virtual bool TraverseBefore(TSTypeOperator* node) { return true; }
        virtual void TraverseAfter(TSTypeOperator* node) {}
        virtual bool TraverseBefore(TSIndexedAccessType* node) { return true; }
        virtual void TraverseAfter(TSIndexedAccessType* node) {}
        virtual bool TraverseBefore(TSMappedType* node) { return true; }
        virtual void TraverseAfter(TSMappedType* node) {}
        virtual bool TraverseBefore(TSLiteralType* node) { return true; }
        virtual void TraverseAfter(TSLiteralType* node) {}
        virtual bool TraverseBefore(TSExpressionWithTypeArguments* node) { return true; }
        virtual void TraverseAfter(TSExpressionWithTypeArguments* node) {}
        virtual bool TraverseBefore(TSInterfaceDeclaration* node) { return true; }
        virtual void TraverseAfter(TSInterfaceDeclaration* node) {}
        virtual bool TraverseBefore(TSInterfaceBody* node) { return true; }
        virtual void TraverseAfter(TSInterfaceBody* node) {}
        virtual bool TraverseBefore(TSTypeAliasDeclaration* node) { return true; }
        virtual void TraverseAfter(TSTypeAliasDeclaration* node) {}
        virtual bool TraverseBefore(TSAsExpression* node) { return true; }
        virtual void TraverseAfter(TSAsExpression* node) {}
        virtual bool TraverseBefore(TSTypeAssertion* node) { return true; }
        virtual void TraverseAfter(TSTypeAssertion* node) {}
        virtual bool TraverseBefore(TSEnumDeclaration* node) { return true; }
        virtual void TraverseAfter(TSEnumDeclaration* node) {}
        virtual bool TraverseBefore(TSEnumMember* node) { return true; }
        virtual void TraverseAfter(TSEnumMember* node) {}
        virtual bool TraverseBefore(TSModuleDeclaration* node) { return true; }
        virtual void TraverseAfter(TSModuleDeclaration* node) {}
        virtual bool TraverseBefore(TSModuleBlock* node) { return true; }
        virtual void TraverseAfter(TSModuleBlock* node) {}
        virtual bool TraverseBefore(TSImportType* node) { return true; }
        virtual void TraverseAfter(TSImportType* node) {}
        virtual bool TraverseBefore(TSImportEqualsDeclaration* node) { return true; }
        virtual void TraverseAfter(TSImportEqualsDeclaration* node) {}
        virtual bool TraverseBefore(TSExternalModuleReference* node) { return true; }
        virtual void TraverseAfter(TSExternalModuleReference* node) {}
        virtual bool TraverseBefore(TSNonNullExpression* node) { return true; }
        virtual void TraverseAfter(TSNonNullExpression* node) {}
        virtual bool TraverseBefore(TSExportAssignment* node) { return true; }
        virtual void TraverseAfter(TSExportAssignment* node) {}
        virtual bool TraverseBefore(TSNamespaceExportDeclaration* node) { return true; }
        virtual void TraverseAfter(TSNamespaceExportDeclaration* node) {}
        virtual bool TraverseBefore(TSTypeAnnotation* node) { return true; }
        virtual void TraverseAfter(TSTypeAnnotation* node) {}
        virtual bool TraverseBefore(TSTypeParameterInstantiation* node) { return true; }
        virtual void TraverseAfter(TSTypeParameterInstantiation* node) {}
        virtual bool TraverseBefore(TSTypeParameterDeclaration* node) { return true; }
        virtual void TraverseAfter(TSTypeParameterDeclaration* node) {}
        virtual bool TraverseBefore(TSTypeParameter* node) { return true; }
        virtual void TraverseAfter(TSTypeParameter* node) {}

        virtual ~AutoNodeTraverser() = default;

    };


}
