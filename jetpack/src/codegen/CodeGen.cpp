//
// Created by Duzhong Chen on 2019/9/27.
//

#include <iostream>
#include <algorithm>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include "CodeGen.h"
#include "scope/Variable.h"

using namespace std;

#define S_COMMA (config_.minify ? "," : ", ")

namespace jetpack {

#define DEF_OP_PREC(OP_STR, OP_VAL) } else if (op == OP_STR) { \
    return OP_VAL;

    inline int BinaryStrPrecedence(const std::string& op) {
        if (false) {

        DEF_OP_PREC("||", 1)
        DEF_OP_PREC("&&", 2)
        DEF_OP_PREC("|", 3)
        DEF_OP_PREC("^", 4)
        DEF_OP_PREC("&", 5)

        DEF_OP_PREC("==", 6)
        DEF_OP_PREC("!=", 6)
        DEF_OP_PREC("===", 6)
        DEF_OP_PREC("!==", 6)

        DEF_OP_PREC("<", 7)
        DEF_OP_PREC(">", 7)
        DEF_OP_PREC("<=", 7)
        DEF_OP_PREC(">=", 7)

        DEF_OP_PREC(">>", 8)
        DEF_OP_PREC("<<", 8)
        DEF_OP_PREC(">>>", 8)

        DEF_OP_PREC("+", 9)
        DEF_OP_PREC("-", 9)

        DEF_OP_PREC("*", 11)
        DEF_OP_PREC("/", 11)
        DEF_OP_PREC("%", 11)

        } else if (op == "instanceof") {
            return 7;
        } else if (op == "in") {
            return 7;
        }

        return 0;
    }

    CodeGen::CodeGen(
            const CodeGenConfig& config,
            Sp<MappingCollector> mc):
            config_(config), mappingCollector(std::move(mc)) {}

    void CodeGen::AddSnippet(const std::string &content) {
        std::vector<std::string> lines;
        boost::split(lines, content, boost::is_any_of("\n"), boost::token_compress_on);
        for (const auto& line : lines) {
            Write(line);
            WriteLineEnd();
        }
    }

    void CodeGen::Write(const std::string& str) {
        output += str;
        state_.column += UTF16LenOfUtf8(str);
    }

    void CodeGen::WriteLineEnd() {
        if (likely(mappingCollector)) {
            mappingCollector->EndLine();
        }
        if (!config_.minify) {
            output += config_.line_end;
            state_.line++;
            state_.column = 0;
        }
    }

    void CodeGen::WriteIndent() {
        if (config_.minify) return;
        for (std::uint32_t i = 0; i < state_.indent_level; i++) {
            Write(config_.indent);
        }
    }

