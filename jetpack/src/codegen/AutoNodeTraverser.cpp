//
// Created by Duzhong Chen on 2021/9/17.
//

#include "NodeTraverser.h"

namespace jetpack {

    void AutoNodeTraverser::TraverseNode(const Sp<SyntaxNode>& node) {
        switch (node->type) {

            case SyntaxNodeType::ArrayExpression: {
                auto child = std::dynamic_pointer_cast<ArrayExpression>(node);
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
                auto child = std::dynamic_pointer_cast<ArrayPattern>(node);
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
                auto child = std::dynamic_pointer_cast<ArrowFunctionExpression>(node);
                if (!this->TraverseBefore(child)) return;
                if (child->id) {
                    TraverseNode(*child->id);
                }

                for (auto& i : child->params) {
                    TraverseNode(i);
                }
                TraverseNode(child->body);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::AssignmentExpression: {
                auto child = std::dynamic_pointer_cast<AssignmentExpression>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->left);
                TraverseNode(child->right);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::AssignmentPattern: {
                auto child = std::dynamic_pointer_cast<AssignmentPattern>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->left);
                TraverseNode(child->right);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::AwaitExpression: {
                auto child = std::dynamic_pointer_cast<AwaitExpression>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->argument);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::BinaryExpression: {
                auto child = std::dynamic_pointer_cast<BinaryExpression>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->left);
                TraverseNode(child->right);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::BlockStatement: {
                auto child = std::dynamic_pointer_cast<BlockStatement>(node);
                if (!this->TraverseBefore(child)) return;

                for (auto& i : child->body) {
                    TraverseNode(i);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::BreakStatement: {
                auto child = std::dynamic_pointer_cast<BreakStatement>(node);
                if (!this->TraverseBefore(child)) return;
                if (child->label) {
                    TraverseNode(*child->label);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::CallExpression: {
                auto child = std::dynamic_pointer_cast<CallExpression>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->callee);

                for (auto& i : child->arguments) {
                    TraverseNode(i);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::CatchClause: {
                auto child = std::dynamic_pointer_cast<CatchClause>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->param);
                TraverseNode(child->body);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ClassBody: {
                auto child = std::dynamic_pointer_cast<ClassBody>(node);
                if (!this->TraverseBefore(child)) return;

                for (auto& i : child->body) {
                    TraverseNode(i);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ClassDeclaration: {
                auto child = std::dynamic_pointer_cast<ClassDeclaration>(node);
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
                auto child = std::dynamic_pointer_cast<ClassExpression>(node);
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
                auto child = std::dynamic_pointer_cast<ConditionalExpression>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->test);
                TraverseNode(child->consequent);
                TraverseNode(child->alternate);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ContinueStatement: {
                auto child = std::dynamic_pointer_cast<ContinueStatement>(node);
                if (!this->TraverseBefore(child)) return;
                if (child->label) {
                    TraverseNode(*child->label);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::DebuggerStatement: {
                auto child = std::dynamic_pointer_cast<DebuggerStatement>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::Directive: {
                auto child = std::dynamic_pointer_cast<Directive>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->expression);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::DoWhileStatement: {
                auto child = std::dynamic_pointer_cast<DoWhileStatement>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->body);
                TraverseNode(child->test);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::EmptyStatement: {
                auto child = std::dynamic_pointer_cast<EmptyStatement>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ExportAllDeclaration: {
                auto child = std::dynamic_pointer_cast<ExportAllDeclaration>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->source);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ExportDefaultDeclaration: {
                auto child = std::dynamic_pointer_cast<ExportDefaultDeclaration>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->declaration);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ExportNamedDeclaration: {
                auto child = std::dynamic_pointer_cast<ExportNamedDeclaration>(node);
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
                auto child = std::dynamic_pointer_cast<ExportSpecifier>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->exported);
                TraverseNode(child->local);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ExpressionStatement: {
                auto child = std::dynamic_pointer_cast<ExpressionStatement>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->expression);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ForInStatement: {
                auto child = std::dynamic_pointer_cast<ForInStatement>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->left);
                TraverseNode(child->right);
                TraverseNode(child->body);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ForOfStatement: {
                auto child = std::dynamic_pointer_cast<ForOfStatement>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->left);
                TraverseNode(child->right);
                TraverseNode(child->body);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ForStatement: {
                auto child = std::dynamic_pointer_cast<ForStatement>(node);
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
                auto child = std::dynamic_pointer_cast<FunctionDeclaration>(node);
                if (!this->TraverseBefore(child)) return;
                if (child->id) {
                    TraverseNode(*child->id);
                }

                for (auto& i : child->params) {
                    TraverseNode(i);
                }
                TraverseNode(child->body);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::FunctionExpression: {
                auto child = std::dynamic_pointer_cast<FunctionExpression>(node);
                if (!this->TraverseBefore(child)) return;
                if (child->id) {
                    TraverseNode(*child->id);
                }

                for (auto& i : child->params) {
                    TraverseNode(i);
                }
                TraverseNode(child->body);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::Identifier: {
                auto child = std::dynamic_pointer_cast<Identifier>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::IfStatement: {
                auto child = std::dynamic_pointer_cast<IfStatement>(node);
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
                auto child = std::dynamic_pointer_cast<Import>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ImportDeclaration: {
                auto child = std::dynamic_pointer_cast<ImportDeclaration>(node);
                if (!this->TraverseBefore(child)) return;

                for (auto& i : child->specifiers) {
                    TraverseNode(i);
                }
                TraverseNode(child->source);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ImportDefaultSpecifier: {
                auto child = std::dynamic_pointer_cast<ImportDefaultSpecifier>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->local);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ImportNamespaceSpecifier: {
                auto child = std::dynamic_pointer_cast<ImportNamespaceSpecifier>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->local);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ImportSpecifier: {
                auto child = std::dynamic_pointer_cast<ImportSpecifier>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->local);
                TraverseNode(child->imported);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::LabeledStatement: {
                auto child = std::dynamic_pointer_cast<LabeledStatement>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->label);
                TraverseNode(child->body);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::Literal: {
                auto child = std::dynamic_pointer_cast<Literal>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::MetaProperty: {
                auto child = std::dynamic_pointer_cast<MetaProperty>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->meta);
                TraverseNode(child->property);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::MethodDefinition: {
                auto child = std::dynamic_pointer_cast<MethodDefinition>(node);
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
                auto child = std::dynamic_pointer_cast<Module>(node);
                if (!this->TraverseBefore(child)) return;

                for (auto& i : child->body) {
                    TraverseNode(i);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::NewExpression: {
                auto child = std::dynamic_pointer_cast<NewExpression>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->callee);

                for (auto& i : child->arguments) {
                    TraverseNode(i);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ObjectExpression: {
                auto child = std::dynamic_pointer_cast<ObjectExpression>(node);
                if (!this->TraverseBefore(child)) return;

                for (auto& i : child->properties) {
                    TraverseNode(i);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ObjectPattern: {
                auto child = std::dynamic_pointer_cast<ObjectPattern>(node);
                if (!this->TraverseBefore(child)) return;

                for (auto& i : child->properties) {
                    TraverseNode(i);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::Property: {
                auto child = std::dynamic_pointer_cast<Property>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->key);
                if (child->value) {
                    TraverseNode(*child->value);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::RegexLiteral: {
                auto child = std::dynamic_pointer_cast<RegexLiteral>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::RestElement: {
                auto child = std::dynamic_pointer_cast<RestElement>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->argument);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ReturnStatement: {
                auto child = std::dynamic_pointer_cast<ReturnStatement>(node);
                if (!this->TraverseBefore(child)) return;
                if (child->argument) {
                    TraverseNode(*child->argument);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::Script: {
                auto child = std::dynamic_pointer_cast<Script>(node);
                if (!this->TraverseBefore(child)) return;

                for (auto& i : child->body) {
                    TraverseNode(i);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::SequenceExpression: {
                auto child = std::dynamic_pointer_cast<SequenceExpression>(node);
                if (!this->TraverseBefore(child)) return;

                for (auto& i : child->expressions) {
                    TraverseNode(i);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::SpreadElement: {
                auto child = std::dynamic_pointer_cast<SpreadElement>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->argument);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::MemberExpression: {
                auto child = std::dynamic_pointer_cast<MemberExpression>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->object);
                TraverseNode(child->property);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::Super: {
                auto child = std::dynamic_pointer_cast<Super>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::SwitchCase: {
                auto child = std::dynamic_pointer_cast<SwitchCase>(node);
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
                auto child = std::dynamic_pointer_cast<SwitchStatement>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->discrimiant);

                for (auto& i : child->cases) {
                    TraverseNode(i);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TaggedTemplateExpression: {
                auto child = std::dynamic_pointer_cast<TaggedTemplateExpression>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->tag);
                TraverseNode(child->quasi);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TemplateElement: {
                auto child = std::dynamic_pointer_cast<TemplateElement>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TemplateLiteral: {
                auto child = std::dynamic_pointer_cast<TemplateLiteral>(node);
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
                auto child = std::dynamic_pointer_cast<ThisExpression>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ThrowStatement: {
                auto child = std::dynamic_pointer_cast<ThrowStatement>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->argument);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TryStatement: {
                auto child = std::dynamic_pointer_cast<TryStatement>(node);
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
                auto child = std::dynamic_pointer_cast<UnaryExpression>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->argument);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::UpdateExpression: {
                auto child = std::dynamic_pointer_cast<UpdateExpression>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->argument);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::VariableDeclaration: {
                auto child = std::dynamic_pointer_cast<VariableDeclaration>(node);
                if (!this->TraverseBefore(child)) return;

                for (auto& i : child->declarations) {
                    TraverseNode(i);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::VariableDeclarator: {
                auto child = std::dynamic_pointer_cast<VariableDeclarator>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->id);
                if (child->init) {
                    TraverseNode(*child->init);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::WhileStatement: {
                auto child = std::dynamic_pointer_cast<WhileStatement>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->test);
                TraverseNode(child->body);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::WithStatement: {
                auto child = std::dynamic_pointer_cast<WithStatement>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->object);
                TraverseNode(child->body);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::YieldExpression: {
                auto child = std::dynamic_pointer_cast<YieldExpression>(node);
                if (!this->TraverseBefore(child)) return;
                if (child->argument) {
                    TraverseNode(*child->argument);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::ArrowParameterPlaceHolder: {
                auto child = std::dynamic_pointer_cast<ArrowParameterPlaceHolder>(node);
                if (!this->TraverseBefore(child)) return;

                for (auto& i : child->params) {
                    TraverseNode(i);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::JSXClosingElement: {
                auto child = std::dynamic_pointer_cast<JSXClosingElement>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->name);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::JSXElement: {
                auto child = std::dynamic_pointer_cast<JSXElement>(node);
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
                auto child = std::dynamic_pointer_cast<JSXEmptyExpression>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::JSXExpressionContainer: {
                auto child = std::dynamic_pointer_cast<JSXExpressionContainer>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->expression);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::JSXIdentifier: {
                auto child = std::dynamic_pointer_cast<JSXIdentifier>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::JSXMemberExpression: {
                auto child = std::dynamic_pointer_cast<JSXMemberExpression>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->object);
                TraverseNode(child->property);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::JSXAttribute: {
                auto child = std::dynamic_pointer_cast<JSXAttribute>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->name);
                if (child->value) {
                    TraverseNode(*child->value);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::JSXNamespacedName: {
                auto child = std::dynamic_pointer_cast<JSXNamespacedName>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->namespace_);
                TraverseNode(child->name);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::JSXOpeningElement: {
                auto child = std::dynamic_pointer_cast<JSXOpeningElement>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->name);

                for (auto& i : child->attributes) {
                    TraverseNode(i);
                }

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::JSXSpreadAttribute: {
                auto child = std::dynamic_pointer_cast<JSXSpreadAttribute>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->argument);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::JSXText: {
                auto child = std::dynamic_pointer_cast<JSXText>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSParameterProperty: {
                auto child = std::dynamic_pointer_cast<TSParameterProperty>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->parameter);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSDeclareFunction: {
                auto child = std::dynamic_pointer_cast<TSDeclareFunction>(node);
                if (!this->TraverseBefore(child)) return;
                TraverseNode(child->id);
                TraverseNode(child->return_type);

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSDeclareMethod: {
                auto child = std::dynamic_pointer_cast<TSDeclareMethod>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSQualifiedName: {
                auto child = std::dynamic_pointer_cast<TSQualifiedName>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSCallSignatureDeclaration: {
                auto child = std::dynamic_pointer_cast<TSCallSignatureDeclaration>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSConstructSignatureDeclaration: {
                auto child = std::dynamic_pointer_cast<TSConstructSignatureDeclaration>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSPropertySignature: {
                auto child = std::dynamic_pointer_cast<TSPropertySignature>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSMethodSignature: {
                auto child = std::dynamic_pointer_cast<TSMethodSignature>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSIndexSignature: {
                auto child = std::dynamic_pointer_cast<TSIndexSignature>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSAnyKeyword: {
                auto child = std::dynamic_pointer_cast<TSAnyKeyword>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSBooleanKeyword: {
                auto child = std::dynamic_pointer_cast<TSBooleanKeyword>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSBigIntKeyword: {
                auto child = std::dynamic_pointer_cast<TSBigIntKeyword>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSNeverKeyword: {
                auto child = std::dynamic_pointer_cast<TSNeverKeyword>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSNullKeyword: {
                auto child = std::dynamic_pointer_cast<TSNullKeyword>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSNumberKeyword: {
                auto child = std::dynamic_pointer_cast<TSNumberKeyword>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSObjectKeyword: {
                auto child = std::dynamic_pointer_cast<TSObjectKeyword>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSStringKeyword: {
                auto child = std::dynamic_pointer_cast<TSStringKeyword>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSSymbolKeyword: {
                auto child = std::dynamic_pointer_cast<TSSymbolKeyword>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSUndefinedKeyword: {
                auto child = std::dynamic_pointer_cast<TSUndefinedKeyword>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSUnknownKeyword: {
                auto child = std::dynamic_pointer_cast<TSUnknownKeyword>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSVoidKeyword: {
                auto child = std::dynamic_pointer_cast<TSVoidKeyword>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSThisType: {
                auto child = std::dynamic_pointer_cast<TSThisType>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSFunctionType: {
                auto child = std::dynamic_pointer_cast<TSFunctionType>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSConstructorType: {
                auto child = std::dynamic_pointer_cast<TSConstructorType>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSTypeReference: {
                auto child = std::dynamic_pointer_cast<TSTypeReference>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSTypePredicate: {
                auto child = std::dynamic_pointer_cast<TSTypePredicate>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSTypeQuery: {
                auto child = std::dynamic_pointer_cast<TSTypeQuery>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSTypeLiteral: {
                auto child = std::dynamic_pointer_cast<TSTypeLiteral>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSArrayType: {
                auto child = std::dynamic_pointer_cast<TSArrayType>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSTupleType: {
                auto child = std::dynamic_pointer_cast<TSTupleType>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSOptionalType: {
                auto child = std::dynamic_pointer_cast<TSOptionalType>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSRestType: {
                auto child = std::dynamic_pointer_cast<TSRestType>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSUnionType: {
                auto child = std::dynamic_pointer_cast<TSUnionType>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSIntersectionType: {
                auto child = std::dynamic_pointer_cast<TSIntersectionType>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSConditionalType: {
                auto child = std::dynamic_pointer_cast<TSConditionalType>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSInferType: {
                auto child = std::dynamic_pointer_cast<TSInferType>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSParenthesizedType: {
                auto child = std::dynamic_pointer_cast<TSParenthesizedType>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSTypeOperator: {
                auto child = std::dynamic_pointer_cast<TSTypeOperator>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSIndexedAccessType: {
                auto child = std::dynamic_pointer_cast<TSIndexedAccessType>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSMappedType: {
                auto child = std::dynamic_pointer_cast<TSMappedType>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSLiteralType: {
                auto child = std::dynamic_pointer_cast<TSLiteralType>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSExpressionWithTypeArguments: {
                auto child = std::dynamic_pointer_cast<TSExpressionWithTypeArguments>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSInterfaceDeclaration: {
                auto child = std::dynamic_pointer_cast<TSInterfaceDeclaration>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSInterfaceBody: {
                auto child = std::dynamic_pointer_cast<TSInterfaceBody>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSTypeAliasDeclaration: {
                auto child = std::dynamic_pointer_cast<TSTypeAliasDeclaration>(node);
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
                auto child = std::dynamic_pointer_cast<TSAsExpression>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSTypeAssertion: {
                auto child = std::dynamic_pointer_cast<TSTypeAssertion>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSEnumDeclaration: {
                auto child = std::dynamic_pointer_cast<TSEnumDeclaration>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSEnumMember: {
                auto child = std::dynamic_pointer_cast<TSEnumMember>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSModuleDeclaration: {
                auto child = std::dynamic_pointer_cast<TSModuleDeclaration>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSModuleBlock: {
                auto child = std::dynamic_pointer_cast<TSModuleBlock>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSImportType: {
                auto child = std::dynamic_pointer_cast<TSImportType>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSImportEqualsDeclaration: {
                auto child = std::dynamic_pointer_cast<TSImportEqualsDeclaration>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSExternalModuleReference: {
                auto child = std::dynamic_pointer_cast<TSExternalModuleReference>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSNonNullExpression: {
                auto child = std::dynamic_pointer_cast<TSNonNullExpression>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSExportAssignment: {
                auto child = std::dynamic_pointer_cast<TSExportAssignment>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSNamespaceExportDeclaration: {
                auto child = std::dynamic_pointer_cast<TSNamespaceExportDeclaration>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSTypeAnnotation: {
                auto child = std::dynamic_pointer_cast<TSTypeAnnotation>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSTypeParameterInstantiation: {
                auto child = std::dynamic_pointer_cast<TSTypeParameterInstantiation>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSTypeParameterDeclaration: {
                auto child = std::dynamic_pointer_cast<TSTypeParameterDeclaration>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            case SyntaxNodeType::TSTypeParameter: {
                auto child = std::dynamic_pointer_cast<TSTypeParameter>(node);
                if (!this->TraverseBefore(child)) return;

                this->TraverseAfter(child);
                break;
            }

            default:
                return;

        }
    }

}
