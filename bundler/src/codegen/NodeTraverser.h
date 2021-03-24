/* Generated by Ruby Script! */


#pragma once
#include "parser/SyntaxNodes.h"

namespace jetpack {

    class AutoNodeTraverser {
    public:

        AutoNodeTraverser() = default;

        void TraverseNode(const Sp<SyntaxNode>& node);

        virtual bool TraverseBefore(const Sp<ArrayExpression>& node) { return true; }
        virtual void TraverseAfter(const Sp<ArrayExpression>& node) {}
        virtual bool TraverseBefore(const Sp<ArrayPattern>& node) { return true; }
        virtual void TraverseAfter(const Sp<ArrayPattern>& node) {}
        virtual bool TraverseBefore(const Sp<ArrowFunctionExpression>& node) { return true; }
        virtual void TraverseAfter(const Sp<ArrowFunctionExpression>& node) {}
        virtual bool TraverseBefore(const Sp<AssignmentExpression>& node) { return true; }
        virtual void TraverseAfter(const Sp<AssignmentExpression>& node) {}
        virtual bool TraverseBefore(const Sp<AssignmentPattern>& node) { return true; }
        virtual void TraverseAfter(const Sp<AssignmentPattern>& node) {}
        virtual bool TraverseBefore(const Sp<AwaitExpression>& node) { return true; }
        virtual void TraverseAfter(const Sp<AwaitExpression>& node) {}
        virtual bool TraverseBefore(const Sp<BinaryExpression>& node) { return true; }
        virtual void TraverseAfter(const Sp<BinaryExpression>& node) {}
        virtual bool TraverseBefore(const Sp<BlockStatement>& node) { return true; }
        virtual void TraverseAfter(const Sp<BlockStatement>& node) {}
        virtual bool TraverseBefore(const Sp<BreakStatement>& node) { return true; }
        virtual void TraverseAfter(const Sp<BreakStatement>& node) {}
        virtual bool TraverseBefore(const Sp<CallExpression>& node) { return true; }
        virtual void TraverseAfter(const Sp<CallExpression>& node) {}
        virtual bool TraverseBefore(const Sp<CatchClause>& node) { return true; }
        virtual void TraverseAfter(const Sp<CatchClause>& node) {}
        virtual bool TraverseBefore(const Sp<ClassBody>& node) { return true; }
        virtual void TraverseAfter(const Sp<ClassBody>& node) {}
        virtual bool TraverseBefore(const Sp<ClassDeclaration>& node) { return true; }
        virtual void TraverseAfter(const Sp<ClassDeclaration>& node) {}
        virtual bool TraverseBefore(const Sp<ClassExpression>& node) { return true; }
        virtual void TraverseAfter(const Sp<ClassExpression>& node) {}
        virtual bool TraverseBefore(const Sp<ConditionalExpression>& node) { return true; }
        virtual void TraverseAfter(const Sp<ConditionalExpression>& node) {}
        virtual bool TraverseBefore(const Sp<ContinueStatement>& node) { return true; }
        virtual void TraverseAfter(const Sp<ContinueStatement>& node) {}
        virtual bool TraverseBefore(const Sp<DebuggerStatement>& node) { return true; }
        virtual void TraverseAfter(const Sp<DebuggerStatement>& node) {}
        virtual bool TraverseBefore(const Sp<Directive>& node) { return true; }
        virtual void TraverseAfter(const Sp<Directive>& node) {}
        virtual bool TraverseBefore(const Sp<DoWhileStatement>& node) { return true; }
        virtual void TraverseAfter(const Sp<DoWhileStatement>& node) {}
        virtual bool TraverseBefore(const Sp<EmptyStatement>& node) { return true; }
        virtual void TraverseAfter(const Sp<EmptyStatement>& node) {}
        virtual bool TraverseBefore(const Sp<ExportAllDeclaration>& node) { return true; }
        virtual void TraverseAfter(const Sp<ExportAllDeclaration>& node) {}
        virtual bool TraverseBefore(const Sp<ExportDefaultDeclaration>& node) { return true; }
        virtual void TraverseAfter(const Sp<ExportDefaultDeclaration>& node) {}
        virtual bool TraverseBefore(const Sp<ExportNamedDeclaration>& node) { return true; }
        virtual void TraverseAfter(const Sp<ExportNamedDeclaration>& node) {}
        virtual bool TraverseBefore(const Sp<ExportSpecifier>& node) { return true; }
        virtual void TraverseAfter(const Sp<ExportSpecifier>& node) {}
        virtual bool TraverseBefore(const Sp<ExpressionStatement>& node) { return true; }
        virtual void TraverseAfter(const Sp<ExpressionStatement>& node) {}
        virtual bool TraverseBefore(const Sp<ForInStatement>& node) { return true; }
        virtual void TraverseAfter(const Sp<ForInStatement>& node) {}
        virtual bool TraverseBefore(const Sp<ForOfStatement>& node) { return true; }
        virtual void TraverseAfter(const Sp<ForOfStatement>& node) {}
        virtual bool TraverseBefore(const Sp<ForStatement>& node) { return true; }
        virtual void TraverseAfter(const Sp<ForStatement>& node) {}
        virtual bool TraverseBefore(const Sp<FunctionDeclaration>& node) { return true; }
        virtual void TraverseAfter(const Sp<FunctionDeclaration>& node) {}
        virtual bool TraverseBefore(const Sp<FunctionExpression>& node) { return true; }
        virtual void TraverseAfter(const Sp<FunctionExpression>& node) {}
        virtual bool TraverseBefore(const Sp<Identifier>& node) { return true; }
        virtual void TraverseAfter(const Sp<Identifier>& node) {}
        virtual bool TraverseBefore(const Sp<IfStatement>& node) { return true; }
        virtual void TraverseAfter(const Sp<IfStatement>& node) {}
        virtual bool TraverseBefore(const Sp<Import>& node) { return true; }
        virtual void TraverseAfter(const Sp<Import>& node) {}
        virtual bool TraverseBefore(const Sp<ImportDeclaration>& node) { return true; }
        virtual void TraverseAfter(const Sp<ImportDeclaration>& node) {}
        virtual bool TraverseBefore(const Sp<ImportDefaultSpecifier>& node) { return true; }
        virtual void TraverseAfter(const Sp<ImportDefaultSpecifier>& node) {}
        virtual bool TraverseBefore(const Sp<ImportNamespaceSpecifier>& node) { return true; }
        virtual void TraverseAfter(const Sp<ImportNamespaceSpecifier>& node) {}
        virtual bool TraverseBefore(const Sp<ImportSpecifier>& node) { return true; }
        virtual void TraverseAfter(const Sp<ImportSpecifier>& node) {}
        virtual bool TraverseBefore(const Sp<LabeledStatement>& node) { return true; }
        virtual void TraverseAfter(const Sp<LabeledStatement>& node) {}
        virtual bool TraverseBefore(const Sp<Literal>& node) { return true; }
        virtual void TraverseAfter(const Sp<Literal>& node) {}
        virtual bool TraverseBefore(const Sp<MetaProperty>& node) { return true; }
        virtual void TraverseAfter(const Sp<MetaProperty>& node) {}
        virtual bool TraverseBefore(const Sp<MethodDefinition>& node) { return true; }
        virtual void TraverseAfter(const Sp<MethodDefinition>& node) {}
        virtual bool TraverseBefore(const Sp<Module>& node) { return true; }
        virtual void TraverseAfter(const Sp<Module>& node) {}
        virtual bool TraverseBefore(const Sp<NewExpression>& node) { return true; }
        virtual void TraverseAfter(const Sp<NewExpression>& node) {}
        virtual bool TraverseBefore(const Sp<ObjectExpression>& node) { return true; }
        virtual void TraverseAfter(const Sp<ObjectExpression>& node) {}
        virtual bool TraverseBefore(const Sp<ObjectPattern>& node) { return true; }
        virtual void TraverseAfter(const Sp<ObjectPattern>& node) {}
        virtual bool TraverseBefore(const Sp<Property>& node) { return true; }
        virtual void TraverseAfter(const Sp<Property>& node) {}
        virtual bool TraverseBefore(const Sp<RegexLiteral>& node) { return true; }
        virtual void TraverseAfter(const Sp<RegexLiteral>& node) {}
        virtual bool TraverseBefore(const Sp<RestElement>& node) { return true; }
        virtual void TraverseAfter(const Sp<RestElement>& node) {}
        virtual bool TraverseBefore(const Sp<ReturnStatement>& node) { return true; }
        virtual void TraverseAfter(const Sp<ReturnStatement>& node) {}
        virtual bool TraverseBefore(const Sp<Script>& node) { return true; }
        virtual void TraverseAfter(const Sp<Script>& node) {}
        virtual bool TraverseBefore(const Sp<SequenceExpression>& node) { return true; }
        virtual void TraverseAfter(const Sp<SequenceExpression>& node) {}
        virtual bool TraverseBefore(const Sp<SpreadElement>& node) { return true; }
        virtual void TraverseAfter(const Sp<SpreadElement>& node) {}
        virtual bool TraverseBefore(const Sp<MemberExpression>& node) { return true; }
        virtual void TraverseAfter(const Sp<MemberExpression>& node) {}
        virtual bool TraverseBefore(const Sp<Super>& node) { return true; }
        virtual void TraverseAfter(const Sp<Super>& node) {}
        virtual bool TraverseBefore(const Sp<SwitchCase>& node) { return true; }
        virtual void TraverseAfter(const Sp<SwitchCase>& node) {}
        virtual bool TraverseBefore(const Sp<SwitchStatement>& node) { return true; }
        virtual void TraverseAfter(const Sp<SwitchStatement>& node) {}
        virtual bool TraverseBefore(const Sp<TaggedTemplateExpression>& node) { return true; }
        virtual void TraverseAfter(const Sp<TaggedTemplateExpression>& node) {}
        virtual bool TraverseBefore(const Sp<TemplateElement>& node) { return true; }
        virtual void TraverseAfter(const Sp<TemplateElement>& node) {}
        virtual bool TraverseBefore(const Sp<TemplateLiteral>& node) { return true; }
        virtual void TraverseAfter(const Sp<TemplateLiteral>& node) {}
        virtual bool TraverseBefore(const Sp<ThisExpression>& node) { return true; }
        virtual void TraverseAfter(const Sp<ThisExpression>& node) {}
        virtual bool TraverseBefore(const Sp<ThrowStatement>& node) { return true; }
        virtual void TraverseAfter(const Sp<ThrowStatement>& node) {}
        virtual bool TraverseBefore(const Sp<TryStatement>& node) { return true; }
        virtual void TraverseAfter(const Sp<TryStatement>& node) {}
        virtual bool TraverseBefore(const Sp<UnaryExpression>& node) { return true; }
        virtual void TraverseAfter(const Sp<UnaryExpression>& node) {}
        virtual bool TraverseBefore(const Sp<UpdateExpression>& node) { return true; }
        virtual void TraverseAfter(const Sp<UpdateExpression>& node) {}
        virtual bool TraverseBefore(const Sp<VariableDeclaration>& node) { return true; }
        virtual void TraverseAfter(const Sp<VariableDeclaration>& node) {}
        virtual bool TraverseBefore(const Sp<VariableDeclarator>& node) { return true; }
        virtual void TraverseAfter(const Sp<VariableDeclarator>& node) {}
        virtual bool TraverseBefore(const Sp<WhileStatement>& node) { return true; }
        virtual void TraverseAfter(const Sp<WhileStatement>& node) {}
        virtual bool TraverseBefore(const Sp<WithStatement>& node) { return true; }
        virtual void TraverseAfter(const Sp<WithStatement>& node) {}
        virtual bool TraverseBefore(const Sp<YieldExpression>& node) { return true; }
        virtual void TraverseAfter(const Sp<YieldExpression>& node) {}
        virtual bool TraverseBefore(const Sp<ArrowParameterPlaceHolder>& node) { return true; }
        virtual void TraverseAfter(const Sp<ArrowParameterPlaceHolder>& node) {}
        virtual bool TraverseBefore(const Sp<JSXClosingElement>& node) { return true; }
        virtual void TraverseAfter(const Sp<JSXClosingElement>& node) {}
        virtual bool TraverseBefore(const Sp<JSXElement>& node) { return true; }
        virtual void TraverseAfter(const Sp<JSXElement>& node) {}
        virtual bool TraverseBefore(const Sp<JSXEmptyExpression>& node) { return true; }
        virtual void TraverseAfter(const Sp<JSXEmptyExpression>& node) {}
        virtual bool TraverseBefore(const Sp<JSXExpressionContainer>& node) { return true; }
        virtual void TraverseAfter(const Sp<JSXExpressionContainer>& node) {}
        virtual bool TraverseBefore(const Sp<JSXIdentifier>& node) { return true; }
        virtual void TraverseAfter(const Sp<JSXIdentifier>& node) {}
        virtual bool TraverseBefore(const Sp<JSXMemberExpression>& node) { return true; }
        virtual void TraverseAfter(const Sp<JSXMemberExpression>& node) {}
        virtual bool TraverseBefore(const Sp<JSXAttribute>& node) { return true; }
        virtual void TraverseAfter(const Sp<JSXAttribute>& node) {}
        virtual bool TraverseBefore(const Sp<JSXNamespacedName>& node) { return true; }
        virtual void TraverseAfter(const Sp<JSXNamespacedName>& node) {}
        virtual bool TraverseBefore(const Sp<JSXOpeningElement>& node) { return true; }
        virtual void TraverseAfter(const Sp<JSXOpeningElement>& node) {}
        virtual bool TraverseBefore(const Sp<JSXSpreadAttribute>& node) { return true; }
        virtual void TraverseAfter(const Sp<JSXSpreadAttribute>& node) {}
        virtual bool TraverseBefore(const Sp<JSXText>& node) { return true; }
        virtual void TraverseAfter(const Sp<JSXText>& node) {}
        virtual bool TraverseBefore(const Sp<TSParameterProperty>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSParameterProperty>& node) {}
        virtual bool TraverseBefore(const Sp<TSDeclareFunction>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSDeclareFunction>& node) {}
        virtual bool TraverseBefore(const Sp<TSDeclareMethod>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSDeclareMethod>& node) {}
        virtual bool TraverseBefore(const Sp<TSQualifiedName>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSQualifiedName>& node) {}
        virtual bool TraverseBefore(const Sp<TSCallSignatureDeclaration>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSCallSignatureDeclaration>& node) {}
        virtual bool TraverseBefore(const Sp<TSConstructSignatureDeclaration>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSConstructSignatureDeclaration>& node) {}
        virtual bool TraverseBefore(const Sp<TSPropertySignature>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSPropertySignature>& node) {}
        virtual bool TraverseBefore(const Sp<TSMethodSignature>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSMethodSignature>& node) {}
        virtual bool TraverseBefore(const Sp<TSIndexSignature>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSIndexSignature>& node) {}
        virtual bool TraverseBefore(const Sp<TSAnyKeyword>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSAnyKeyword>& node) {}
        virtual bool TraverseBefore(const Sp<TSBooleanKeyword>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSBooleanKeyword>& node) {}
        virtual bool TraverseBefore(const Sp<TSBigIntKeyword>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSBigIntKeyword>& node) {}
        virtual bool TraverseBefore(const Sp<TSNeverKeyword>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSNeverKeyword>& node) {}
        virtual bool TraverseBefore(const Sp<TSNullKeyword>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSNullKeyword>& node) {}
        virtual bool TraverseBefore(const Sp<TSNumberKeyword>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSNumberKeyword>& node) {}
        virtual bool TraverseBefore(const Sp<TSObjectKeyword>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSObjectKeyword>& node) {}
        virtual bool TraverseBefore(const Sp<TSStringKeyword>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSStringKeyword>& node) {}
        virtual bool TraverseBefore(const Sp<TSSymbolKeyword>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSSymbolKeyword>& node) {}
        virtual bool TraverseBefore(const Sp<TSUndefinedKeyword>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSUndefinedKeyword>& node) {}
        virtual bool TraverseBefore(const Sp<TSUnknownKeyword>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSUnknownKeyword>& node) {}
        virtual bool TraverseBefore(const Sp<TSVoidKeyword>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSVoidKeyword>& node) {}
        virtual bool TraverseBefore(const Sp<TSThisType>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSThisType>& node) {}
        virtual bool TraverseBefore(const Sp<TSFunctionType>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSFunctionType>& node) {}
        virtual bool TraverseBefore(const Sp<TSConstructorType>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSConstructorType>& node) {}
        virtual bool TraverseBefore(const Sp<TSTypeReference>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSTypeReference>& node) {}
        virtual bool TraverseBefore(const Sp<TSTypePredicate>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSTypePredicate>& node) {}
        virtual bool TraverseBefore(const Sp<TSTypeQuery>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSTypeQuery>& node) {}
        virtual bool TraverseBefore(const Sp<TSTypeLiteral>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSTypeLiteral>& node) {}
        virtual bool TraverseBefore(const Sp<TSArrayType>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSArrayType>& node) {}
        virtual bool TraverseBefore(const Sp<TSTupleType>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSTupleType>& node) {}
        virtual bool TraverseBefore(const Sp<TSOptionalType>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSOptionalType>& node) {}
        virtual bool TraverseBefore(const Sp<TSRestType>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSRestType>& node) {}
        virtual bool TraverseBefore(const Sp<TSUnionType>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSUnionType>& node) {}
        virtual bool TraverseBefore(const Sp<TSIntersectionType>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSIntersectionType>& node) {}
        virtual bool TraverseBefore(const Sp<TSConditionalType>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSConditionalType>& node) {}
        virtual bool TraverseBefore(const Sp<TSInferType>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSInferType>& node) {}
        virtual bool TraverseBefore(const Sp<TSParenthesizedType>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSParenthesizedType>& node) {}
        virtual bool TraverseBefore(const Sp<TSTypeOperator>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSTypeOperator>& node) {}
        virtual bool TraverseBefore(const Sp<TSIndexedAccessType>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSIndexedAccessType>& node) {}
        virtual bool TraverseBefore(const Sp<TSMappedType>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSMappedType>& node) {}
        virtual bool TraverseBefore(const Sp<TSLiteralType>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSLiteralType>& node) {}
        virtual bool TraverseBefore(const Sp<TSExpressionWithTypeArguments>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSExpressionWithTypeArguments>& node) {}
        virtual bool TraverseBefore(const Sp<TSInterfaceDeclaration>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSInterfaceDeclaration>& node) {}
        virtual bool TraverseBefore(const Sp<TSInterfaceBody>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSInterfaceBody>& node) {}
        virtual bool TraverseBefore(const Sp<TSTypeAliasDeclaration>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSTypeAliasDeclaration>& node) {}
        virtual bool TraverseBefore(const Sp<TSAsExpression>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSAsExpression>& node) {}
        virtual bool TraverseBefore(const Sp<TSTypeAssertion>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSTypeAssertion>& node) {}
        virtual bool TraverseBefore(const Sp<TSEnumDeclaration>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSEnumDeclaration>& node) {}
        virtual bool TraverseBefore(const Sp<TSEnumMember>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSEnumMember>& node) {}
        virtual bool TraverseBefore(const Sp<TSModuleDeclaration>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSModuleDeclaration>& node) {}
        virtual bool TraverseBefore(const Sp<TSModuleBlock>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSModuleBlock>& node) {}
        virtual bool TraverseBefore(const Sp<TSImportType>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSImportType>& node) {}
        virtual bool TraverseBefore(const Sp<TSImportEqualsDeclaration>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSImportEqualsDeclaration>& node) {}
        virtual bool TraverseBefore(const Sp<TSExternalModuleReference>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSExternalModuleReference>& node) {}
        virtual bool TraverseBefore(const Sp<TSNonNullExpression>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSNonNullExpression>& node) {}
        virtual bool TraverseBefore(const Sp<TSExportAssignment>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSExportAssignment>& node) {}
        virtual bool TraverseBefore(const Sp<TSNamespaceExportDeclaration>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSNamespaceExportDeclaration>& node) {}
        virtual bool TraverseBefore(const Sp<TSTypeAnnotation>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSTypeAnnotation>& node) {}
        virtual bool TraverseBefore(const Sp<TSTypeParameterInstantiation>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSTypeParameterInstantiation>& node) {}
        virtual bool TraverseBefore(const Sp<TSTypeParameterDeclaration>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSTypeParameterDeclaration>& node) {}
        virtual bool TraverseBefore(const Sp<TSTypeParameter>& node) { return true; }
        virtual void TraverseAfter(const Sp<TSTypeParameter>& node) {}

