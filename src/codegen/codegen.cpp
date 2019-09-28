//
// Created by Duzhong Chen on 2019/9/27.
//

#include "codegen.h"

using namespace std;

CodeGen::CodeGen() {
    CodeGen(Config());
}

CodeGen::CodeGen(const Config& config) {
    config_ = config;
}


int CodeGen::ExpressionPrecedence(SyntaxNodeType t) {
    switch (t) {
        case SyntaxNodeType::ArrayExpression:
        case SyntaxNodeType::TaggedTemplateExpression:
        case SyntaxNodeType::ThisExpression:
        case SyntaxNodeType::Identifier:
        case SyntaxNodeType::TemplateLiteral:
        case SyntaxNodeType::Super:
        case SyntaxNodeType::SequenceExpression:
            return 20;

        case SyntaxNodeType::Literal:
            return 18;

        // Operations
        case SyntaxNodeType::ComputedMemberExpression:
        case SyntaxNodeType::StaticMemberExpression:
        case SyntaxNodeType::CallExpression:
        case SyntaxNodeType::NewExpression:
            return 19;

        // Other definitions
        case SyntaxNodeType::ArrowFunctionExpression:
        case SyntaxNodeType::ClassExpression:
        case SyntaxNodeType::FunctionExpression:
        case SyntaxNodeType::ObjectExpression:
            return needs_parentheses;

        // Other operations
        case SyntaxNodeType::UpdateExpression:
            return 16;

        case SyntaxNodeType::UnaryExpression:
            return 15;

        case SyntaxNodeType::BinaryExpression:
            return 14;

            // TODO: Logical?
//        case SyntaxNodeType:::
//            return 13;

        case SyntaxNodeType::ConditionalExpression:
            return 4;

        case SyntaxNodeType::AssignmentExpression:
            return 3;

        case SyntaxNodeType::AwaitExpression:
            return 2;

        case SyntaxNodeType::YieldExpression:
            return 2;

        case SyntaxNodeType::RestElement:
            return 1;

        default:
            return 0;

    }
}

void CodeGen::Traverse(const Sp<ArrayExpression> &node) {
    output << '[';
    std::size_t count = 0;
    for (auto& elem : node->elements) {
        if (elem.has_value()) {
            TraverseNode(*elem);
        }
        if (count++ < node->elements.size() - 1) {
            output << ", ";
        } else if (!elem.has_value()) {
            output << ", ";
        }
    }
    output << ']';
}

void CodeGen::Traverse(const Sp<BlockStatement> &node) {
    output << '{';
    state_.indent_level++;

    if (!node->body.empty()) {
        WriteLineEnd();
        for (auto& elem : node->body) {
            WriteIndent();
            TraverseNode(elem);
            WriteLineEnd();
        }
    }

    state_.indent_level--;
    output << '}';
}

void CodeGen::Traverse(const Sp<EmptyStatement> &node) {
    Write(';');
}

void CodeGen::Traverse(const Sp<ExpressionStatement> &node) {
    int precedence = ExpressionPrecedence(node->type);
    if (
        (precedence == needs_parentheses) ||
            (precedence == 3 && node->expression->type == SyntaxNodeType::ObjectPattern)) {
        Write('(');
        TraverseNode(node->expression);
        Write(')');
    } else {
        TraverseNode(node->expression);
    }
    Write(';');
}

void CodeGen::Traverse(const Sp<IfStatement> &node) {
    Write("if (");
    TraverseNode(node->test);
    Write(")");
    TraverseNode(node->consequent);
    if (!node->alternate.has_value()) {
        Write(" else ");
        TraverseNode(*node->alternate);
    }
}

void CodeGen::Traverse(const Sp<LabeledStatement> &node) {
    TraverseNode(node->label);
    Write(": ");
    TraverseNode(node->body);
}

void CodeGen::Traverse(const Sp<BreakStatement> &node) {
    Write("break");
    if (node->label.has_value()) {
        Write(" ");
        TraverseNode(*node->label);
    }
    Write(";");
}

void CodeGen::Traverse(const Sp<ContinueStatement> &node) {
    Write("continue");
    if (node->label.has_value()) {
        Write(" ");
        TraverseNode(*node->label);
    }
    Write(";");
}

