
#include "NodeTraverser.h"

namespace jetpack {

    void NodeTraverser::TraverseNode(SyntaxNode& node) {
        switch (node.type) {

            case SyntaxNodeType::ArrayExpression: {
                this->Traverse(*dynamic_cast<ArrayExpression*>(&node));
                break;
            }

            case SyntaxNodeType::ArrayPattern: {
                this->Traverse(*dynamic_cast<ArrayPattern*>(&node));
                break;
            }

            case SyntaxNodeType::ArrowFunctionExpression: {
                this->Traverse(*dynamic_cast<ArrowFunctionExpression*>(&node));
                break;
            }

            case SyntaxNodeType::AssignmentExpression: {
                this->Traverse(*dynamic_cast<AssignmentExpression*>(&node));
                break;
            }

            case SyntaxNodeType::AssignmentPattern: {
                this->Traverse(*dynamic_cast<AssignmentPattern*>(&node));
                break;
            }

            case SyntaxNodeType::AwaitExpression: {
                this->Traverse(*dynamic_cast<AwaitExpression*>(&node));
                break;
            }

            case SyntaxNodeType::BinaryExpression: {
                this->Traverse(*dynamic_cast<BinaryExpression*>(&node));
                break;
            }

            case SyntaxNodeType::BlockStatement: {
                this->Traverse(*dynamic_cast<BlockStatement*>(&node));
                break;
            }

            case SyntaxNodeType::BreakStatement: {
                this->Traverse(*dynamic_cast<BreakStatement*>(&node));
                break;
            }

            case SyntaxNodeType::CallExpression: {
                this->Traverse(*dynamic_cast<CallExpression*>(&node));
                break;
            }

            case SyntaxNodeType::CatchClause: {
                this->Traverse(*dynamic_cast<CatchClause*>(&node));
                break;
            }

            case SyntaxNodeType::ClassBody: {
                this->Traverse(*dynamic_cast<ClassBody*>(&node));
                break;
            }

            case SyntaxNodeType::ClassDeclaration: {
                this->Traverse(*dynamic_cast<ClassDeclaration*>(&node));
                break;
            }

            case SyntaxNodeType::ClassExpression: {
                this->Traverse(*dynamic_cast<ClassExpression*>(&node));
                break;
            }

            case SyntaxNodeType::ConditionalExpression: {
                this->Traverse(*dynamic_cast<ConditionalExpression*>(&node));
                break;
            }

            case SyntaxNodeType::ContinueStatement: {
                this->Traverse(*dynamic_cast<ContinueStatement*>(&node));
                break;
            }

            case SyntaxNodeType::DebuggerStatement: {
                this->Traverse(*dynamic_cast<DebuggerStatement*>(&node));
                break;
            }

            case SyntaxNodeType::Directive: {
                this->Traverse(*dynamic_cast<Directive*>(&node));
                break;
            }

            case SyntaxNodeType::DoWhileStatement: {
                this->Traverse(*dynamic_cast<DoWhileStatement*>(&node));
                break;
            }

            case SyntaxNodeType::EmptyStatement: {
                this->Traverse(*dynamic_cast<EmptyStatement*>(&node));
                break;
            }

            case SyntaxNodeType::ExportAllDeclaration: {
                this->Traverse(*dynamic_cast<ExportAllDeclaration*>(&node));
                break;
            }

            case SyntaxNodeType::ExportDefaultDeclaration: {
                this->Traverse(*dynamic_cast<ExportDefaultDeclaration*>(&node));
                break;
            }

            case SyntaxNodeType::ExportNamedDeclaration: {
                this->Traverse(*dynamic_cast<ExportNamedDeclaration*>(&node));
                break;
            }

            case SyntaxNodeType::ExportSpecifier: {
                this->Traverse(*dynamic_cast<ExportSpecifier*>(&node));
                break;
            }

            case SyntaxNodeType::ExpressionStatement: {
                this->Traverse(*dynamic_cast<ExpressionStatement*>(&node));
                break;
            }

            case SyntaxNodeType::ForInStatement: {
                this->Traverse(*dynamic_cast<ForInStatement*>(&node));
                break;
            }

            case SyntaxNodeType::ForOfStatement: {
                this->Traverse(*dynamic_cast<ForOfStatement*>(&node));
                break;
            }

            case SyntaxNodeType::ForStatement: {
                this->Traverse(*dynamic_cast<ForStatement*>(&node));
                break;
            }

            case SyntaxNodeType::FunctionDeclaration: {
                this->Traverse(*dynamic_cast<FunctionDeclaration*>(&node));
                break;
            }

            case SyntaxNodeType::FunctionExpression: {
                this->Traverse(*dynamic_cast<FunctionExpression*>(&node));
                break;
            }

            case SyntaxNodeType::Identifier: {
                this->Traverse(*dynamic_cast<Identifier*>(&node));
                break;
            }

            case SyntaxNodeType::IfStatement: {
                this->Traverse(*dynamic_cast<IfStatement*>(&node));
                break;
            }

            case SyntaxNodeType::Import: {
                this->Traverse(*dynamic_cast<Import*>(&node));
                break;
            }

            case SyntaxNodeType::ImportDeclaration: {
                this->Traverse(*dynamic_cast<ImportDeclaration*>(&node));
                break;
            }

            case SyntaxNodeType::ImportDefaultSpecifier: {
                this->Traverse(*dynamic_cast<ImportDefaultSpecifier*>(&node));
                break;
            }

            case SyntaxNodeType::ImportNamespaceSpecifier: {
                this->Traverse(*dynamic_cast<ImportNamespaceSpecifier*>(&node));
                break;
            }

            case SyntaxNodeType::ImportSpecifier: {
                this->Traverse(*dynamic_cast<ImportSpecifier*>(&node));
                break;
            }

            case SyntaxNodeType::LabeledStatement: {
                this->Traverse(*dynamic_cast<LabeledStatement*>(&node));
                break;
            }

            case SyntaxNodeType::Literal: {
                this->Traverse(*dynamic_cast<Literal*>(&node));
                break;
            }

            case SyntaxNodeType::MetaProperty: {
                this->Traverse(*dynamic_cast<MetaProperty*>(&node));
                break;
            }

            case SyntaxNodeType::MethodDefinition: {
                this->Traverse(*dynamic_cast<MethodDefinition*>(&node));
                break;
            }

            case SyntaxNodeType::Module: {
                this->Traverse(*dynamic_cast<Module*>(&node));
                break;
            }

            case SyntaxNodeType::NewExpression: {
                this->Traverse(*dynamic_cast<NewExpression*>(&node));
                break;
            }

            case SyntaxNodeType::ObjectExpression: {
                this->Traverse(*dynamic_cast<ObjectExpression*>(&node));
                break;
            }

            case SyntaxNodeType::ObjectPattern: {
                this->Traverse(*dynamic_cast<ObjectPattern*>(&node));
                break;
            }

            case SyntaxNodeType::Property: {
                this->Traverse(*dynamic_cast<Property*>(&node));
                break;
            }

            case SyntaxNodeType::RegexLiteral: {
                this->Traverse(*dynamic_cast<RegexLiteral*>(&node));
                break;
            }

            case SyntaxNodeType::RestElement: {
                this->Traverse(*dynamic_cast<RestElement*>(&node));
                break;
            }

            case SyntaxNodeType::ReturnStatement: {
                this->Traverse(*dynamic_cast<ReturnStatement*>(&node));
                break;
            }

            case SyntaxNodeType::Script: {
                this->Traverse(*dynamic_cast<Script*>(&node));
                break;
            }

            case SyntaxNodeType::SequenceExpression: {
                this->Traverse(*dynamic_cast<SequenceExpression*>(&node));
                break;
            }

            case SyntaxNodeType::SpreadElement: {
                this->Traverse(*dynamic_cast<SpreadElement*>(&node));
                break;
            }

            case SyntaxNodeType::MemberExpression: {
                this->Traverse(*dynamic_cast<MemberExpression*>(&node));
                break;
            }

            case SyntaxNodeType::Super: {
                this->Traverse(*dynamic_cast<Super*>(&node));
                break;
            }

            case SyntaxNodeType::SwitchCase: {
                this->Traverse(*dynamic_cast<SwitchCase*>(&node));
                break;
            }

            case SyntaxNodeType::SwitchStatement: {
                this->Traverse(*dynamic_cast<SwitchStatement*>(&node));
                break;
            }

            case SyntaxNodeType::TaggedTemplateExpression: {
                this->Traverse(*dynamic_cast<TaggedTemplateExpression*>(&node));
                break;
            }

            case SyntaxNodeType::TemplateElement: {
                this->Traverse(*dynamic_cast<TemplateElement*>(&node));
                break;
            }

            case SyntaxNodeType::TemplateLiteral: {
                this->Traverse(*dynamic_cast<TemplateLiteral*>(&node));
                break;
            }

            case SyntaxNodeType::ThisExpression: {
                this->Traverse(*dynamic_cast<ThisExpression*>(&node));
                break;
            }

            case SyntaxNodeType::ThrowStatement: {
                this->Traverse(*dynamic_cast<ThrowStatement*>(&node));
                break;
            }

            case SyntaxNodeType::TryStatement: {
                this->Traverse(*dynamic_cast<TryStatement*>(&node));
                break;
            }

            case SyntaxNodeType::UnaryExpression: {
                this->Traverse(*dynamic_cast<UnaryExpression*>(&node));
                break;
            }

            case SyntaxNodeType::UpdateExpression: {
                this->Traverse(*dynamic_cast<UpdateExpression*>(&node));
                break;
            }

            case SyntaxNodeType::VariableDeclaration: {
                this->Traverse(*dynamic_cast<VariableDeclaration*>(&node));
                break;
            }

            case SyntaxNodeType::VariableDeclarator: {
                this->Traverse(*dynamic_cast<VariableDeclarator*>(&node));
                break;
            }

            case SyntaxNodeType::WhileStatement: {
                this->Traverse(*dynamic_cast<WhileStatement*>(&node));
                break;
            }

            case SyntaxNodeType::WithStatement: {
                this->Traverse(*dynamic_cast<WithStatement*>(&node));
                break;
            }

            case SyntaxNodeType::YieldExpression: {
                this->Traverse(*dynamic_cast<YieldExpression*>(&node));
                break;
            }

            case SyntaxNodeType::ArrowParameterPlaceHolder: {
                this->Traverse(*dynamic_cast<ArrowParameterPlaceHolder*>(&node));
                break;
            }

            case SyntaxNodeType::JSXClosingElement: {
                this->Traverse(*dynamic_cast<JSXClosingElement*>(&node));
                break;
            }

            case SyntaxNodeType::JSXElement: {
                this->Traverse(*dynamic_cast<JSXElement*>(&node));
                break;
            }

            case SyntaxNodeType::JSXEmptyExpression: {
                this->Traverse(*dynamic_cast<JSXEmptyExpression*>(&node));
                break;
            }

            case SyntaxNodeType::JSXExpressionContainer: {
                this->Traverse(*dynamic_cast<JSXExpressionContainer*>(&node));
                break;
            }

            case SyntaxNodeType::JSXIdentifier: {
                this->Traverse(*dynamic_cast<JSXIdentifier*>(&node));
                break;
            }

            case SyntaxNodeType::JSXMemberExpression: {
                this->Traverse(*dynamic_cast<JSXMemberExpression*>(&node));
                break;
            }

            case SyntaxNodeType::JSXAttribute: {
                this->Traverse(*dynamic_cast<JSXAttribute*>(&node));
                break;
            }

            case SyntaxNodeType::JSXNamespacedName: {
                this->Traverse(*dynamic_cast<JSXNamespacedName*>(&node));
                break;
            }

            case SyntaxNodeType::JSXOpeningElement: {
                this->Traverse(*dynamic_cast<JSXOpeningElement*>(&node));
                break;
            }

            case SyntaxNodeType::JSXSpreadAttribute: {
                this->Traverse(*dynamic_cast<JSXSpreadAttribute*>(&node));
                break;
            }

            case SyntaxNodeType::JSXText: {
                this->Traverse(*dynamic_cast<JSXText*>(&node));
                break;
            }

            case SyntaxNodeType::TSParameterProperty: {
                this->Traverse(*dynamic_cast<TSParameterProperty*>(&node));
                break;
            }

            case SyntaxNodeType::TSDeclareFunction: {
                this->Traverse(*dynamic_cast<TSDeclareFunction*>(&node));
                break;
            }

            case SyntaxNodeType::TSDeclareMethod: {
                this->Traverse(*dynamic_cast<TSDeclareMethod*>(&node));
                break;
            }

            case SyntaxNodeType::TSQualifiedName: {
                this->Traverse(*dynamic_cast<TSQualifiedName*>(&node));
                break;
            }

            case SyntaxNodeType::TSCallSignatureDeclaration: {
                this->Traverse(*dynamic_cast<TSCallSignatureDeclaration*>(&node));
                break;
            }

            case SyntaxNodeType::TSConstructSignatureDeclaration: {
                this->Traverse(*dynamic_cast<TSConstructSignatureDeclaration*>(&node));
                break;
            }

            case SyntaxNodeType::TSPropertySignature: {
                this->Traverse(*dynamic_cast<TSPropertySignature*>(&node));
                break;
            }

            case SyntaxNodeType::TSMethodSignature: {
                this->Traverse(*dynamic_cast<TSMethodSignature*>(&node));
                break;
            }

            case SyntaxNodeType::TSIndexSignature: {
                this->Traverse(*dynamic_cast<TSIndexSignature*>(&node));
                break;
            }

            case SyntaxNodeType::TSAnyKeyword: {
                this->Traverse(*dynamic_cast<TSAnyKeyword*>(&node));
                break;
            }

            case SyntaxNodeType::TSBooleanKeyword: {
                this->Traverse(*dynamic_cast<TSBooleanKeyword*>(&node));
                break;
            }

            case SyntaxNodeType::TSBigIntKeyword: {
                this->Traverse(*dynamic_cast<TSBigIntKeyword*>(&node));
                break;
            }

            case SyntaxNodeType::TSNeverKeyword: {
                this->Traverse(*dynamic_cast<TSNeverKeyword*>(&node));
                break;
            }

            case SyntaxNodeType::TSNullKeyword: {
                this->Traverse(*dynamic_cast<TSNullKeyword*>(&node));
                break;
            }

            case SyntaxNodeType::TSNumberKeyword: {
                this->Traverse(*dynamic_cast<TSNumberKeyword*>(&node));
                break;
            }

            case SyntaxNodeType::TSObjectKeyword: {
                this->Traverse(*dynamic_cast<TSObjectKeyword*>(&node));
                break;
            }

            case SyntaxNodeType::TSStringKeyword: {
                this->Traverse(*dynamic_cast<TSStringKeyword*>(&node));
                break;
            }

            case SyntaxNodeType::TSSymbolKeyword: {
                this->Traverse(*dynamic_cast<TSSymbolKeyword*>(&node));
                break;
            }

            case SyntaxNodeType::TSUndefinedKeyword: {
                this->Traverse(*dynamic_cast<TSUndefinedKeyword*>(&node));
                break;
            }

            case SyntaxNodeType::TSUnknownKeyword: {
                this->Traverse(*dynamic_cast<TSUnknownKeyword*>(&node));
                break;
            }

            case SyntaxNodeType::TSVoidKeyword: {
                this->Traverse(*dynamic_cast<TSVoidKeyword*>(&node));
                break;
            }

            case SyntaxNodeType::TSThisType: {
                this->Traverse(*dynamic_cast<TSThisType*>(&node));
                break;
            }

            case SyntaxNodeType::TSFunctionType: {
                this->Traverse(*dynamic_cast<TSFunctionType*>(&node));
                break;
            }

            case SyntaxNodeType::TSConstructorType: {
                this->Traverse(*dynamic_cast<TSConstructorType*>(&node));
                break;
            }

            case SyntaxNodeType::TSTypeReference: {
                this->Traverse(*dynamic_cast<TSTypeReference*>(&node));
                break;
            }

            case SyntaxNodeType::TSTypePredicate: {
                this->Traverse(*dynamic_cast<TSTypePredicate*>(&node));
                break;
            }

            case SyntaxNodeType::TSTypeQuery: {
                this->Traverse(*dynamic_cast<TSTypeQuery*>(&node));
                break;
            }

            case SyntaxNodeType::TSTypeLiteral: {
                this->Traverse(*dynamic_cast<TSTypeLiteral*>(&node));
                break;
            }

            case SyntaxNodeType::TSArrayType: {
                this->Traverse(*dynamic_cast<TSArrayType*>(&node));
                break;
            }

            case SyntaxNodeType::TSTupleType: {
                this->Traverse(*dynamic_cast<TSTupleType*>(&node));
                break;
            }

            case SyntaxNodeType::TSOptionalType: {
                this->Traverse(*dynamic_cast<TSOptionalType*>(&node));
                break;
            }

            case SyntaxNodeType::TSRestType: {
                this->Traverse(*dynamic_cast<TSRestType*>(&node));
                break;
            }

            case SyntaxNodeType::TSUnionType: {
                this->Traverse(*dynamic_cast<TSUnionType*>(&node));
                break;
            }

            case SyntaxNodeType::TSIntersectionType: {
                this->Traverse(*dynamic_cast<TSIntersectionType*>(&node));
                break;
            }

            case SyntaxNodeType::TSConditionalType: {
                this->Traverse(*dynamic_cast<TSConditionalType*>(&node));
                break;
            }

            case SyntaxNodeType::TSInferType: {
                this->Traverse(*dynamic_cast<TSInferType*>(&node));
                break;
            }

            case SyntaxNodeType::TSParenthesizedType: {
                this->Traverse(*dynamic_cast<TSParenthesizedType*>(&node));
                break;
            }

            case SyntaxNodeType::TSTypeOperator: {
                this->Traverse(*dynamic_cast<TSTypeOperator*>(&node));
                break;
            }

            case SyntaxNodeType::TSIndexedAccessType: {
                this->Traverse(*dynamic_cast<TSIndexedAccessType*>(&node));
                break;
            }

            case SyntaxNodeType::TSMappedType: {
                this->Traverse(*dynamic_cast<TSMappedType*>(&node));
                break;
            }

            case SyntaxNodeType::TSLiteralType: {
                this->Traverse(*dynamic_cast<TSLiteralType*>(&node));
                break;
            }

            case SyntaxNodeType::TSExpressionWithTypeArguments: {
                this->Traverse(*dynamic_cast<TSExpressionWithTypeArguments*>(&node));
                break;
            }

            case SyntaxNodeType::TSInterfaceDeclaration: {
                this->Traverse(*dynamic_cast<TSInterfaceDeclaration*>(&node));
                break;
            }

            case SyntaxNodeType::TSInterfaceBody: {
                this->Traverse(*dynamic_cast<TSInterfaceBody*>(&node));
                break;
            }

            case SyntaxNodeType::TSTypeAliasDeclaration: {
                this->Traverse(*dynamic_cast<TSTypeAliasDeclaration*>(&node));
                break;
            }

            case SyntaxNodeType::TSAsExpression: {
                this->Traverse(*dynamic_cast<TSAsExpression*>(&node));
                break;
            }

            case SyntaxNodeType::TSTypeAssertion: {
                this->Traverse(*dynamic_cast<TSTypeAssertion*>(&node));
                break;
            }

            case SyntaxNodeType::TSEnumDeclaration: {
                this->Traverse(*dynamic_cast<TSEnumDeclaration*>(&node));
                break;
            }

            case SyntaxNodeType::TSEnumMember: {
                this->Traverse(*dynamic_cast<TSEnumMember*>(&node));
                break;
            }

            case SyntaxNodeType::TSModuleDeclaration: {
                this->Traverse(*dynamic_cast<TSModuleDeclaration*>(&node));
                break;
            }

            case SyntaxNodeType::TSModuleBlock: {
                this->Traverse(*dynamic_cast<TSModuleBlock*>(&node));
                break;
            }

            case SyntaxNodeType::TSImportType: {
                this->Traverse(*dynamic_cast<TSImportType*>(&node));
                break;
            }

            case SyntaxNodeType::TSImportEqualsDeclaration: {
                this->Traverse(*dynamic_cast<TSImportEqualsDeclaration*>(&node));
                break;
            }

            case SyntaxNodeType::TSExternalModuleReference: {
                this->Traverse(*dynamic_cast<TSExternalModuleReference*>(&node));
                break;
            }

            case SyntaxNodeType::TSNonNullExpression: {
                this->Traverse(*dynamic_cast<TSNonNullExpression*>(&node));
                break;
            }

            case SyntaxNodeType::TSExportAssignment: {
                this->Traverse(*dynamic_cast<TSExportAssignment*>(&node));
                break;
            }

            case SyntaxNodeType::TSNamespaceExportDeclaration: {
                this->Traverse(*dynamic_cast<TSNamespaceExportDeclaration*>(&node));
                break;
            }

            case SyntaxNodeType::TSTypeAnnotation: {
                this->Traverse(*dynamic_cast<TSTypeAnnotation*>(&node));
                break;
            }

            case SyntaxNodeType::TSTypeParameterInstantiation: {
                this->Traverse(*dynamic_cast<TSTypeParameterInstantiation*>(&node));
                break;
            }

            case SyntaxNodeType::TSTypeParameterDeclaration: {
                this->Traverse(*dynamic_cast<TSTypeParameterDeclaration*>(&node));
                break;
            }

            case SyntaxNodeType::TSTypeParameter: {
                this->Traverse(*dynamic_cast<TSTypeParameter*>(&node));
                break;
            }

            default:
                return;

        }
    }

}

