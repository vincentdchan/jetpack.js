
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

    void NodeTraverser::TraverseNode(const Sp<SyntaxNode>& node) {
        switch (node->type) {

            case SyntaxNodeType::ArrayExpression: {
                this->Traverse(std::dynamic_pointer_cast<ArrayExpression>(node));
                break;
            }

            case SyntaxNodeType::ArrayPattern: {
                this->Traverse(std::dynamic_pointer_cast<ArrayPattern>(node));
                break;
            }

            case SyntaxNodeType::ArrowFunctionExpression: {
                this->Traverse(std::dynamic_pointer_cast<ArrowFunctionExpression>(node));
                break;
            }

            case SyntaxNodeType::AssignmentExpression: {
                this->Traverse(std::dynamic_pointer_cast<AssignmentExpression>(node));
                break;
            }

            case SyntaxNodeType::AssignmentPattern: {
                this->Traverse(std::dynamic_pointer_cast<AssignmentPattern>(node));
                break;
            }

            case SyntaxNodeType::AwaitExpression: {
                this->Traverse(std::dynamic_pointer_cast<AwaitExpression>(node));
                break;
            }

            case SyntaxNodeType::BinaryExpression: {
                this->Traverse(std::dynamic_pointer_cast<BinaryExpression>(node));
                break;
            }

            case SyntaxNodeType::BlockStatement: {
                this->Traverse(std::dynamic_pointer_cast<BlockStatement>(node));
                break;
            }

            case SyntaxNodeType::BreakStatement: {
                this->Traverse(std::dynamic_pointer_cast<BreakStatement>(node));
                break;
            }

            case SyntaxNodeType::CallExpression: {
                this->Traverse(std::dynamic_pointer_cast<CallExpression>(node));
                break;
            }

            case SyntaxNodeType::CatchClause: {
                this->Traverse(std::dynamic_pointer_cast<CatchClause>(node));
                break;
            }

            case SyntaxNodeType::ClassBody: {
                this->Traverse(std::dynamic_pointer_cast<ClassBody>(node));
                break;
            }

            case SyntaxNodeType::ClassDeclaration: {
                this->Traverse(std::dynamic_pointer_cast<ClassDeclaration>(node));
                break;
            }

            case SyntaxNodeType::ClassExpression: {
                this->Traverse(std::dynamic_pointer_cast<ClassExpression>(node));
                break;
            }

            case SyntaxNodeType::ConditionalExpression: {
                this->Traverse(std::dynamic_pointer_cast<ConditionalExpression>(node));
                break;
            }

            case SyntaxNodeType::ContinueStatement: {
                this->Traverse(std::dynamic_pointer_cast<ContinueStatement>(node));
                break;
            }

            case SyntaxNodeType::DebuggerStatement: {
                this->Traverse(std::dynamic_pointer_cast<DebuggerStatement>(node));
                break;
            }

            case SyntaxNodeType::Directive: {
                this->Traverse(std::dynamic_pointer_cast<Directive>(node));
                break;
            }

            case SyntaxNodeType::DoWhileStatement: {
                this->Traverse(std::dynamic_pointer_cast<DoWhileStatement>(node));
                break;
            }

            case SyntaxNodeType::EmptyStatement: {
                this->Traverse(std::dynamic_pointer_cast<EmptyStatement>(node));
                break;
            }

            case SyntaxNodeType::ExportAllDeclaration: {
                this->Traverse(std::dynamic_pointer_cast<ExportAllDeclaration>(node));
                break;
            }

            case SyntaxNodeType::ExportDefaultDeclaration: {
                this->Traverse(std::dynamic_pointer_cast<ExportDefaultDeclaration>(node));
                break;
            }

            case SyntaxNodeType::ExportNamedDeclaration: {
                this->Traverse(std::dynamic_pointer_cast<ExportNamedDeclaration>(node));
                break;
            }

            case SyntaxNodeType::ExportSpecifier: {
                this->Traverse(std::dynamic_pointer_cast<ExportSpecifier>(node));
                break;
            }

            case SyntaxNodeType::ExpressionStatement: {
                this->Traverse(std::dynamic_pointer_cast<ExpressionStatement>(node));
                break;
            }

            case SyntaxNodeType::ForInStatement: {
                this->Traverse(std::dynamic_pointer_cast<ForInStatement>(node));
                break;
            }

            case SyntaxNodeType::ForOfStatement: {
                this->Traverse(std::dynamic_pointer_cast<ForOfStatement>(node));
                break;
            }

            case SyntaxNodeType::ForStatement: {
                this->Traverse(std::dynamic_pointer_cast<ForStatement>(node));
                break;
            }

            case SyntaxNodeType::FunctionDeclaration: {
                this->Traverse(std::dynamic_pointer_cast<FunctionDeclaration>(node));
                break;
            }

            case SyntaxNodeType::FunctionExpression: {
                this->Traverse(std::dynamic_pointer_cast<FunctionExpression>(node));
                break;
            }

            case SyntaxNodeType::Identifier: {
                this->Traverse(std::dynamic_pointer_cast<Identifier>(node));
                break;
            }

            case SyntaxNodeType::IfStatement: {
                this->Traverse(std::dynamic_pointer_cast<IfStatement>(node));
                break;
            }

            case SyntaxNodeType::Import: {
                this->Traverse(std::dynamic_pointer_cast<Import>(node));
                break;
            }

            case SyntaxNodeType::ImportDeclaration: {
                this->Traverse(std::dynamic_pointer_cast<ImportDeclaration>(node));
                break;
            }

            case SyntaxNodeType::ImportDefaultSpecifier: {
                this->Traverse(std::dynamic_pointer_cast<ImportDefaultSpecifier>(node));
                break;
            }

            case SyntaxNodeType::ImportNamespaceSpecifier: {
                this->Traverse(std::dynamic_pointer_cast<ImportNamespaceSpecifier>(node));
                break;
            }

            case SyntaxNodeType::ImportSpecifier: {
                this->Traverse(std::dynamic_pointer_cast<ImportSpecifier>(node));
                break;
            }

            case SyntaxNodeType::LabeledStatement: {
                this->Traverse(std::dynamic_pointer_cast<LabeledStatement>(node));
                break;
            }

            case SyntaxNodeType::Literal: {
                this->Traverse(std::dynamic_pointer_cast<Literal>(node));
                break;
            }

            case SyntaxNodeType::MetaProperty: {
                this->Traverse(std::dynamic_pointer_cast<MetaProperty>(node));
                break;
            }

            case SyntaxNodeType::MethodDefinition: {
                this->Traverse(std::dynamic_pointer_cast<MethodDefinition>(node));
                break;
            }

            case SyntaxNodeType::Module: {
                this->Traverse(std::dynamic_pointer_cast<Module>(node));
                break;
            }

            case SyntaxNodeType::NewExpression: {
                this->Traverse(std::dynamic_pointer_cast<NewExpression>(node));
                break;
            }

            case SyntaxNodeType::ObjectExpression: {
                this->Traverse(std::dynamic_pointer_cast<ObjectExpression>(node));
                break;
            }

            case SyntaxNodeType::ObjectPattern: {
                this->Traverse(std::dynamic_pointer_cast<ObjectPattern>(node));
                break;
            }

            case SyntaxNodeType::Property: {
                this->Traverse(std::dynamic_pointer_cast<Property>(node));
                break;
            }

            case SyntaxNodeType::RegexLiteral: {
                this->Traverse(std::dynamic_pointer_cast<RegexLiteral>(node));
                break;
            }

            case SyntaxNodeType::RestElement: {
                this->Traverse(std::dynamic_pointer_cast<RestElement>(node));
                break;
            }

            case SyntaxNodeType::ReturnStatement: {
                this->Traverse(std::dynamic_pointer_cast<ReturnStatement>(node));
                break;
            }

            case SyntaxNodeType::Script: {
                this->Traverse(std::dynamic_pointer_cast<Script>(node));
                break;
            }

            case SyntaxNodeType::SequenceExpression: {
                this->Traverse(std::dynamic_pointer_cast<SequenceExpression>(node));
                break;
            }

            case SyntaxNodeType::SpreadElement: {
                this->Traverse(std::dynamic_pointer_cast<SpreadElement>(node));
                break;
            }

            case SyntaxNodeType::MemberExpression: {
                this->Traverse(std::dynamic_pointer_cast<MemberExpression>(node));
                break;
            }

            case SyntaxNodeType::Super: {
                this->Traverse(std::dynamic_pointer_cast<Super>(node));
                break;
            }

            case SyntaxNodeType::SwitchCase: {
                this->Traverse(std::dynamic_pointer_cast<SwitchCase>(node));
                break;
            }

            case SyntaxNodeType::SwitchStatement: {
                this->Traverse(std::dynamic_pointer_cast<SwitchStatement>(node));
                break;
            }

            case SyntaxNodeType::TaggedTemplateExpression: {
                this->Traverse(std::dynamic_pointer_cast<TaggedTemplateExpression>(node));
                break;
            }

            case SyntaxNodeType::TemplateElement: {
                this->Traverse(std::dynamic_pointer_cast<TemplateElement>(node));
                break;
            }

            case SyntaxNodeType::TemplateLiteral: {
                this->Traverse(std::dynamic_pointer_cast<TemplateLiteral>(node));
                break;
            }

            case SyntaxNodeType::ThisExpression: {
                this->Traverse(std::dynamic_pointer_cast<ThisExpression>(node));
                break;
            }

            case SyntaxNodeType::ThrowStatement: {
                this->Traverse(std::dynamic_pointer_cast<ThrowStatement>(node));
                break;
            }

            case SyntaxNodeType::TryStatement: {
                this->Traverse(std::dynamic_pointer_cast<TryStatement>(node));
                break;
            }

            case SyntaxNodeType::UnaryExpression: {
                this->Traverse(std::dynamic_pointer_cast<UnaryExpression>(node));
                break;
            }

            case SyntaxNodeType::UpdateExpression: {
                this->Traverse(std::dynamic_pointer_cast<UpdateExpression>(node));
                break;
            }

            case SyntaxNodeType::VariableDeclaration: {
                this->Traverse(std::dynamic_pointer_cast<VariableDeclaration>(node));
                break;
            }

            case SyntaxNodeType::VariableDeclarator: {
                this->Traverse(std::dynamic_pointer_cast<VariableDeclarator>(node));
                break;
            }

            case SyntaxNodeType::WhileStatement: {
                this->Traverse(std::dynamic_pointer_cast<WhileStatement>(node));
                break;
            }

            case SyntaxNodeType::WithStatement: {
                this->Traverse(std::dynamic_pointer_cast<WithStatement>(node));
                break;
            }

            case SyntaxNodeType::YieldExpression: {
                this->Traverse(std::dynamic_pointer_cast<YieldExpression>(node));
                break;
            }

            case SyntaxNodeType::ArrowParameterPlaceHolder: {
                this->Traverse(std::dynamic_pointer_cast<ArrowParameterPlaceHolder>(node));
                break;
            }

            case SyntaxNodeType::JSXClosingElement: {
                this->Traverse(std::dynamic_pointer_cast<JSXClosingElement>(node));
                break;
            }

            case SyntaxNodeType::JSXElement: {
                this->Traverse(std::dynamic_pointer_cast<JSXElement>(node));
                break;
            }

            case SyntaxNodeType::JSXEmptyExpression: {
                this->Traverse(std::dynamic_pointer_cast<JSXEmptyExpression>(node));
                break;
            }

            case SyntaxNodeType::JSXExpressionContainer: {
                this->Traverse(std::dynamic_pointer_cast<JSXExpressionContainer>(node));
                break;
            }

            case SyntaxNodeType::JSXIdentifier: {
                this->Traverse(std::dynamic_pointer_cast<JSXIdentifier>(node));
                break;
            }

            case SyntaxNodeType::JSXMemberExpression: {
                this->Traverse(std::dynamic_pointer_cast<JSXMemberExpression>(node));
                break;
            }

            case SyntaxNodeType::JSXAttribute: {
                this->Traverse(std::dynamic_pointer_cast<JSXAttribute>(node));
                break;
            }

            case SyntaxNodeType::JSXNamespacedName: {
                this->Traverse(std::dynamic_pointer_cast<JSXNamespacedName>(node));
                break;
            }

            case SyntaxNodeType::JSXOpeningElement: {
                this->Traverse(std::dynamic_pointer_cast<JSXOpeningElement>(node));
                break;
            }

            case SyntaxNodeType::JSXSpreadAttribute: {
                this->Traverse(std::dynamic_pointer_cast<JSXSpreadAttribute>(node));
                break;
            }

            case SyntaxNodeType::JSXText: {
                this->Traverse(std::dynamic_pointer_cast<JSXText>(node));
                break;
            }

            case SyntaxNodeType::TSParameterProperty: {
                this->Traverse(std::dynamic_pointer_cast<TSParameterProperty>(node));
                break;
            }

            case SyntaxNodeType::TSDeclareFunction: {
                this->Traverse(std::dynamic_pointer_cast<TSDeclareFunction>(node));
                break;
            }

            case SyntaxNodeType::TSDeclareMethod: {
                this->Traverse(std::dynamic_pointer_cast<TSDeclareMethod>(node));
                break;
            }

            case SyntaxNodeType::TSQualifiedName: {
                this->Traverse(std::dynamic_pointer_cast<TSQualifiedName>(node));
                break;
            }

            case SyntaxNodeType::TSCallSignatureDeclaration: {
                this->Traverse(std::dynamic_pointer_cast<TSCallSignatureDeclaration>(node));
                break;
            }

            case SyntaxNodeType::TSConstructSignatureDeclaration: {
                this->Traverse(std::dynamic_pointer_cast<TSConstructSignatureDeclaration>(node));
                break;
            }

            case SyntaxNodeType::TSPropertySignature: {
                this->Traverse(std::dynamic_pointer_cast<TSPropertySignature>(node));
                break;
            }

            case SyntaxNodeType::TSMethodSignature: {
                this->Traverse(std::dynamic_pointer_cast<TSMethodSignature>(node));
                break;
            }

            case SyntaxNodeType::TSIndexSignature: {
                this->Traverse(std::dynamic_pointer_cast<TSIndexSignature>(node));
                break;
            }

            case SyntaxNodeType::TSAnyKeyword: {
                this->Traverse(std::dynamic_pointer_cast<TSAnyKeyword>(node));
                break;
            }

            case SyntaxNodeType::TSBooleanKeyword: {
                this->Traverse(std::dynamic_pointer_cast<TSBooleanKeyword>(node));
                break;
            }

            case SyntaxNodeType::TSBigIntKeyword: {
                this->Traverse(std::dynamic_pointer_cast<TSBigIntKeyword>(node));
                break;
            }

            case SyntaxNodeType::TSNeverKeyword: {
                this->Traverse(std::dynamic_pointer_cast<TSNeverKeyword>(node));
                break;
            }

            case SyntaxNodeType::TSNullKeyword: {
                this->Traverse(std::dynamic_pointer_cast<TSNullKeyword>(node));
                break;
            }

            case SyntaxNodeType::TSNumberKeyword: {
                this->Traverse(std::dynamic_pointer_cast<TSNumberKeyword>(node));
                break;
            }

            case SyntaxNodeType::TSObjectKeyword: {
                this->Traverse(std::dynamic_pointer_cast<TSObjectKeyword>(node));
                break;
            }

            case SyntaxNodeType::TSStringKeyword: {
                this->Traverse(std::dynamic_pointer_cast<TSStringKeyword>(node));
                break;
            }

            case SyntaxNodeType::TSSymbolKeyword: {
                this->Traverse(std::dynamic_pointer_cast<TSSymbolKeyword>(node));
                break;
            }

            case SyntaxNodeType::TSUndefinedKeyword: {
                this->Traverse(std::dynamic_pointer_cast<TSUndefinedKeyword>(node));
                break;
            }

            case SyntaxNodeType::TSUnknownKeyword: {
                this->Traverse(std::dynamic_pointer_cast<TSUnknownKeyword>(node));
                break;
            }

            case SyntaxNodeType::TSVoidKeyword: {
                this->Traverse(std::dynamic_pointer_cast<TSVoidKeyword>(node));
                break;
            }

            case SyntaxNodeType::TSThisType: {
                this->Traverse(std::dynamic_pointer_cast<TSThisType>(node));
                break;
            }

            case SyntaxNodeType::TSFunctionType: {
                this->Traverse(std::dynamic_pointer_cast<TSFunctionType>(node));
                break;
            }

            case SyntaxNodeType::TSConstructorType: {
                this->Traverse(std::dynamic_pointer_cast<TSConstructorType>(node));
                break;
            }

            case SyntaxNodeType::TSTypeReference: {
                this->Traverse(std::dynamic_pointer_cast<TSTypeReference>(node));
                break;
            }

            case SyntaxNodeType::TSTypePredicate: {
                this->Traverse(std::dynamic_pointer_cast<TSTypePredicate>(node));
                break;
            }

            case SyntaxNodeType::TSTypeQuery: {
                this->Traverse(std::dynamic_pointer_cast<TSTypeQuery>(node));
                break;
            }

            case SyntaxNodeType::TSTypeLiteral: {
                this->Traverse(std::dynamic_pointer_cast<TSTypeLiteral>(node));
                break;
            }

            case SyntaxNodeType::TSArrayType: {
                this->Traverse(std::dynamic_pointer_cast<TSArrayType>(node));
                break;
            }

            case SyntaxNodeType::TSTupleType: {
                this->Traverse(std::dynamic_pointer_cast<TSTupleType>(node));
                break;
            }

            case SyntaxNodeType::TSOptionalType: {
                this->Traverse(std::dynamic_pointer_cast<TSOptionalType>(node));
                break;
            }

            case SyntaxNodeType::TSRestType: {
                this->Traverse(std::dynamic_pointer_cast<TSRestType>(node));
                break;
            }

            case SyntaxNodeType::TSUnionType: {
                this->Traverse(std::dynamic_pointer_cast<TSUnionType>(node));
                break;
            }

            case SyntaxNodeType::TSIntersectionType: {
                this->Traverse(std::dynamic_pointer_cast<TSIntersectionType>(node));
                break;
            }

            case SyntaxNodeType::TSConditionalType: {
                this->Traverse(std::dynamic_pointer_cast<TSConditionalType>(node));
                break;
            }

            case SyntaxNodeType::TSInferType: {
                this->Traverse(std::dynamic_pointer_cast<TSInferType>(node));
                break;
            }

            case SyntaxNodeType::TSParenthesizedType: {
                this->Traverse(std::dynamic_pointer_cast<TSParenthesizedType>(node));
                break;
            }

            case SyntaxNodeType::TSTypeOperator: {
                this->Traverse(std::dynamic_pointer_cast<TSTypeOperator>(node));
                break;
            }

            case SyntaxNodeType::TSIndexedAccessType: {
                this->Traverse(std::dynamic_pointer_cast<TSIndexedAccessType>(node));
                break;
            }

            case SyntaxNodeType::TSMappedType: {
                this->Traverse(std::dynamic_pointer_cast<TSMappedType>(node));
                break;
            }

            case SyntaxNodeType::TSLiteralType: {
                this->Traverse(std::dynamic_pointer_cast<TSLiteralType>(node));
                break;
            }

            case SyntaxNodeType::TSExpressionWithTypeArguments: {
                this->Traverse(std::dynamic_pointer_cast<TSExpressionWithTypeArguments>(node));
                break;
            }

            case SyntaxNodeType::TSInterfaceDeclaration: {
                this->Traverse(std::dynamic_pointer_cast<TSInterfaceDeclaration>(node));
                break;
            }

            case SyntaxNodeType::TSInterfaceBody: {
                this->Traverse(std::dynamic_pointer_cast<TSInterfaceBody>(node));
                break;
            }

            case SyntaxNodeType::TSTypeAliasDeclaration: {
                this->Traverse(std::dynamic_pointer_cast<TSTypeAliasDeclaration>(node));
                break;
            }

            case SyntaxNodeType::TSAsExpression: {
                this->Traverse(std::dynamic_pointer_cast<TSAsExpression>(node));
                break;
            }

            case SyntaxNodeType::TSTypeAssertion: {
                this->Traverse(std::dynamic_pointer_cast<TSTypeAssertion>(node));
                break;
            }

            case SyntaxNodeType::TSEnumDeclaration: {
                this->Traverse(std::dynamic_pointer_cast<TSEnumDeclaration>(node));
                break;
            }

            case SyntaxNodeType::TSEnumMember: {
                this->Traverse(std::dynamic_pointer_cast<TSEnumMember>(node));
                break;
            }

            case SyntaxNodeType::TSModuleDeclaration: {
                this->Traverse(std::dynamic_pointer_cast<TSModuleDeclaration>(node));
                break;
            }

            case SyntaxNodeType::TSModuleBlock: {
                this->Traverse(std::dynamic_pointer_cast<TSModuleBlock>(node));
                break;
            }

            case SyntaxNodeType::TSImportType: {
                this->Traverse(std::dynamic_pointer_cast<TSImportType>(node));
                break;
            }

            case SyntaxNodeType::TSImportEqualsDeclaration: {
                this->Traverse(std::dynamic_pointer_cast<TSImportEqualsDeclaration>(node));
                break;
            }

            case SyntaxNodeType::TSExternalModuleReference: {
                this->Traverse(std::dynamic_pointer_cast<TSExternalModuleReference>(node));
                break;
            }

            case SyntaxNodeType::TSNonNullExpression: {
                this->Traverse(std::dynamic_pointer_cast<TSNonNullExpression>(node));
                break;
            }

            case SyntaxNodeType::TSExportAssignment: {
                this->Traverse(std::dynamic_pointer_cast<TSExportAssignment>(node));
                break;
            }

            case SyntaxNodeType::TSNamespaceExportDeclaration: {
                this->Traverse(std::dynamic_pointer_cast<TSNamespaceExportDeclaration>(node));
                break;
            }

            case SyntaxNodeType::TSTypeAnnotation: {
                this->Traverse(std::dynamic_pointer_cast<TSTypeAnnotation>(node));
                break;
            }

            case SyntaxNodeType::TSTypeParameterInstantiation: {
                this->Traverse(std::dynamic_pointer_cast<TSTypeParameterInstantiation>(node));
                break;
            }

            case SyntaxNodeType::TSTypeParameterDeclaration: {
                this->Traverse(std::dynamic_pointer_cast<TSTypeParameterDeclaration>(node));
                break;
            }

            case SyntaxNodeType::TSTypeParameter: {
                this->Traverse(std::dynamic_pointer_cast<TSTypeParameter>(node));
                break;
            }

            default:
                return;

        }
    }

}

