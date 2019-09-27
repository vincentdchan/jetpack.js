
#include "./node_traverser.h"

void NodeTraverser::TraverseNode(const Sp<SyntaxNode>& node) {
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
        }

        case SyntaxNodeType::AssignmentExpression: {
            auto child = std::dynamic_pointer_cast<AssignmentExpression>(node);
            if (!this->TraverseBefore(child)) return;
            TraverseNode(child->left);
            TraverseNode(child->right);

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::AssignmentPattern: {
            auto child = std::dynamic_pointer_cast<AssignmentPattern>(node);
            if (!this->TraverseBefore(child)) return;
            TraverseNode(child->left);
            TraverseNode(child->right);

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::AwaitExpression: {
            auto child = std::dynamic_pointer_cast<AwaitExpression>(node);
            if (!this->TraverseBefore(child)) return;
            TraverseNode(child->argument);

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::BinaryExpression: {
            auto child = std::dynamic_pointer_cast<BinaryExpression>(node);
            if (!this->TraverseBefore(child)) return;
            TraverseNode(child->left);
            TraverseNode(child->right);

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::BlockStatement: {
            auto child = std::dynamic_pointer_cast<BlockStatement>(node);
            if (!this->TraverseBefore(child)) return;

              for (auto& i : child->body) {
                  TraverseNode(i);
              }

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::BreakStatement: {
            auto child = std::dynamic_pointer_cast<BreakStatement>(node);
            if (!this->TraverseBefore(child)) return;
            if (child->label) {
                TraverseNode(*child->label);
            }

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::CallExpression: {
            auto child = std::dynamic_pointer_cast<CallExpression>(node);
            if (!this->TraverseBefore(child)) return;
            TraverseNode(child->callee);

              for (auto& i : child->arguments) {
                  TraverseNode(i);
              }

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::CatchClause: {
            auto child = std::dynamic_pointer_cast<CatchClause>(node);
            if (!this->TraverseBefore(child)) return;
            TraverseNode(child->param);
            TraverseNode(child->body);

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::ClassBody: {
            auto child = std::dynamic_pointer_cast<ClassBody>(node);
            if (!this->TraverseBefore(child)) return;

              for (auto& i : child->body) {
                  TraverseNode(i);
              }

            this->TraverseAfter(child);
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
        }

        case SyntaxNodeType::ComputedMemberExpression: {
            auto child = std::dynamic_pointer_cast<ComputedMemberExpression>(node);
            if (!this->TraverseBefore(child)) return;
            TraverseNode(child->object);
            TraverseNode(child->property);

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::ConditionalExpression: {
            auto child = std::dynamic_pointer_cast<ConditionalExpression>(node);
            if (!this->TraverseBefore(child)) return;
            TraverseNode(child->test);
            TraverseNode(child->consequent);
            TraverseNode(child->alternate);

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::ContinueStatement: {
            auto child = std::dynamic_pointer_cast<ContinueStatement>(node);
            if (!this->TraverseBefore(child)) return;
            if (child->label) {
                TraverseNode(*child->label);
            }

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::DebuggerStatement: {
            auto child = std::dynamic_pointer_cast<DebuggerStatement>(node);
            if (!this->TraverseBefore(child)) return;

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::Directive: {
            auto child = std::dynamic_pointer_cast<Directive>(node);
            if (!this->TraverseBefore(child)) return;
            TraverseNode(child->expression);

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::DoWhileStatement: {
            auto child = std::dynamic_pointer_cast<DoWhileStatement>(node);
            if (!this->TraverseBefore(child)) return;
            TraverseNode(child->body);
            TraverseNode(child->test);

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::EmptyStatement: {
            auto child = std::dynamic_pointer_cast<EmptyStatement>(node);
            if (!this->TraverseBefore(child)) return;

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::ExportAllDeclaration: {
            auto child = std::dynamic_pointer_cast<ExportAllDeclaration>(node);
            if (!this->TraverseBefore(child)) return;
            TraverseNode(child->source);

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::ExportDefaultDeclaration: {
            auto child = std::dynamic_pointer_cast<ExportDefaultDeclaration>(node);
            if (!this->TraverseBefore(child)) return;
            TraverseNode(child->declaration);

            this->TraverseAfter(child);
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
        }

        case SyntaxNodeType::ExportSpecifier: {
            auto child = std::dynamic_pointer_cast<ExportSpecifier>(node);
            if (!this->TraverseBefore(child)) return;
            TraverseNode(child->exported);
            TraverseNode(child->local);

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::ExpressionStatement: {
            auto child = std::dynamic_pointer_cast<ExpressionStatement>(node);
            if (!this->TraverseBefore(child)) return;
            TraverseNode(child->expression);

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::ForInStatement: {
            auto child = std::dynamic_pointer_cast<ForInStatement>(node);
            if (!this->TraverseBefore(child)) return;
            TraverseNode(child->left);
            TraverseNode(child->right);
            TraverseNode(child->body);

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::ForOfStatement: {
            auto child = std::dynamic_pointer_cast<ForOfStatement>(node);
            if (!this->TraverseBefore(child)) return;
            TraverseNode(child->left);
            TraverseNode(child->right);
            TraverseNode(child->body);

            this->TraverseAfter(child);
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
        }

        case SyntaxNodeType::Identifier: {
            auto child = std::dynamic_pointer_cast<Identifier>(node);
            if (!this->TraverseBefore(child)) return;

            this->TraverseAfter(child);
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
        }

        case SyntaxNodeType::Import: {
            auto child = std::dynamic_pointer_cast<Import>(node);
            if (!this->TraverseBefore(child)) return;

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::ImportDeclaration: {
            auto child = std::dynamic_pointer_cast<ImportDeclaration>(node);
            if (!this->TraverseBefore(child)) return;

              for (auto& i : child->specifiers) {
                  TraverseNode(i);
              }
            TraverseNode(child->source);

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::ImportDefaultSpecifier: {
            auto child = std::dynamic_pointer_cast<ImportDefaultSpecifier>(node);
            if (!this->TraverseBefore(child)) return;
            TraverseNode(child->local);

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::ImportNamespaceSpecifier: {
            auto child = std::dynamic_pointer_cast<ImportNamespaceSpecifier>(node);
            if (!this->TraverseBefore(child)) return;
            TraverseNode(child->local);

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::ImportSpecifier: {
            auto child = std::dynamic_pointer_cast<ImportSpecifier>(node);
            if (!this->TraverseBefore(child)) return;
            TraverseNode(child->local);
            TraverseNode(child->imported);

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::LabeledStatement: {
            auto child = std::dynamic_pointer_cast<LabeledStatement>(node);
            if (!this->TraverseBefore(child)) return;
            TraverseNode(child->label);
            TraverseNode(child->body);

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::Literal: {
            auto child = std::dynamic_pointer_cast<Literal>(node);
            if (!this->TraverseBefore(child)) return;

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::MetaProperty: {
            auto child = std::dynamic_pointer_cast<MetaProperty>(node);
            if (!this->TraverseBefore(child)) return;
            TraverseNode(child->meta);
            TraverseNode(child->property);

            this->TraverseAfter(child);
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
        }

        case SyntaxNodeType::Module: {
            auto child = std::dynamic_pointer_cast<Module>(node);
            if (!this->TraverseBefore(child)) return;

              for (auto& i : child->body) {
                  TraverseNode(i);
              }

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::NewExpression: {
            auto child = std::dynamic_pointer_cast<NewExpression>(node);
            if (!this->TraverseBefore(child)) return;
            TraverseNode(child->callee);

              for (auto& i : child->arguments) {
                  TraverseNode(i);
              }

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::ObjectExpression: {
            auto child = std::dynamic_pointer_cast<ObjectExpression>(node);
            if (!this->TraverseBefore(child)) return;

              for (auto& i : child->properties) {
                  TraverseNode(i);
              }

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::ObjectPattern: {
            auto child = std::dynamic_pointer_cast<ObjectPattern>(node);
            if (!this->TraverseBefore(child)) return;

              for (auto& i : child->properties) {
                  TraverseNode(i);
              }

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::Property: {
            auto child = std::dynamic_pointer_cast<Property>(node);
            if (!this->TraverseBefore(child)) return;
            TraverseNode(child->key);
            if (child->value) {
                TraverseNode(*child->value);
            }

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::RegexLiteral: {
            auto child = std::dynamic_pointer_cast<RegexLiteral>(node);
            if (!this->TraverseBefore(child)) return;

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::RestElement: {
            auto child = std::dynamic_pointer_cast<RestElement>(node);
            if (!this->TraverseBefore(child)) return;
            TraverseNode(child->argument);

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::ReturnStatement: {
            auto child = std::dynamic_pointer_cast<ReturnStatement>(node);
            if (!this->TraverseBefore(child)) return;
            if (child->argument) {
                TraverseNode(*child->argument);
            }

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::Script: {
            auto child = std::dynamic_pointer_cast<Script>(node);
            if (!this->TraverseBefore(child)) return;

              for (auto& i : child->body) {
                  TraverseNode(i);
              }

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::SequenceExpression: {
            auto child = std::dynamic_pointer_cast<SequenceExpression>(node);
            if (!this->TraverseBefore(child)) return;

              for (auto& i : child->expressions) {
                  TraverseNode(i);
              }

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::SpreadElement: {
            auto child = std::dynamic_pointer_cast<SpreadElement>(node);
            if (!this->TraverseBefore(child)) return;
            TraverseNode(child->argument);

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::StaticMemberExpression: {
            auto child = std::dynamic_pointer_cast<StaticMemberExpression>(node);
            if (!this->TraverseBefore(child)) return;
            TraverseNode(child->object);
            TraverseNode(child->property);

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::Super: {
            auto child = std::dynamic_pointer_cast<Super>(node);
            if (!this->TraverseBefore(child)) return;

            this->TraverseAfter(child);
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
        }

        case SyntaxNodeType::SwitchStatement: {
            auto child = std::dynamic_pointer_cast<SwitchStatement>(node);
            if (!this->TraverseBefore(child)) return;
            TraverseNode(child->discrimiant);

              for (auto& i : child->cases) {
                  TraverseNode(i);
              }

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::TaggedTemplateExpression: {
            auto child = std::dynamic_pointer_cast<TaggedTemplateExpression>(node);
            if (!this->TraverseBefore(child)) return;
            TraverseNode(child->tag);
            TraverseNode(child->quasi);

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::TemplateElement: {
            auto child = std::dynamic_pointer_cast<TemplateElement>(node);
            if (!this->TraverseBefore(child)) return;

            this->TraverseAfter(child);
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
        }

        case SyntaxNodeType::ThisExpression: {
            auto child = std::dynamic_pointer_cast<ThisExpression>(node);
            if (!this->TraverseBefore(child)) return;

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::ThrowStatement: {
            auto child = std::dynamic_pointer_cast<ThrowStatement>(node);
            if (!this->TraverseBefore(child)) return;
            TraverseNode(child->argument);

            this->TraverseAfter(child);
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
        }

        case SyntaxNodeType::UnaryExpression: {
            auto child = std::dynamic_pointer_cast<UnaryExpression>(node);
            if (!this->TraverseBefore(child)) return;
            TraverseNode(child->argument);

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::UpdateExpression: {
            auto child = std::dynamic_pointer_cast<UpdateExpression>(node);
            if (!this->TraverseBefore(child)) return;
            TraverseNode(child->argument);

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::VariableDeclaration: {
            auto child = std::dynamic_pointer_cast<VariableDeclaration>(node);
            if (!this->TraverseBefore(child)) return;

              for (auto& i : child->declarations) {
                  TraverseNode(i);
              }

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::VariableDeclarator: {
            auto child = std::dynamic_pointer_cast<VariableDeclarator>(node);
            if (!this->TraverseBefore(child)) return;
            TraverseNode(child->id);
            if (child->init) {
                TraverseNode(*child->init);
            }

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::WhileStatement: {
            auto child = std::dynamic_pointer_cast<WhileStatement>(node);
            if (!this->TraverseBefore(child)) return;
            TraverseNode(child->test);
            TraverseNode(child->body);

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::WithStatement: {
            auto child = std::dynamic_pointer_cast<WithStatement>(node);
            if (!this->TraverseBefore(child)) return;
            TraverseNode(child->object);
            TraverseNode(child->body);

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::YieldExpression: {
            auto child = std::dynamic_pointer_cast<YieldExpression>(node);
            if (!this->TraverseBefore(child)) return;
            if (child->argument) {
                TraverseNode(*child->argument);
            }

            this->TraverseAfter(child);
        }

        case SyntaxNodeType::ArrowParameterPlaceHolder: {
            auto child = std::dynamic_pointer_cast<ArrowParameterPlaceHolder>(node);
            if (!this->TraverseBefore(child)) return;

              for (auto& i : child->params) {
                  TraverseNode(i);
              }

            this->TraverseAfter(child);
        }

        default:
            return;

    }
}