    int CodeGen::ExpressionPrecedence(const Sp<SyntaxNode>& node) {
        switch (node->type) {
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
            case SyntaxNodeType::MemberExpression:
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

            case SyntaxNodeType::BinaryExpression: {
                auto bin_expr = std::dynamic_pointer_cast<BinaryExpression>(node);

                // is logical
                if (bin_expr->operator_ == "&&" || bin_expr->operator_ == "||") {
                    return 13;
                }

                return 14;
            }

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

    void CodeGen::FormatVariableDeclaration(const Sp<VariableDeclaration> &node) {
        switch (node->kind) {
            case VarKind::Var:
                Write("var ");
                break;

            case VarKind::Let:
                Write("let ");
                break;

            case VarKind::Const:
                Write("const ");
                break;

            default:
                break;

        }

        if (!node->declarations.empty()) {
            for (std::size_t i = 0; i < node->declarations.size(); i++) {
                this->Traverse(node->declarations[i]);
                if (i < node->declarations.size() - 1) {
                    Write(S_COMMA);
                }
            }
        }
    }

    void CodeGen::FormatBinaryExpression(const Sp<Expression> &expr, const Sp<BinaryExpression> &parent, bool is_right) {
        if (ExpressionNeedsParenthesis(expr, parent, is_right)) {
            Write("(");
            TraverseNode(expr);
            Write(")");
        } else {
            TraverseNode(expr);
        }
    }

    bool CodeGen::HasCallExpression(const Sp<SyntaxNode>& node) {
        HasCallExpressionTraverser traverser;
        traverser.TraverseNode(node);
        return traverser.has_call;
    }

    void CodeGen::FormatSequence(std::vector<Sp<SyntaxNode>> &params) {
        Write("(");
        for (std::size_t i = 0; i < params.size(); i++) {
            TraverseNode(params[i]);
            if (i < params.size() - 1) {
                Write(S_COMMA);
            }
        }
        Write(")");
    }

    bool CodeGen::ExpressionNeedsParenthesis(const Sp<Expression> &node, const Sp<BinaryExpression> &parent,
                                             bool is_right) {
        int prec = ExpressionPrecedence(node);
        if (prec == needs_parentheses) {
            return true;
        }
        int parent_prec = ExpressionPrecedence(parent);
        if (prec != parent_prec) {
            return (
                    (!is_right &&
                     prec == 15 &&
                     parent_prec == 14 &&
                     parent->operator_ == "**") ||
                    prec < parent_prec
            );
        }
        if (prec != 13 && prec != 14) {
            return false;
        }
        if (node->type != SyntaxNodeType::BinaryExpression) {
            return false;
        }
        auto cb = dynamic_pointer_cast<BinaryExpression>(node);
        if (cb->operator_ == "**" && parent->operator_ == "**") {
            return !is_right;
        }
//        if (is_right) {
//            return BinaryStrPrecedence(cb->operator_) <= BinaryStrPrecedence(parent->operator_);
//        }
        return BinaryStrPrecedence(cb->operator_) < BinaryStrPrecedence(parent->operator_);
    }

    void CodeGen::Traverse(const Sp<Script> &node) {
        SortComments(node->comments);

        for (auto& stmt : node->body) {
            WriteIndent();
            TraverseNode(stmt);
            WriteLineEnd();
        }

        ordered_comments_.clear();
        ordered_comments_.shrink_to_fit();
    }

    void CodeGen::Traverse(const Sp<Module> &node) {
        auto module_scope = node->scope->CastToModule();
        SortComments(node->comments);

        for (auto& stmt : node->body) {
            WriteIndent();
            TraverseNode(stmt);
            WriteLineEnd();
        }

        ordered_comments_.clear();
        ordered_comments_.shrink_to_fit();
    }

    void CodeGen::Traverse(const Sp<ArrayExpression> &node) {
        Write("[");
        std::size_t count = 0;
        for (auto& elem : node->elements) {
            if (elem.has_value()) {
                TraverseNode(*elem);
            }
            if (count++ < node->elements.size() - 1) {
                output += S_COMMA;
            } else if (!elem.has_value()) {
                output += S_COMMA;
            }
        }
        Write("]");
    }

    void CodeGen::Traverse(const Sp<BlockStatement> &node) {
        Write("{");
        state_.indent_level++;

        if (!node->body.empty()) {
            if (!config_.minify) {
                WriteLineEnd();
            }
            for (auto& elem : node->body) {
                WriteCommentBefore(elem);

                WriteIndent();

                TraverseNode(elem);
                WriteLineEnd();
            }
        }

        state_.indent_level--;
        WriteIndentWith("}");
    }

    void CodeGen::Traverse(const Sp<EmptyStatement> &node) {
        Write(';');
    }

    void CodeGen::Traverse(const Sp<ExpressionStatement> &node) {
        int precedence = ExpressionPrecedence(node);
        if (
                (precedence == needs_parentheses) ||
                (precedence == 3 && node->expression->type == SyntaxNodeType::ObjectPattern)) {
            Write('(');
            TraverseNode(node->expression);
            Write(')');
        } else {
            TraverseNode(node->expression);
        }
        Write(u';');
    }

    void CodeGen::Traverse(const Sp<IfStatement> &node) {
        Write(config_.minify ? "if(" : "if (");
        TraverseNode(node->test);
        Write(config_.minify ? ")" : ") ");
        TraverseNode(node->consequent);
        if (node->alternate.has_value()) {
//            Write(config_.minify ? "else" : " else ");
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
        Write(config_.minify ? "with(" : "with (");
        TraverseNode(node->object);
        Write(config_.minify ? ")" : ") ");
        TraverseNode(node->body);
    }

    void CodeGen::Traverse(const Sp<SwitchStatement> &node) {
        Write(config_.minify ? "switch(" : "switch (");
        TraverseNode(node->discrimiant);
        Write(config_.minify ? "){" : ") {");
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
        Write(config_.minify ? "try" : "try ");
        TraverseNode(node->block);

        if (node->handler.has_value()) {
            auto handler = *node->handler;
            Write(config_.minify ? "catch(" : " catch (");
            TraverseNode(handler->param);
            Write(")");
            TraverseNode(handler->body);
        }

        if (node->finalizer.has_value()) {
            Write(config_.minify ? "finally" : " finally ");
            TraverseNode(*node->finalizer);
        }
    }

    void CodeGen::Traverse(const Sp<WhileStatement> &node) {
        Write(config_.minify ? "while(" : "while (");
        TraverseNode(node->test);
        Write(config_.minify ? ")" : ") ");
        TraverseNode(node->body);
    }

    void CodeGen::Traverse(const Sp<DoWhileStatement> &node) {
        Write(config_.minify ? "do" : "do ");
        TraverseNode(node->body);
        Write(config_.minify ? "while(" : " while (");
        TraverseNode(node->test);
        Write(");");
    }

    void CodeGen::Traverse(const Sp<ForStatement> &node) {
        Write(config_.minify ? "for(" : "for (");
        if (node->init.has_value()) {
            auto init = *node->init;
            if (init->type == SyntaxNodeType::VariableDeclaration) {
                auto decl = dynamic_pointer_cast<VariableDeclaration>(init);
                FormatVariableDeclaration(decl);
            } else {
                TraverseNode(init);
            }
        }
        Write(config_.minify ? ";" : "; ");
        if (node->test.has_value()) {
            TraverseNode(*node->test);
        }
        Write(config_.minify ? ";" : "; ");
        if (node->update.has_value()) {
            TraverseNode(*node->update);
        }
        Write(config_.minify ? ")" : ") ");
        TraverseNode(node->body);
    }

    void CodeGen::Traverse(const Sp<ForInStatement> &node) {
        Write(config_.minify ? "for(" : "for (");
        if (node->left->type == SyntaxNodeType::VariableDeclaration) {
            auto decl = dynamic_pointer_cast<VariableDeclaration>(node->left);
            FormatVariableDeclaration(decl);
        } else {
            TraverseNode(node->left);
        }
        Write(" in ");
        TraverseNode(node->right);
        Write(config_.minify ? ")" : ") ");
        TraverseNode(node->body);
    }

    void CodeGen::Traverse(const Sp<ForOfStatement> & node) {
        Write(config_.minify ? "for(" : "for (");
        if (node->left->type == SyntaxNodeType::VariableDeclaration) {
            auto decl = dynamic_pointer_cast<VariableDeclaration>(node->left);
            FormatVariableDeclaration(decl);
        } else {
            TraverseNode(node->left);
        }
        Write(" of ");
        TraverseNode(node->right);
        Write(config_.minify ? ")" : ") ");
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
            Write("function ");
        }

        if (node->id.has_value()) {
            Write((*node->id)->name);
        }

        FormatSequence(node->params);
        if (!config_.minify) {
            Write(" ");
        }
        TraverseNode(node->body);
    }

    void CodeGen::Traverse(const Sp<FunctionExpression> &node) {
        if (node->async) {
            Write("async ");
        }

        if (node->generator) {
            Write("function*");
        } else {
            Write("function");
        }

        if (node->id.has_value()) {
            Write(" ");
            Write((*node->id)->name);
        }

        FormatSequence(node->params);
        if (!config_.minify) {
            Write(" ");
        }
        TraverseNode(node->body);
    }

    void CodeGen::Traverse(const Sp<VariableDeclaration> &node) {
        FormatVariableDeclaration(node);
        Write(";");
    }

    void CodeGen::Traverse(const Sp<VariableDeclarator> &node) {
        TraverseNode(node->id);
        if (node->init.has_value()) {
            Write(config_.minify ? "=" : " = ");
            TraverseNode(*node->init);
        }
    }

    void CodeGen::Traverse(const Sp<ClassDeclaration> &node) {
        Write("class ");
        if (node->id.has_value()) {
            Write((*node->id)->name);
            Write(" ");
        }
        if (node->super_class.has_value()) {
            Write("extends ");
            TraverseNode(*node->super_class);
            if (!config_.minify) {
                Write(" ");
            }
        }
        TraverseNode(node->body);
    }

    void CodeGen::Traverse(const Sp<ClassBody>& node) {
        Write("{");
        state_.indent_level++;

        if (!node->body.empty()) {
            WriteLineEnd();
            for (auto& elem : node->body) {
                WriteCommentBefore(elem);
                WriteIndent();
                TraverseNode(elem);
                WriteLineEnd();
            }
        }

        state_.indent_level--;
        WriteIndentWith("}");
    }

    void CodeGen::Traverse(const Sp<ImportDeclaration> &node) {
        Write("import ");

        std::uint32_t i = 0;
        if (node->specifiers.size() > 0) {
            for (auto& spec : node->specifiers) {
                if (i > 0) {
                    Write(config_.minify ? "," : ", ");
                }
                if (spec->type == SyntaxNodeType::ImportDefaultSpecifier) {
                    auto default_ = dynamic_pointer_cast<ImportDefaultSpecifier>(spec);
                    Write(default_->local->name, default_);
                    i++;
                } else if (spec->type == SyntaxNodeType::ImportNamespaceSpecifier) {
                    auto namespace_ = dynamic_pointer_cast<ImportNamespaceSpecifier>(spec);
                    std::string temp = "* as " + namespace_->local->name;
                    Write(temp, namespace_);
                    i++;
                } else {
                    break;
                }
            }
            if (i < node->specifiers.size()) {
                Write(config_.minify ? "{" : "{ ");
                while (true) {
                    auto spec = node->specifiers[i];
                    auto import_ = dynamic_pointer_cast<ImportSpecifier>(spec);
                    Write(import_->imported->name, spec);
                    if (import_->imported->name != import_->local->name) {
                        std::string temp = " as " + import_->local->name;
                        Write(temp);
                    }
                    if (++i < node->specifiers.size()) {
                        Write(S_COMMA);
                    } else {
                        break;
                    }
                }
                Write(config_.minify ? "}" : " }");
            }
            Write(" from ");
        }

        this->Traverse(node->source);
        Write(";");
    }

    void CodeGen::Traverse(const Sp<ExportDefaultDeclaration> &node) {
        Write("export default ");
        TraverseNode(node->declaration);
        if (ExpressionPrecedence(node->declaration) > 0 &&
            node->declaration->type == SyntaxNodeType::FunctionExpression) {
            Write(";");
        }
    }

    void CodeGen::Traverse(const Sp<ExportNamedDeclaration> &node) {
        Write("export ");
        if (node->declaration.has_value()) {
            TraverseNode(*node->declaration);
        } else {
            Write(config_.minify ? "{" : "{ ");
            if (node->specifiers.size() > 0) {
                std::uint32_t i = 0;
                for (auto& spec : node->specifiers) {
                    Write(spec->local->name, spec);
                    if (spec->local->name != spec->exported->name) {
                        std::string temp = " as " + spec->exported->name;
                        Write(temp);
                    }
                    if (++i < node->specifiers.size()) {
                        Write(S_COMMA);
                    } else {
                        break;
                    }
                }
            }
            Write(config_.minify ? "}" : " }");
            if (node->source.has_value()) {
                Write(" from ");
                this->Traverse(*node->source);
            }
            Write(";");
        }
    }

    void CodeGen::Traverse(const Sp<ExportAllDeclaration> &node) {
        Write("export * from ");
        this->Traverse(node->source);
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

            default:
                break;

        }

        if (node->key.has_value() && node->value.has_value()) {
            auto fun_expr = std::dynamic_pointer_cast<FunctionExpression>(*node->value);
            if (!fun_expr) {
                return;
            }

            if (fun_expr->IsAsync()) {
                Write("async ");
            }
            if (fun_expr->IsGenerator()) {
                Write("*");
            }
            if (fun_expr->IsComputed()) {
                Write('[');
                TraverseNode(*node->key);
                Write(']');
            } else {
                TraverseNode(*node->key);
            }
            FormatSequence(fun_expr->params);
            if (!config_.minify) {
                Write(" ");
            }
            TraverseNode(fun_expr->body);
        }
    }

    void CodeGen::Traverse(const Sp<ArrowFunctionExpression> &node) {
        if (node->async) {
            Write("async ", node);
        }
        auto& params = node->params;
        if (!params.empty()) {
            if (params.size() == 1 && params[0]->type == SyntaxNodeType::Identifier) {
                auto id = dynamic_pointer_cast<Identifier>(params[0]);
                Write(id->name, id);
            } else {
                FormatSequence(params);
            }
        } else {
            Write("()");
        }
        Write(config_.minify ? "=>" : " => ");
        if (node->body->type == SyntaxNodeType::ObjectExpression) {
            Write("(");
            Sp<ObjectExpression> oe = dynamic_pointer_cast<ObjectExpression>(node->body);
            this->Traverse(oe);
            Write(")");
        } else {
            TraverseNode(node->body);
        }
    }

    void CodeGen::Traverse(const Sp<ThisExpression> &node) {
        Write("this", node);
    }

    void CodeGen::Traverse(const Sp<Super> &node) {
        Write("super", node);
    }

    void CodeGen::Traverse(const Sp<RestElement> &node) {
        Write("...");
        TraverseNode(node->argument);
    }

    void CodeGen::Traverse(const Sp<SpreadElement> &node) {
        Write("...");
        TraverseNode(node->argument);
    }

    void CodeGen::Traverse(const Sp<YieldExpression> &node) {
        if (node->delegate) {
            Write("yield*");
        } else {
            Write("yield");
        }

        if (node->argument.has_value()) {
            Write(" ");
            TraverseNode(*node->argument);
        }
    }

    void CodeGen::Traverse(const Sp<AwaitExpression> &node) {
        Write("await ");
        TraverseNode(node->argument);
    }

    void CodeGen::Traverse(const Sp<TemplateLiteral> &node) {
        Write("`");
        for (std::size_t i = 0; i < node->expressions.size(); i++) {
            auto expr = node->expressions[i];
            Write(node->quasis[i]->cooked);
            Write("${");
            TraverseNode(expr);
            Write("}");
        }
        Write(node->quasis[node->quasis.size() - 1]->cooked);
        Write("`");
    }

    void CodeGen::Traverse(const Sp<TaggedTemplateExpression> &node) {
        TraverseNode(node->tag);
        TraverseNode(node->quasi);
    }

    void CodeGen::Traverse(const Sp<ObjectExpression> &node) {
        state_.indent_level++;
        Write("{");
        if (!node->properties.empty()) {
            WriteLineEnd();
            std::string comma = "," + config_.line_end;
            std::size_t i = 0;
            while (true) {
                auto prop = node->properties[i];
                WriteIndent();
                TraverseNode(prop);
                if (++i < node->properties.size()) {
                    Write(comma);
                } else {
                    break;
                }
            }
            WriteLineEnd();
        }
        state_.indent_level--;
        WriteIndentWith("}");
    }

    void CodeGen::Traverse(const Sp<Property> &node) {
        switch (node->kind) {
            case VarKind::Get: {
                Write("get ");
                TraverseNode(node->key);
                if (!node->value.has_value()) return;
                auto fun = std::dynamic_pointer_cast<FunctionExpression>(*node->value);
                if (fun == nullptr) return;

                FormatSequence(fun->params);
                if (!config_.minify) {
                    Write(" ");
                }
                TraverseNode(fun->body);
                break;
            }

            case VarKind::Set: {
                Write("set ");
                TraverseNode(node->key);
                if (!node->value.has_value()) return;
                auto fun = std::dynamic_pointer_cast<FunctionExpression>(*node->value);
                if (fun == nullptr) return;

                FormatSequence(fun->params);
                if (!config_.minify) {
                    Write(" ");
                }
                TraverseNode(fun->body);
                break;
            }

            default: {
                bool shorthand = node->shorthand;
                if (node->value.has_value()
                    && node->key->type == SyntaxNodeType::Identifier
                    && (*node->value)->type == SyntaxNodeType::Identifier) {
                    auto key_id = std::dynamic_pointer_cast<Identifier>(node->key);
                    auto val_id = std::dynamic_pointer_cast<Identifier>(*node->value);
                    shorthand = key_id->name == val_id->name;
                }
                if (!shorthand) {
                    if (node->computed) {
                        Write("[");
                        TraverseNode(node->key);
                        Write("]");
                    } else {
                        TraverseNode(node->key);
                    }
                }
                if (node->value.has_value()) {
                    if (!shorthand) {
                        Write(config_.minify ? ":" : ": ");
                    }
                    TraverseNode(*node->value);
                }
                break;
            }
        }
    }

    void CodeGen::Traverse(const Sp<SequenceExpression> &node) {
        std::vector<Sp<SyntaxNode>> nodes;
        for (auto& i : node->expressions) {
            nodes.push_back(i);
        }
        FormatSequence(nodes);
    }

    void CodeGen::Traverse(const Sp<UnaryExpression> &node) {
        if (node->prefix) {
            Write(node->operator_);
            if (node->operator_.size() > 1) {
                Write(" ");
            }
            if (ExpressionPrecedence(node->argument) <
                ExpressionPrecedence(std::make_shared<UnaryExpression>())
                    ) {
                Write("(");
                TraverseNode(node->argument);
                Write(")");
            } else {
                TraverseNode(node->argument);
            }
        } else {
            TraverseNode(node->argument);
            Write(node->operator_);
        }
    }

    void CodeGen::Traverse(const Sp<AssignmentExpression> &node) {
        TraverseNode(node->left);
        if (config_.minify) {
            Write(node->operator_);
        } else {
            Write(std::string(" ") + node->operator_ + " ");
        }
        TraverseNode(node->right);
    }

    void CodeGen::Traverse(const Sp<AssignmentPattern> &node) {
        TraverseNode(node->left);
        Write(config_.minify ? "=" : " = ");
        TraverseNode(node->right);
    }

    void CodeGen::Traverse(const Sp<BinaryExpression> &node) {
        bool is_in = node->operator_ == "in";
        if (is_in) {
            Write("(");
        }
        FormatBinaryExpression(node->left, node, false);
        if (config_.minify && node->operator_ != "in" && node->operator_ != "instanceof") {
            Write(node->operator_);
        } else {
            Write(" " + node->operator_ + " ");
        }
        FormatBinaryExpression(node->right, node, true);
        if (is_in) {
            Write(")");
        }
    }

    void CodeGen::Traverse(const Sp<ConditionalExpression> &node) {
        if (ExpressionPrecedence(node->test) >
            ExpressionPrecedence(std::make_shared<ConditionalExpression>())) {
            TraverseNode(node->test);
        } else {
            Write("(");
            TraverseNode(node->test);
            Write(")");
        }
        Write(config_.minify ? "?" : " ? ");
        TraverseNode(node->consequent);
        Write(config_.minify ? ":" : " : ");
        TraverseNode(node->alternate);
    }

    void CodeGen::Traverse(const Sp<NewExpression> &node) {
        Write("new ");
        if (ExpressionPrecedence(node->callee) <
            ExpressionPrecedence(std::make_shared<CallExpression>()) ||
            HasCallExpression(node->callee)) {
            Write("(");
            TraverseNode(node->callee);
            Write(")");
        } else {
            TraverseNode(node->callee);
        }
        FormatSequence(node->arguments);
    }

    void CodeGen::Traverse(const Sp<MemberExpression> &node) {
        if (ExpressionPrecedence(node->object) < ExpressionPrecedence(std::make_shared<MemberExpression>())) {
            Write('(');
            TraverseNode(node->object);
            Write(')');
        } else {
            TraverseNode(node->object);
        }
        if (node->computed) {
            Write('[');
            TraverseNode(node->property);
            Write(']');
        } else {
            Write('.');
            TraverseNode(node->property);
        }
    }

    void CodeGen::Traverse(const Sp<CallExpression> &node) {
        if (ExpressionPrecedence(node->callee) <
            ExpressionPrecedence(std::make_shared<CallExpression>())) {
            Write("(");
            TraverseNode(node->callee);
            Write(")");
        } else {
            TraverseNode(node->callee);
        }
        FormatSequence(node->arguments);
    }

    void CodeGen::Traverse(const Sp<Identifier> &node) {
        if (likely(mappingCollector)) {
            mappingCollector->AddMapping(node->name, node->location, state_.column);
        }
        if (config_.minify && node->name == "undefined") {
            Write("void 0");
        } else {
            Write(node->name, node);
        }
    }

    void CodeGen::Traverse(const Sp<Literal> &lit) {
        if (config_.minify && lit->ty == Literal::Ty::Boolean) {
            if (lit->raw == "true") {
                Write("!0");
            } else {
                Write("!1");
            }
        } else {
            Write(lit->raw, lit);
        }
    }

    void CodeGen::Traverse(const Sp<RegexLiteral> &lit) {
        Write(lit->value, lit);
    }

    void CodeGen::Traverse(const Sp<UpdateExpression>& update) {
        if (update->prefix) {
            Write(update->operator_);
            TraverseNode(update->argument);
        } else {
            TraverseNode(update->argument);
            Write(update->operator_);
        }
    }

    void CodeGen::Traverse(const Sp<ObjectPattern>& node) {
        Write(config_.minify ? "{" : "{ ");
        for (std::size_t i = 0; ;) {
            TraverseNode(node->properties[i]);
            if (++i < node->properties.size()) {
                Write(", ");
            } else {
                break;
            }
        }
        Write(config_.minify ? "}" : " }");
    }

    void CodeGen::SortComments(std::vector<Sp<Comment>> comments) {
        std::sort(comments.begin(), comments.end(), [](const Sp<Comment>& a, const Sp<Comment>& b) {
            return a->range_.first < b->range_.first;
        });

//    for (auto& i : comments) {
//        std::cout << "comment " << i->range_.first << " " << bundle-utils::To_UTF8(i->value_) << std::endl;
//    }

        for (auto& i : comments) {
            ordered_comments_.push_back(i);
        }
    }

    void CodeGen::WriteTopCommentBefore_(const Sp<SyntaxNode> &node) {
        while (!ordered_comments_.empty()) {
            auto& top = ordered_comments_.front();
            if (top->range_.second < node->range.first) {
                if (top->multi_line_) {
                    Write("/*");
                    Write(top->value_);
                    Write("*/");
                } else {
                    WriteIndent();
                    Write("//");
                    Write(top->value_);
                }
                ordered_comments_.pop_front();
            } else {
                break;
            }
        }
    }

}

