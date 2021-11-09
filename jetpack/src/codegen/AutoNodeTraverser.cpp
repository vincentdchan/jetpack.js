//
// Created by Duzhong Chen on 2021/9/17.
//

#include "AutoNodeTraverser.h"

namespace jetpack {

    void AutoNodeTraverser::TraverseNode(SyntaxNode* node) {
        switch (node->type) {

            case SyntaxNodeType::ArrayExpression: {
                auto child = dynamic_cast<ArrayExpression*>(node);
                if (!this->TraverseBefore(child)) return;

                for (auto& i : child->elements) {
                    if (i.has_value()) {
                        TraverseNode(*i);
                    }
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ArrayPattern: {
                auto child = dynamic_cast<ArrayPattern*>(node);
                if (!this->TraverseBefore(child)) return;

                for (auto& i : child->elements) {
                    if (i.has_value()) {
                        TraverseNode(*i);
                    }
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ArrowFunctionExpression: {
                auto child = dynamic_cast<ArrowFunctionExpression*>(node);
                if (!this->TraverseBefore(child)) return;
                if (child->id) {
                    TraverseNode(*child->id);
                }

                for (auto i : child->params) {
                    TraverseNode(i);
                }
                TraverseNode(child->body);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::AssignmentExpression: {
                auto child = dynamic_cast<AssignmentExpression*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->left);
                TraverseNode(child->right);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::AssignmentPattern: {
                auto child = dynamic_cast<AssignmentPattern*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->left);
                TraverseNode(child->right);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::AwaitExpression: {
                auto child = dynamic_cast<AwaitExpression*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->argument);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::BinaryExpression: {
                auto child = dynamic_cast<BinaryExpression*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->left);
                TraverseNode(child->right);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::BlockStatement: {
                auto child = dynamic_cast<BlockStatement*>(node);
                if (!this->TraverseBefore(child)) return;

                for (auto i : child->body) {
                    TraverseNode(i);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::BreakStatement: {
                auto child = dynamic_cast<BreakStatement*>(node);
                if (!this->TraverseBefore(child)) return;
                if (child->label) {
                    TraverseNode(*child->label);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::CallExpression: {
                auto child = dynamic_cast<CallExpression*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->callee);

                for (auto i : child->arguments) {
                    TraverseNode(i);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::CatchClause: {
                auto child = dynamic_cast<CatchClause*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->param);
                TraverseNode(child->body);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ClassBody: {
                auto child = dynamic_cast<ClassBody*>(node);
                if (!this->TraverseBefore(child)) return;

                for (auto& i : child->body) {
                    TraverseNode(i);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ClassDeclaration: {
                auto child = dynamic_cast<ClassDeclaration*>(node);
                if (!this->TraverseBefore(child)) return;
                if (child->id) {
                    TraverseNode(*child->id);
                }
                if (child->super_class) {
                    TraverseNode(*child->super_class);
                }
                TraverseNode(child->body);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ClassExpression: {
                auto child = dynamic_cast<ClassExpression*>(node);
                if (!this->TraverseBefore(child)) return;
                if (child->id) {
                    TraverseNode(*child->id);
                }
                if (child->super_class) {
                    TraverseNode(*child->super_class);
                }
                if (child->body) {
                    TraverseNode(*child->body);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ConditionalExpression: {
                auto child = dynamic_cast<ConditionalExpression*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->test);
                TraverseNode(child->consequent);
                TraverseNode(child->alternate);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ContinueStatement: {
                auto child = dynamic_cast<ContinueStatement*>(node);
                if (!this->TraverseBefore(child)) return;
                if (child->label) {
                    TraverseNode(*child->label);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::DebuggerStatement: {
                auto child = dynamic_cast<DebuggerStatement*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::Directive: {
                auto child = dynamic_cast<Directive*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->expression);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::DoWhileStatement: {
                auto child = dynamic_cast<DoWhileStatement*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->body);
                TraverseNode(child->test);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::EmptyStatement: {
                auto child = dynamic_cast<EmptyStatement*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ExportAllDeclaration: {
                auto child = dynamic_cast<ExportAllDeclaration*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->source);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ExportDefaultDeclaration: {
                auto child = dynamic_cast<ExportDefaultDeclaration*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->declaration);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ExportNamedDeclaration: {
                auto child = dynamic_cast<ExportNamedDeclaration*>(node);
                if (!this->TraverseBefore(child)) return;
                if (child->declaration) {
                    TraverseNode(*child->declaration);
                }

                for (auto& i : child->specifiers) {
                    TraverseNode(i);
                }
                if (child->source) {
                    TraverseNode(*child->source);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ExportSpecifier: {
                auto child = dynamic_cast<ExportSpecifier*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->exported);
                TraverseNode(child->local);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ExpressionStatement: {
                auto child = dynamic_cast<ExpressionStatement*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->expression);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ForInStatement: {
                auto child = dynamic_cast<ForInStatement*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->left);
                TraverseNode(child->right);
                TraverseNode(child->body);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ForOfStatement: {
                auto child = dynamic_cast<ForOfStatement*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->left);
                TraverseNode(child->right);
                TraverseNode(child->body);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ForStatement: {
                auto child = dynamic_cast<ForStatement*>(node);
                if (!this->TraverseBefore(child)) return;
                if (child->init) {
                    TraverseNode(*child->init);
                }
                if (child->test) {
                    TraverseNode(*child->test);
                }
                if (child->update) {
                    TraverseNode(*child->update);
                }
                TraverseNode(child->body);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::FunctionDeclaration: {
                auto child = dynamic_cast<FunctionDeclaration*>(node);
                if (!this->TraverseBefore(child)) return;
                if (child->id) {
                    TraverseNode(*child->id);
                }

                for (auto i : child->params) {
                    TraverseNode(i);
                }
                TraverseNode(child->body);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::FunctionExpression: {
                auto child = dynamic_cast<FunctionExpression*>(node);
                if (!this->TraverseBefore(child)) return;
                if (child->id) {
                    TraverseNode(*child->id);
                }

                for (auto i : child->params) {
                    TraverseNode(i);
                }
                TraverseNode(child->body);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::Identifier: {
                auto child = dynamic_cast<Identifier*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::IfStatement: {
                auto child = dynamic_cast<IfStatement*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->test);
                TraverseNode(child->consequent);
                if (child->alternate) {
                    TraverseNode(*child->alternate);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::Import: {
                auto child = dynamic_cast<Import*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ImportDeclaration: {
                auto child = dynamic_cast<ImportDeclaration*>(node);
                if (!this->TraverseBefore(child)) return;

                for (auto& i : child->specifiers) {
                    TraverseNode(i);
                }
                TraverseNode(child->source);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ImportDefaultSpecifier: {
                auto child = dynamic_cast<ImportDefaultSpecifier*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->local);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ImportNamespaceSpecifier: {
                auto child = dynamic_cast<ImportNamespaceSpecifier*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->local);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ImportSpecifier: {
                auto child = dynamic_cast<ImportSpecifier*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->local);
                TraverseNode(child->imported);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::LabeledStatement: {
                auto child = dynamic_cast<LabeledStatement*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->label);
                TraverseNode(child->body);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::Literal: {
                auto child = dynamic_cast<Literal*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::MetaProperty: {
                auto child = dynamic_cast<MetaProperty*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->meta);
                TraverseNode(child->property);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::MethodDefinition: {
                auto child = dynamic_cast<MethodDefinition*>(node);
                if (!this->TraverseBefore(child)) return;
                if (child->key) {
                    TraverseNode(*child->key);
                }
                if (child->value) {
                    TraverseNode(*child->value);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::Module: {
                auto child = dynamic_cast<Module*>(node);
                if (!this->TraverseBefore(child)) return;

                for (auto i : child->body) {
                    TraverseNode(i);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::NewExpression: {
                auto child = dynamic_cast<NewExpression*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->callee);

                for (auto i : child->arguments) {
                    TraverseNode(i);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ObjectExpression: {
                auto child = dynamic_cast<ObjectExpression*>(node);
                if (!this->TraverseBefore(child)) return;

                for (auto& i : child->properties) {
                    TraverseNode(i);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ObjectPattern: {
                auto child = dynamic_cast<ObjectPattern*>(node);
                if (!this->TraverseBefore(child)) return;

                for (auto& i : child->properties) {
                    TraverseNode(i);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::Property: {
                auto child = dynamic_cast<Property*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->key);
                if (child->value) {
                    TraverseNode(*child->value);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::RegexLiteral: {
                auto child = dynamic_cast<RegexLiteral*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::RestElement: {
                auto child = dynamic_cast<RestElement*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->argument);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ReturnStatement: {
                auto child = dynamic_cast<ReturnStatement*>(node);
                if (!this->TraverseBefore(child)) return;
                if (child->argument) {
                    TraverseNode(*child->argument);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::Script: {
                auto child = dynamic_cast<Script*>(node);
                if (!this->TraverseBefore(child)) return;

                for (auto i : child->body) {
                    TraverseNode(i);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::SequenceExpression: {
                auto child = dynamic_cast<SequenceExpression*>(node);
                if (!this->TraverseBefore(child)) return;

                for (auto& i : child->expressions) {
                    TraverseNode(i);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::SpreadElement: {
                auto child = dynamic_cast<SpreadElement*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->argument);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::MemberExpression: {
                auto child = dynamic_cast<MemberExpression*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->object);
                TraverseNode(child->property);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::Super: {
                auto child = dynamic_cast<Super*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::SwitchCase: {
                auto child = dynamic_cast<SwitchCase*>(node);
                if (!this->TraverseBefore(child)) return;
                if (child->test) {
                    TraverseNode(*child->test);
                }

                for (auto& i : child->consequent) {
                    TraverseNode(i);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::SwitchStatement: {
                auto child = dynamic_cast<SwitchStatement*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->discrimiant);

                for (auto& i : child->cases) {
                    TraverseNode(i);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TaggedTemplateExpression: {
                auto child = dynamic_cast<TaggedTemplateExpression*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->tag);
                TraverseNode(child->quasi);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TemplateElement: {
                auto child = dynamic_cast<TemplateElement*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TemplateLiteral: {
                auto child = dynamic_cast<TemplateLiteral*>(node);
                if (!this->TraverseBefore(child)) return;

                for (auto& i : child->quasis) {
                    TraverseNode(i);
                }

                for (auto& i : child->expressions) {
                    TraverseNode(i);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ThisExpression: {
                auto child = dynamic_cast<ThisExpression*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ThrowStatement: {
                auto child = dynamic_cast<ThrowStatement*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->argument);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TryStatement: {
                auto child = dynamic_cast<TryStatement*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->block);
                if (child->handler) {
                    TraverseNode(*child->handler);
                }
                if (child->finalizer) {
                    TraverseNode(*child->finalizer);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::UnaryExpression: {
                auto child = dynamic_cast<UnaryExpression*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->argument);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::UpdateExpression: {
                auto child = dynamic_cast<UpdateExpression*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->argument);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::VariableDeclaration: {
                auto child = dynamic_cast<VariableDeclaration*>(node);
                if (!this->TraverseBefore(child)) return;

                for (auto& i : child->declarations) {
                    TraverseNode(i);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::VariableDeclarator: {
                auto child = dynamic_cast<VariableDeclarator*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->id);
                if (child->init) {
                    TraverseNode(*child->init);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::WhileStatement: {
                auto child = dynamic_cast<WhileStatement*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->test);
                TraverseNode(child->body);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::WithStatement: {
                auto child = dynamic_cast<WithStatement*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->object);
                TraverseNode(child->body);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::YieldExpression: {
                auto child = dynamic_cast<YieldExpression*>(node);
                if (!this->TraverseBefore(child)) return;
                if (child->argument) {
                    TraverseNode(*child->argument);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ArrowParameterPlaceHolder: {
                auto child = dynamic_cast<ArrowParameterPlaceHolder*>(node);
                if (!this->TraverseBefore(child)) return;

                for (auto i : child->params) {
                    TraverseNode(i);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::JSXClosingElement: {
                auto child = dynamic_cast<JSXClosingElement*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->name);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::JSXElement: {
                auto child = dynamic_cast<JSXElement*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->opening_element);

                for (auto& i : child->children) {
                    TraverseNode(i);
                }
                if (child->closing_element) {
                    TraverseNode(*child->closing_element);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::JSXEmptyExpression: {
                auto child = dynamic_cast<JSXEmptyExpression*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::JSXExpressionContainer: {
                auto child = dynamic_cast<JSXExpressionContainer*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->expression);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::JSXIdentifier: {
                auto child = dynamic_cast<JSXIdentifier*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::JSXMemberExpression: {
                auto child = dynamic_cast<JSXMemberExpression*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->object);
                TraverseNode(child->property);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::JSXAttribute: {
                auto child = dynamic_cast<JSXAttribute*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->name);
                if (child->value) {
                    TraverseNode(*child->value);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::JSXNamespacedName: {
                auto child = dynamic_cast<JSXNamespacedName*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->namespace_);
                TraverseNode(child->name);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::JSXOpeningElement: {
                auto child = dynamic_cast<JSXOpeningElement*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->name);

                for (auto& i : child->attributes) {
                    TraverseNode(i);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::JSXSpreadAttribute: {
                auto child = dynamic_cast<JSXSpreadAttribute*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->argument);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::JSXText: {
                auto child = dynamic_cast<JSXText*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSParameterProperty: {
                auto child = dynamic_cast<TSParameterProperty*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->parameter);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSDeclareFunction: {
                auto child = dynamic_cast<TSDeclareFunction*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->id);
                TraverseNode(child->return_type);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSDeclareMethod: {
                auto child = dynamic_cast<TSDeclareMethod*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSQualifiedName: {
                auto child = dynamic_cast<TSQualifiedName*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSCallSignatureDeclaration: {
                auto child = dynamic_cast<TSCallSignatureDeclaration*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSConstructSignatureDeclaration: {
                auto child = dynamic_cast<TSConstructSignatureDeclaration*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSPropertySignature: {
                auto child = dynamic_cast<TSPropertySignature*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSMethodSignature: {
                auto child = dynamic_cast<TSMethodSignature*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSIndexSignature: {
                auto child = dynamic_cast<TSIndexSignature*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSAnyKeyword: {
                auto child = dynamic_cast<TSAnyKeyword*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSBooleanKeyword: {
                auto child = dynamic_cast<TSBooleanKeyword*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSBigIntKeyword: {
                auto child = dynamic_cast<TSBigIntKeyword*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSNeverKeyword: {
                auto child = dynamic_cast<TSNeverKeyword*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSNullKeyword: {
                auto child = dynamic_cast<TSNullKeyword*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSNumberKeyword: {
                auto child = dynamic_cast<TSNumberKeyword*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSObjectKeyword: {
                auto child = dynamic_cast<TSObjectKeyword*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSStringKeyword: {
                auto child = dynamic_cast<TSStringKeyword*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSSymbolKeyword: {
                auto child = dynamic_cast<TSSymbolKeyword*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSUndefinedKeyword: {
                auto child = dynamic_cast<TSUndefinedKeyword*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSUnknownKeyword: {
                auto child = dynamic_cast<TSUnknownKeyword*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSVoidKeyword: {
                auto child = dynamic_cast<TSVoidKeyword*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSThisType: {
                auto child = dynamic_cast<TSThisType*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSFunctionType: {
                auto child = dynamic_cast<TSFunctionType*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSConstructorType: {
                auto child = dynamic_cast<TSConstructorType*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSTypeReference: {
                auto child = dynamic_cast<TSTypeReference*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSTypePredicate: {
                auto child = dynamic_cast<TSTypePredicate*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSTypeQuery: {
                auto child = dynamic_cast<TSTypeQuery*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSTypeLiteral: {
                auto child = dynamic_cast<TSTypeLiteral*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSArrayType: {
                auto child = dynamic_cast<TSArrayType*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSTupleType: {
                auto child = dynamic_cast<TSTupleType*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSOptionalType: {
                auto child = dynamic_cast<TSOptionalType*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSRestType: {
                auto child = dynamic_cast<TSRestType*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSUnionType: {
                auto child = dynamic_cast<TSUnionType*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSIntersectionType: {
                auto child = dynamic_cast<TSIntersectionType*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSConditionalType: {
                auto child = dynamic_cast<TSConditionalType*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSInferType: {
                auto child = dynamic_cast<TSInferType*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSParenthesizedType: {
                auto child = dynamic_cast<TSParenthesizedType*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSTypeOperator: {
                auto child = dynamic_cast<TSTypeOperator*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSIndexedAccessType: {
                auto child = dynamic_cast<TSIndexedAccessType*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSMappedType: {
                auto child = dynamic_cast<TSMappedType*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSLiteralType: {
                auto child = dynamic_cast<TSLiteralType*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSExpressionWithTypeArguments: {
                auto child = dynamic_cast<TSExpressionWithTypeArguments*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSInterfaceDeclaration: {
                auto child = dynamic_cast<TSInterfaceDeclaration*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSInterfaceBody: {
                auto child = dynamic_cast<TSInterfaceBody*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSTypeAliasDeclaration: {
                auto child = dynamic_cast<TSTypeAliasDeclaration*>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->id);
                if (child->type_parameters) {
                    TraverseNode(*child->type_parameters);
                }
                TraverseNode(child->type_annotation);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSAsExpression: {
                auto child = dynamic_cast<TSAsExpression*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSTypeAssertion: {
                auto child = dynamic_cast<TSTypeAssertion*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSEnumDeclaration: {
                auto child = dynamic_cast<TSEnumDeclaration*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSEnumMember: {
                auto child = dynamic_cast<TSEnumMember*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSModuleDeclaration: {
                auto child = dynamic_cast<TSModuleDeclaration*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSModuleBlock: {
                auto child = dynamic_cast<TSModuleBlock*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSImportType: {
                auto child = dynamic_cast<TSImportType*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSImportEqualsDeclaration: {
                auto child = dynamic_cast<TSImportEqualsDeclaration*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSExternalModuleReference: {
                auto child = dynamic_cast<TSExternalModuleReference*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSNonNullExpression: {
                auto child = dynamic_cast<TSNonNullExpression*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSExportAssignment: {
                auto child = dynamic_cast<TSExportAssignment*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSNamespaceExportDeclaration: {
                auto child = dynamic_cast<TSNamespaceExportDeclaration*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSTypeAnnotation: {
                auto child = dynamic_cast<TSTypeAnnotation*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSTypeParameterInstantiation: {
                auto child = dynamic_cast<TSTypeParameterInstantiation*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSTypeParameterDeclaration: {
                auto child = dynamic_cast<TSTypeParameterDeclaration*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSTypeParameter: {
                auto child = dynamic_cast<TSTypeParameter*>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            default:
                return;

        }
    }

}