void CodeGen::Traverse(const Sp<WithStatement> &node) {
    Write("with (");
    TraverseNode(node->object);
    Write(") ");
    TraverseNode(node->body);
}

void CodeGen::Traverse(const Sp<SwitchStatement> &node) {
    Write("switch (");
    TraverseNode(node->discrimiant);
    Write(") {");
    WriteLineEnd();
    state_.indent_level++;

    for (auto& case_ : node->cases) {

        WriteIndent();
        if (case_->test.has_value()) {
            Write("case ");
            TraverseNode(*case_->test);
            Write(":");
            WriteLineEnd();
        } else {
            Write("default:");
            WriteLineEnd();
        }

        for (auto& cons : case_->consequent) {
            WriteIndent();
            TraverseNode(cons);
            WriteLineEnd();
        }

    }

    state_.indent_level--;
    WriteIndentWith("}");
}

void CodeGen::Traverse(const Sp<ReturnStatement> &node) {
    Write("return");
    if (node->argument.has_value()) {
        Write(" ");
        TraverseNode(*node->argument);
    }
    Write(";");
}

void CodeGen::Traverse(const Sp<ThrowStatement> &node) {
    Write("throw ");
    TraverseNode(node->argument);
    Write(";");
}

void CodeGen::Traverse(const Sp<TryStatement> &node) {
    Write("try ");
    TraverseNode(node->block);

    if (node->handler.has_value()) {
        auto handler = *node->handler;
        Write(" catch (");
        TraverseNode(handler->param);
        Write(")");
        TraverseNode(handler->body);
    }

    if (node->finalizer.has_value()) {
        Write(" finally ");
        TraverseNode(*node->finalizer);
    }
}

void CodeGen::Traverse(const Sp<WhileStatement> &node) {
    Write("while (");
    TraverseNode(node->test);
    Write(") ");
    TraverseNode(node->body);
}

void CodeGen::Traverse(const Sp<DoWhileStatement> &node) {
    Write("do ");
    TraverseNode(node->body);
    Write(" while (");
    TraverseNode(node->test);
    Write(");");
}

void CodeGen::Traverse(const Sp<ForStatement> &node) {
    Write("for (");
    if (node->init.has_value()) {
        auto init = *node->init;
        if (init->type == SyntaxNodeType::VariableDeclaration) {
            auto decl = dynamic_pointer_cast<VariableDeclaration>(init);
            FormatVariableDeclaration(decl);
        } else {
            TraverseNode(init);
        }
    }
    Write("; ");
    if (node->test.has_value()) {
        TraverseNode(*node->test);
    }
    Write("; ");
    if (node->update.has_value()) {
        TraverseNode(*node->update);
    }
    Write(") ");
    TraverseNode(node->body);
}

void CodeGen::Traverse(const Sp<ForInStatement> &node) {
    Write("for (");
    if (node->left->type == SyntaxNodeType::VariableDeclaration) {
        auto decl = dynamic_pointer_cast<VariableDeclaration>(node->left);
        FormatVariableDeclaration(decl);
    } else {
        TraverseNode(node->left);
    }
    Write(" in ");
    TraverseNode(node->right);
    Write(") ");
    TraverseNode(node->body);
}

void CodeGen::Traverse(const Sp<ForOfStatement> & node) {
    Write("for (");
    if (node->left->type == SyntaxNodeType::VariableDeclaration) {
        auto decl = dynamic_pointer_cast<VariableDeclaration>(node->left);
        FormatVariableDeclaration(decl);
    } else {
        TraverseNode(node->left);
    }
    Write(" of ");
    TraverseNode(node->right);
    Write(") ");
    TraverseNode(node->body);
}

void CodeGen::Traverse(const Sp<DebuggerStatement> &node) {
    Write("debugger;");
    WriteLineEnd();
}

void CodeGen::Traverse(const Sp<FunctionDeclaration> &node) {
    if (node->async) {
        Write("async ");
    }

    if (node->generator) {
        Write("function* ");
    } else {
        Write("function");
    }

    if (node->id.has_value()) {
        Write((*node->id)->name);
    }

    FormatSequence(node->params);
    Write(" ");
    TraverseNode(node->body);
}