        virtual ~AutoNodeTraverser() = default;

    };


    class NodeTraverser {
    public:

        NodeTraverser() = default;

        void TraverseNode(const Sp<SyntaxNode>& node);

        virtual void Traverse(const Sp<ArrayExpression>& node) {}
        virtual void Traverse(const Sp<ArrayPattern>& node) {}
        virtual void Traverse(const Sp<ArrowFunctionExpression>& node) {}
        virtual void Traverse(const Sp<AssignmentExpression>& node) {}
        virtual void Traverse(const Sp<AssignmentPattern>& node) {}
        virtual void Traverse(const Sp<AwaitExpression>& node) {}
        virtual void Traverse(const Sp<BinaryExpression>& node) {}
        virtual void Traverse(const Sp<BlockStatement>& node) {}
        virtual void Traverse(const Sp<BreakStatement>& node) {}
        virtual void Traverse(const Sp<CallExpression>& node) {}
        virtual void Traverse(const Sp<CatchClause>& node) {}
        virtual void Traverse(const Sp<ClassBody>& node) {}
        virtual void Traverse(const Sp<ClassDeclaration>& node) {}
        virtual void Traverse(const Sp<ClassExpression>& node) {}
        virtual void Traverse(const Sp<ConditionalExpression>& node) {}
        virtual void Traverse(const Sp<ContinueStatement>& node) {}
        virtual void Traverse(const Sp<DebuggerStatement>& node) {}
        virtual void Traverse(const Sp<Directive>& node) {}
        virtual void Traverse(const Sp<DoWhileStatement>& node) {}
        virtual void Traverse(const Sp<EmptyStatement>& node) {}
        virtual void Traverse(const Sp<ExportAllDeclaration>& node) {}
        virtual void Traverse(const Sp<ExportDefaultDeclaration>& node) {}
        virtual void Traverse(const Sp<ExportNamedDeclaration>& node) {}
        virtual void Traverse(const Sp<ExportSpecifier>& node) {}
        virtual void Traverse(const Sp<ExpressionStatement>& node) {}
        virtual void Traverse(const Sp<ForInStatement>& node) {}
        virtual void Traverse(const Sp<ForOfStatement>& node) {}
        virtual void Traverse(const Sp<ForStatement>& node) {}
        virtual void Traverse(const Sp<FunctionDeclaration>& node) {}
        virtual void Traverse(const Sp<FunctionExpression>& node) {}
        virtual void Traverse(const Sp<Identifier>& node) {}
        virtual void Traverse(const Sp<IfStatement>& node) {}
        virtual void Traverse(const Sp<Import>& node) {}
        virtual void Traverse(const Sp<ImportDeclaration>& node) {}
        virtual void Traverse(const Sp<ImportDefaultSpecifier>& node) {}
        virtual void Traverse(const Sp<ImportNamespaceSpecifier>& node) {}
        virtual void Traverse(const Sp<ImportSpecifier>& node) {}
        virtual void Traverse(const Sp<LabeledStatement>& node) {}
        virtual void Traverse(const Sp<Literal>& node) {}
        virtual void Traverse(const Sp<MetaProperty>& node) {}
        virtual void Traverse(const Sp<MethodDefinition>& node) {}
        virtual void Traverse(const Sp<Module>& node) {}
        virtual void Traverse(const Sp<NewExpression>& node) {}
        virtual void Traverse(const Sp<ObjectExpression>& node) {}
        virtual void Traverse(const Sp<ObjectPattern>& node) {}
        virtual void Traverse(const Sp<Property>& node) {}
        virtual void Traverse(const Sp<RegexLiteral>& node) {}
        virtual void Traverse(const Sp<RestElement>& node) {}
        virtual void Traverse(const Sp<ReturnStatement>& node) {}
        virtual void Traverse(const Sp<Script>& node) {}
        virtual void Traverse(const Sp<SequenceExpression>& node) {}
        virtual void Traverse(const Sp<SpreadElement>& node) {}
        virtual void Traverse(const Sp<MemberExpression>& node) {}
        virtual void Traverse(const Sp<Super>& node) {}
        virtual void Traverse(const Sp<SwitchCase>& node) {}
        virtual void Traverse(const Sp<SwitchStatement>& node) {}
        virtual void Traverse(const Sp<TaggedTemplateExpression>& node) {}
        virtual void Traverse(const Sp<TemplateElement>& node) {}
        virtual void Traverse(const Sp<TemplateLiteral>& node) {}
        virtual void Traverse(const Sp<ThisExpression>& node) {}
        virtual void Traverse(const Sp<ThrowStatement>& node) {}
        virtual void Traverse(const Sp<TryStatement>& node) {}
        virtual void Traverse(const Sp<UnaryExpression>& node) {}
        virtual void Traverse(const Sp<UpdateExpression>& node) {}
        virtual void Traverse(const Sp<VariableDeclaration>& node) {}
        virtual void Traverse(const Sp<VariableDeclarator>& node) {}
        virtual void Traverse(const Sp<WhileStatement>& node) {}
        virtual void Traverse(const Sp<WithStatement>& node) {}
        virtual void Traverse(const Sp<YieldExpression>& node) {}
        virtual void Traverse(const Sp<ArrowParameterPlaceHolder>& node) {}
        virtual void Traverse(const Sp<JSXClosingElement>& node) {}
        virtual void Traverse(const Sp<JSXElement>& node) {}
        virtual void Traverse(const Sp<JSXEmptyExpression>& node) {}
        virtual void Traverse(const Sp<JSXExpressionContainer>& node) {}
        virtual void Traverse(const Sp<JSXIdentifier>& node) {}
        virtual void Traverse(const Sp<JSXMemberExpression>& node) {}
        virtual void Traverse(const Sp<JSXAttribute>& node) {}
        virtual void Traverse(const Sp<JSXNamespacedName>& node) {}
        virtual void Traverse(const Sp<JSXOpeningElement>& node) {}
        virtual void Traverse(const Sp<JSXSpreadAttribute>& node) {}
        virtual void Traverse(const Sp<JSXText>& node) {}
        virtual void Traverse(const Sp<TSParameterProperty>& node) {}
        virtual void Traverse(const Sp<TSDeclareFunction>& node) {}
        virtual void Traverse(const Sp<TSDeclareMethod>& node) {}
        virtual void Traverse(const Sp<TSQualifiedName>& node) {}
        virtual void Traverse(const Sp<TSCallSignatureDeclaration>& node) {}
        virtual void Traverse(const Sp<TSConstructSignatureDeclaration>& node) {}
        virtual void Traverse(const Sp<TSPropertySignature>& node) {}
        virtual void Traverse(const Sp<TSMethodSignature>& node) {}
        virtual void Traverse(const Sp<TSIndexSignature>& node) {}
        virtual void Traverse(const Sp<TSAnyKeyword>& node) {}
        virtual void Traverse(const Sp<TSBooleanKeyword>& node) {}
        virtual void Traverse(const Sp<TSBigIntKeyword>& node) {}
        virtual void Traverse(const Sp<TSNeverKeyword>& node) {}
        virtual void Traverse(const Sp<TSNullKeyword>& node) {}
        virtual void Traverse(const Sp<TSNumberKeyword>& node) {}
        virtual void Traverse(const Sp<TSObjectKeyword>& node) {}
        virtual void Traverse(const Sp<TSStringKeyword>& node) {}
        virtual void Traverse(const Sp<TSSymbolKeyword>& node) {}
        virtual void Traverse(const Sp<TSUndefinedKeyword>& node) {}
        virtual void Traverse(const Sp<TSUnknownKeyword>& node) {}
        virtual void Traverse(const Sp<TSVoidKeyword>& node) {}
        virtual void Traverse(const Sp<TSThisType>& node) {}
        virtual void Traverse(const Sp<TSFunctionType>& node) {}
        virtual void Traverse(const Sp<TSConstructorType>& node) {}
        virtual void Traverse(const Sp<TSTypeReference>& node) {}
        virtual void Traverse(const Sp<TSTypePredicate>& node) {}
        virtual void Traverse(const Sp<TSTypeQuery>& node) {}
        virtual void Traverse(const Sp<TSTypeLiteral>& node) {}
        virtual void Traverse(const Sp<TSArrayType>& node) {}
        virtual void Traverse(const Sp<TSTupleType>& node) {}
        virtual void Traverse(const Sp<TSOptionalType>& node) {}
        virtual void Traverse(const Sp<TSRestType>& node) {}
        virtual void Traverse(const Sp<TSUnionType>& node) {}
        virtual void Traverse(const Sp<TSIntersectionType>& node) {}
        virtual void Traverse(const Sp<TSConditionalType>& node) {}
        virtual void Traverse(const Sp<TSInferType>& node) {}
        virtual void Traverse(const Sp<TSParenthesizedType>& node) {}
        virtual void Traverse(const Sp<TSTypeOperator>& node) {}
        virtual void Traverse(const Sp<TSIndexedAccessType>& node) {}
        virtual void Traverse(const Sp<TSMappedType>& node) {}
        virtual void Traverse(const Sp<TSLiteralType>& node) {}
        virtual void Traverse(const Sp<TSExpressionWithTypeArguments>& node) {}
        virtual void Traverse(const Sp<TSInterfaceDeclaration>& node) {}
        virtual void Traverse(const Sp<TSInterfaceBody>& node) {}
        virtual void Traverse(const Sp<TSTypeAliasDeclaration>& node) {}
        virtual void Traverse(const Sp<TSAsExpression>& node) {}
        virtual void Traverse(const Sp<TSTypeAssertion>& node) {}
        virtual void Traverse(const Sp<TSEnumDeclaration>& node) {}
        virtual void Traverse(const Sp<TSEnumMember>& node) {}
        virtual void Traverse(const Sp<TSModuleDeclaration>& node) {}
        virtual void Traverse(const Sp<TSModuleBlock>& node) {}
        virtual void Traverse(const Sp<TSImportType>& node) {}
        virtual void Traverse(const Sp<TSImportEqualsDeclaration>& node) {}
        virtual void Traverse(const Sp<TSExternalModuleReference>& node) {}
        virtual void Traverse(const Sp<TSNonNullExpression>& node) {}
        virtual void Traverse(const Sp<TSExportAssignment>& node) {}
        virtual void Traverse(const Sp<TSNamespaceExportDeclaration>& node) {}
        virtual void Traverse(const Sp<TSTypeAnnotation>& node) {}
        virtual void Traverse(const Sp<TSTypeParameterInstantiation>& node) {}
        virtual void Traverse(const Sp<TSTypeParameterDeclaration>& node) {}
        virtual void Traverse(const Sp<TSTypeParameter>& node) {}

        virtual ~NodeTraverser() = default;

    };

}