void CodeGen::Traverse(const Sp<FunctionExpression> &node) {
    if (node->async) {
        Write("async ");
    }

    if (node->generator) {
        Write("function* ");
    } else {
        Write("function");
    }

    if (node->id.has_value()) {
        Write((*node->id)->name);
    }

    FormatSequence(node->params);
    Write(" ");
    TraverseNode(node->body);
}

void CodeGen::Traverse(const Sp<VariableDeclaration> &node) {
    FormatVariableDeclaration(node);
    Write(";");
}

void CodeGen::Traverse(const Sp<VariableDeclarator> &node) {
    TraverseNode(node->id);
    if (node->init.has_value()) {
        Write(" = ");
        TraverseNode(*node->init);
    }
}

void CodeGen::Traverse(const Sp<ClassDeclaration> &node) {
    Write("class ");
    if (node->id.has_value()) {
        Write((*node->id)->name);
    }
    if (node->super_class.has_value()) {
        Write("extends ");
        TraverseNode(*node->super_class);
        Write(" ");
    }
    TraverseNode(node->body);
}

void CodeGen::Traverse(const Sp<ImportDeclaration> &node) {
    Write("import ");

    std::uint32_t i = 0;
    if (node->specifiers.size() > 0) {
        for (auto& spec : node->specifiers) {
            if (i > 0) {
                Write(", ");
            }
            if (spec->type == SyntaxNodeType::ImportDefaultSpecifier) {
                auto default_ = dynamic_pointer_cast<ImportDefaultSpecifier>(spec);
                Write(default_->local->name, default_);
                i++;
            } else if (spec->type == SyntaxNodeType::ImportNamespaceSpecifier) {
                auto namespace_ = dynamic_pointer_cast<ImportNamespaceSpecifier>(spec);
                UString temp = u"* as " + namespace_->local->name;
                Write(temp, namespace_);
                i++;
            } else {
                break;
            }
        }
        if (i < node->specifiers.size()) {
            Write("{");
            while (true) {
                auto spec = node->specifiers[i];
                auto import_ = dynamic_pointer_cast<ImportSpecifier>(spec);
                Write(import_->imported->name, spec);
                if (import_->imported->name != import_->local->name) {
                    UString temp = u" as " + import_->local->name;
                    Write(temp);
                }
                if (++i < node->specifiers.size()) {
                    Write(", ");
                } else {
                    break;
                }
            }
            Write("}");
        }
        Write(" from ");
    }

    Literal(node->source);
    Write("; ");
}

void CodeGen::Traverse(const Sp<ExportDefaultDeclaration> &node) {
    Write("export default ");
    TraverseNode(node->declaration);
    if (ExpressionPrecedence(node->declaration->type) > 0 &&
        node->declaration->type == SyntaxNodeType::FunctionExpression) {
        Write(";");
    }
}

void CodeGen::Traverse(const Sp<ExportNamedDeclaration> &node) {
    Write("export ");
    if (node->declaration.has_value()) {
        TraverseNode(*node->declaration);
    } else {
        Write("{");
        if (node->specifiers.size() > 0) {
            std::uint32_t i = 0;
            for (auto& spec : node->specifiers) {
                Write(spec->local->name, spec);
                if (spec->local->name != spec->exported->name) {
                    UString temp = u" as " + spec->exported->name;
                    Write(temp);
                }
                if (++i < node->specifiers.size()) {
                    Write(", ");
                } else {
                    break;
                }
            }
        }
        Write("}");
        if (node->source.has_value()) {
            Write(" from ");
            Literal(*node->source);
        }
        Write(";");
    }
}

void CodeGen::Traverse(const Sp<ExportAllDeclaration> &node) {
    Write("export * from ");
    Literal(node->source);
    Write(";");
}

void CodeGen::Traverse(const Sp<MethodDefinition> &node) {
    if (node->static_) {
        Write("static ");
    }
    switch (node->kind) {
        case VarKind::Get:
            Write("get ");
            break;

        case VarKind::Set:
            Write("set ");
            break;

    }
}
