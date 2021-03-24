//
// Created by Duzhong Chen on 2019/9/27.
//

#include "CodeGen.h"
#include <iostream>
#include <algorithm>
#include <scope/Variable.h>

using namespace std;

#define S_COMMA (config_.minify ? u"," : u", ")

namespace jetpack {

#define DEF_OP_PREC(OP_STR, OP_VAL) } else if (op == OP_STR) { \
    return OP_VAL;

    inline int BinaryStrPrecedence(const UString& op) {
        if (false) {

        DEF_OP_PREC(u"||", 1)
        DEF_OP_PREC(u"&&", 2)
        DEF_OP_PREC(u"|", 3)
        DEF_OP_PREC(u"^", 4)
        DEF_OP_PREC(u"&", 5)

        DEF_OP_PREC(u"==", 6)
        DEF_OP_PREC(u"!=", 6)
        DEF_OP_PREC(u"===", 6)
        DEF_OP_PREC(u"!==", 6)

        DEF_OP_PREC(u"<", 7)
        DEF_OP_PREC(u">", 7)
        DEF_OP_PREC(u"<=", 7)
        DEF_OP_PREC(u">=", 7)

        DEF_OP_PREC(u">>", 8)
        DEF_OP_PREC(u"<<", 8)
        DEF_OP_PREC(u">>>", 8)

        DEF_OP_PREC(u"+", 9)
        DEF_OP_PREC(u"-", 9)

        DEF_OP_PREC(u"*", 11)
        DEF_OP_PREC(u"/", 11)
        DEF_OP_PREC(u"%", 11)

        } else if (op == u"instanceof") {
            return 7;
        } else if (op == u"in") {
            return 7;
        }

        return 0;
    }

    CodeGen::CodeGen(const Config& config, OutputStream& output_stream):
            config_(config), output(output_stream) {}

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
                if (bin_expr->operator_ == u"&&" || bin_expr->operator_ == u"||") {
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
                Write(u"var ");
                break;

            case VarKind::Let:
                Write(u"let ");
                break;

            case VarKind::Const:
                Write(u"const ");
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
            Write(u"(");
            TraverseNode(expr);
            Write(u")");
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
        Write(u"(");
        for (std::size_t i = 0; i < params.size(); i++) {
            TraverseNode(params[i]);
            if (i < params.size() - 1) {
                Write(S_COMMA);
            }
        }
        Write(u")");
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
                     parent->operator_ == u"**") ||
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
        if (cb->operator_ == u"**" && parent->operator_ == u"**") {
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
        Write(u"[");
        std::size_t count = 0;
        for (auto& elem : node->elements) {
            if (elem.has_value()) {
                TraverseNode(*elem);
            }
            if (count++ < node->elements.size() - 1) {
                output << S_COMMA;
            } else if (!elem.has_value()) {
                output << S_COMMA;
            }
        }
        Write(u"]");
    }

    void CodeGen::Traverse(const Sp<BlockStatement> &node) {
        Write(u"{");
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
        WriteIndentWith(u"}");
    }

    void CodeGen::Traverse(const Sp<EmptyStatement> &node) {
        Write(u';');
    }

    void CodeGen::Traverse(const Sp<ExpressionStatement> &node) {
        int precedence = ExpressionPrecedence(node);
        if (
                (precedence == needs_parentheses) ||
                (precedence == 3 && node->expression->type == SyntaxNodeType::ObjectPattern)) {
            Write(u'(');
            TraverseNode(node->expression);
            Write(u')');
        } else {
            TraverseNode(node->expression);
        }
        Write(u';');
    }

    void CodeGen::Traverse(const Sp<IfStatement> &node) {
        Write(config_.minify ? u"if(" : u"if (");
        TraverseNode(node->test);
        Write(config_.minify ? u")" : u") ");
        TraverseNode(node->consequent);
        if (node->alternate.has_value()) {
//            Write(config_.minify ? "else" : " else ");
            Write(u" else ");
            TraverseNode(*node->alternate);
        }
    }

    void CodeGen::Traverse(const Sp<LabeledStatement> &node) {
        TraverseNode(node->label);
        Write(u": ");
        TraverseNode(node->body);
    }

    void CodeGen::Traverse(const Sp<BreakStatement> &node) {
        Write(u"break");
        if (node->label.has_value()) {
            Write(u" ");
            TraverseNode(*node->label);
        }
        Write(u";");
    }

    void CodeGen::Traverse(const Sp<ContinueStatement> &node) {
        Write(u"continue");
        if (node->label.has_value()) {
            Write(u" ");
            TraverseNode(*node->label);
        }
        Write(u";");
    }

    void CodeGen::Traverse(const Sp<WithStatement> &node) {
        Write(config_.minify ? u"with(" : u"with (");
        TraverseNode(node->object);
        Write(config_.minify ? u")" : u") ");
        TraverseNode(node->body);
    }

    void CodeGen::Traverse(const Sp<SwitchStatement> &node) {
        Write(config_.minify ? u"switch(" : u"switch (");
        TraverseNode(node->discrimiant);
        Write(config_.minify ? u"){" : u") {");
        WriteLineEnd();
        state_.indent_level++;

        for (auto& case_ : node->cases) {

            WriteIndent();
            if (case_->test.has_value()) {
                Write(u"case ");
                TraverseNode(*case_->test);
                Write(u":");
                WriteLineEnd();
            } else {
                Write(u"default:");
                WriteLineEnd();
            }

            for (auto& cons : case_->consequent) {
                WriteIndent();
                TraverseNode(cons);
                WriteLineEnd();
            }

        }

        state_.indent_level--;
        WriteIndentWith(u"}");
    }

    void CodeGen::Traverse(const Sp<ReturnStatement> &node) {
        Write(u"return");
        if (node->argument.has_value()) {
            Write(u" ");
            TraverseNode(*node->argument);
        }
        Write(u";");
    }

    void CodeGen::Traverse(const Sp<ThrowStatement> &node) {
        Write(u"throw ");
        TraverseNode(node->argument);
        Write(u";");
    }

    void CodeGen::Traverse(const Sp<TryStatement> &node) {
        Write(config_.minify ? u"try" : u"try ");
        TraverseNode(node->block);

        if (node->handler.has_value()) {
            auto handler = *node->handler;
            Write(config_.minify ? u"catch(" : u" catch (");
            TraverseNode(handler->param);
            Write(u")");
            TraverseNode(handler->body);
        }

        if (node->finalizer.has_value()) {
            Write(config_.minify ? u"finally" : u" finally ");
            TraverseNode(*node->finalizer);
        }
    }

    void CodeGen::Traverse(const Sp<WhileStatement> &node) {
        Write(config_.minify ? u"while(" : u"while (");
        TraverseNode(node->test);
        Write(config_.minify ? u")" : u") ");
        TraverseNode(node->body);
    }

    void CodeGen::Traverse(const Sp<DoWhileStatement> &node) {
        Write(config_.minify ? u"do" : u"do ");
        TraverseNode(node->body);
        Write(config_.minify ? u"while(" : u" while (");
        TraverseNode(node->test);
        Write(u");");
    }

    void CodeGen::Traverse(const Sp<ForStatement> &node) {
        Write(config_.minify ? u"for(" : u"for (");
        if (node->init.has_value()) {
            auto init = *node->init;
            if (init->type == SyntaxNodeType::VariableDeclaration) {
                auto decl = dynamic_pointer_cast<VariableDeclaration>(init);
                FormatVariableDeclaration(decl);
            } else {
                TraverseNode(init);
            }
        }
        Write(config_.minify ? u";" : u"; ");
        if (node->test.has_value()) {
            TraverseNode(*node->test);
        }
        Write(config_.minify ? u";" : u"; ");
        if (node->update.has_value()) {
            TraverseNode(*node->update);
        }
        Write(config_.minify ? u")" : u") ");
        TraverseNode(node->body);
    }

    void CodeGen::Traverse(const Sp<ForInStatement> &node) {
        Write(config_.minify ? u"for(" : u"for (");
        if (node->left->type == SyntaxNodeType::VariableDeclaration) {
            auto decl = dynamic_pointer_cast<VariableDeclaration>(node->left);
            FormatVariableDeclaration(decl);
        } else {
            TraverseNode(node->left);
        }
        Write(u" in ");
        TraverseNode(node->right);
        Write(config_.minify ? u")" : u") ");
        TraverseNode(node->body);
    }

    void CodeGen::Traverse(const Sp<ForOfStatement> & node) {
        Write(config_.minify ? u"for(" : u"for (");
        if (node->left->type == SyntaxNodeType::VariableDeclaration) {
            auto decl = dynamic_pointer_cast<VariableDeclaration>(node->left);
            FormatVariableDeclaration(decl);
        } else {
            TraverseNode(node->left);
        }
        Write(u" of ");
        TraverseNode(node->right);
        Write(config_.minify ? u")" : u") ");
        TraverseNode(node->body);
    }

    void CodeGen::Traverse(const Sp<DebuggerStatement> &node) {
        Write(u"debugger;");
        WriteLineEnd();
    }

    void CodeGen::Traverse(const Sp<FunctionDeclaration> &node) {
        if (node->async) {
            Write(u"async ");
        }

        if (node->generator) {
            Write(u"function* ");
        } else {
            Write(u"function ");
        }

        if (node->id.has_value()) {
            Write((*node->id)->name);
        }

        FormatSequence(node->params);
        if (!config_.minify) {
            Write(u" ");
        }
        TraverseNode(node->body);
    }

    void CodeGen::Traverse(const Sp<FunctionExpression> &node) {
        if (node->async) {
            Write(u"async ");
        }

        if (node->generator) {
            Write(u"function*");
        } else {
            Write(u"function");
        }

        if (node->id.has_value()) {
            Write(u" ");
            Write((*node->id)->name);
        }

        FormatSequence(node->params);
        if (!config_.minify) {
            Write(u" ");
        }
        TraverseNode(node->body);
    }

    void CodeGen::Traverse(const Sp<VariableDeclaration> &node) {
        FormatVariableDeclaration(node);
        Write(u";");
    }

    void CodeGen::Traverse(const Sp<VariableDeclarator> &node) {
        TraverseNode(node->id);
        if (node->init.has_value()) {
            Write(config_.minify ? u"=" : u" = ");
            TraverseNode(*node->init);
        }
    }

    void CodeGen::Traverse(const Sp<ClassDeclaration> &node) {
        Write(u"class ");
        if (node->id.has_value()) {
            Write((*node->id)->name);
            Write(u" ");
        }
        if (node->super_class.has_value()) {
            Write(u"extends ");
            TraverseNode(*node->super_class);
            if (!config_.minify) {
                Write(u" ");
            }
        }
        TraverseNode(node->body);
    }

    void CodeGen::Traverse(const Sp<ClassBody>& node) {
        Write(u"{");
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
        WriteIndentWith(u"}");
    }

    void CodeGen::Traverse(const Sp<ImportDeclaration> &node) {
        Write(u"import ");

        std::uint32_t i = 0;
        if (node->specifiers.size() > 0) {
            for (auto& spec : node->specifiers) {
                if (i > 0) {
                    Write(config_.minify ? u"," : u", ");
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
                Write(config_.minify ? u"{" : u"{ ");
                while (true) {
                    auto spec = node->specifiers[i];
                    auto import_ = dynamic_pointer_cast<ImportSpecifier>(spec);
                    Write(import_->imported->name, spec);
                    if (import_->imported->name != import_->local->name) {
                        UString temp = u" as " + import_->local->name;
                        Write(temp);
                    }
                    if (++i < node->specifiers.size()) {
                        Write(S_COMMA);
                    } else {
                        break;
                    }
                }
                Write(config_.minify ? u"}" : u" }");
            }
            Write(u" from ");
        }

        this->Traverse(node->source);
        Write(u";");
    }

    void CodeGen::Traverse(const Sp<ExportDefaultDeclaration> &node) {
        Write(u"export default ");
        TraverseNode(node->declaration);
        if (ExpressionPrecedence(node->declaration) > 0 &&
            node->declaration->type == SyntaxNodeType::FunctionExpression) {
            Write(u";");
        }
    }

    void CodeGen::Traverse(const Sp<ExportNamedDeclaration> &node) {
        Write(u"export ");
        if (node->declaration.has_value()) {
            TraverseNode(*node->declaration);
        } else {
            Write(config_.minify ? u"{" : u"{ ");
            if (node->specifiers.size() > 0) {
                std::uint32_t i = 0;
                for (auto& spec : node->specifiers) {
                    Write(spec->local->name, spec);
                    if (spec->local->name != spec->exported->name) {
                        UString temp = u" as " + spec->exported->name;
                        Write(temp);
                    }
                    if (++i < node->specifiers.size()) {
                        Write(S_COMMA);
                    } else {
                        break;
                    }
                }
            }
            Write(config_.minify ? u"}" : u" }");
            if (node->source.has_value()) {
                Write(u" from ");
                this->Traverse(*node->source);
            }
            Write(u";");
        }
    }

    void CodeGen::Traverse(const Sp<ExportAllDeclaration> &node) {
        Write(u"export * from ");
        this->Traverse(node->source);
        Write(u";");
    }

    void CodeGen::Traverse(const Sp<MethodDefinition> &node) {
        if (node->static_) {
            Write(u"static ");
        }
        switch (node->kind) {
            case VarKind::Get:
                Write(u"get ");
                break;

            case VarKind::Set:
                Write(u"set ");
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
                Write(u"async ");
            }
            if (fun_expr->IsGenerator()) {
                Write(u"*");
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
                Write(u" ");
            }
            TraverseNode(fun_expr->body);
        }
    }

    void CodeGen::Traverse(const Sp<ArrowFunctionExpression> &node) {
        if (node->async) {
            Write(u"async ", node);
        }
        auto& params = node->params;
        if (!params.empty()) {
            if (params.size() == 1 && params[0]->type == SyntaxNodeType::Identifier) {
                auto id = dynamic_pointer_cast<Identifier>(params[0]);
                Write(id->name, id);
            } else {
                FormatSequence(params);
            }
        }
        Write(config_.minify ? u"=>" : u" => ");
        if (node->body->type == SyntaxNodeType::ObjectExpression) {
            Write(u"(");
            Sp<ObjectExpression> oe = dynamic_pointer_cast<ObjectExpression>(node->body);
            this->Traverse(oe);
            Write(u")");
        } else {
            TraverseNode(node->body);
        }
    }

    void CodeGen::Traverse(const Sp<ThisExpression> &node) {
        Write(u"this", node);
    }

    void CodeGen::Traverse(const Sp<Super> &node) {
        Write(u"super", node);
    }

    void CodeGen::Traverse(const Sp<RestElement> &node) {
        Write(u"...");
        TraverseNode(node->argument);
    }

    void CodeGen::Traverse(const Sp<SpreadElement> &node) {
        Write(u"...");
        TraverseNode(node->argument);
    }

    void CodeGen::Traverse(const Sp<YieldExpression> &node) {
        if (node->delegate) {
            Write(u"yield*");
        } else {
            Write(u"yield");
        }

        if (node->argument.has_value()) {
            Write(u" ");
            TraverseNode(*node->argument);
        }
    }

    void CodeGen::Traverse(const Sp<AwaitExpression> &node) {
        Write(u"await ");
        TraverseNode(node->argument);
    }

    void CodeGen::Traverse(const Sp<TemplateLiteral> &node) {
        Write(u"`");
        for (std::size_t i = 0; i < node->expressions.size(); i++) {
            auto expr = node->expressions[i];
            Write(node->quasis[i]->cooked);
            Write(u"${");
            TraverseNode(expr);
            Write(u"}");
        }
        Write(node->quasis[node->quasis.size() - 1]->cooked);
        Write(u"`");
    }

    void CodeGen::Traverse(const Sp<TaggedTemplateExpression> &node) {
        TraverseNode(node->tag);
        TraverseNode(node->quasi);
    }

    void CodeGen::Traverse(const Sp<ObjectExpression> &node) {
        state_.indent_level++;
        Write(u"{");
        if (!node->properties.empty()) {
            WriteLineEnd();
            UString comma = u"," + config_.line_end;
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
        WriteIndentWith(u"}");
    }

    void CodeGen::Traverse(const Sp<Property> &node) {
        switch (node->kind) {
            case VarKind::Get: {
                Write(u"get ");
                TraverseNode(node->key);
                if (!node->value.has_value()) return;
                auto fun = std::dynamic_pointer_cast<FunctionExpression>(*node->value);
                if (fun == nullptr) return;

                FormatSequence(fun->params);
                if (!config_.minify) {
                    Write(u" ");
                }
                TraverseNode(fun->body);
                break;
            }

            case VarKind::Set: {
                Write(u"set ");
                TraverseNode(node->key);
                if (!node->value.has_value()) return;
                auto fun = std::dynamic_pointer_cast<FunctionExpression>(*node->value);
                if (fun == nullptr) return;

                FormatSequence(fun->params);
                if (!config_.minify) {
                    Write(u" ");
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
                        Write(u"[");
                        TraverseNode(node->key);
                        Write(u"]");
                    } else {
                        TraverseNode(node->key);
                    }
                }
                if (node->value.has_value()) {
                    if (!shorthand) {
                        Write(config_.minify ? u":" : u": ");
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
                Write(u" ");
            }
            if (ExpressionPrecedence(node->argument) <
                ExpressionPrecedence(std::make_shared<UnaryExpression>())
                    ) {
                Write(u"(");
                TraverseNode(node->argument);
                Write(u")");
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
            Write(UString(u" ") + node->operator_ + u" ");
        }
        TraverseNode(node->right);
    }

    void CodeGen::Traverse(const Sp<AssignmentPattern> &node) {
        TraverseNode(node->left);
        Write(config_.minify ? u"=" : u" = ");
        TraverseNode(node->right);
    }

    void CodeGen::Traverse(const Sp<BinaryExpression> &node) {
        bool is_in = node->operator_ == u"in";
        if (is_in) {
            Write(u"(");
        }
        FormatBinaryExpression(node->left, node, false);
        if (config_.minify && node->operator_ != u"in" && node->operator_ != u"instanceof") {
            Write(node->operator_);
        } else {
            Write(u" " + node->operator_ + u" ");
        }
        FormatBinaryExpression(node->right, node, true);
        if (is_in) {
            Write(u")");
        }
    }

    void CodeGen::Traverse(const Sp<ConditionalExpression> &node) {
        if (ExpressionPrecedence(node->test) >
            ExpressionPrecedence(std::make_shared<ConditionalExpression>())) {
            TraverseNode(node->test);
        } else {
            Write(u"(");
            TraverseNode(node->test);
            Write(u")");
        }
        Write(config_.minify ? u"?" : u" ? ");
        TraverseNode(node->consequent);
        Write(config_.minify ? u":" : u" : ");
        TraverseNode(node->alternate);
    }

    void CodeGen::Traverse(const Sp<NewExpression> &node) {
        Write(u"new ");
        if (ExpressionPrecedence(node->callee) <
            ExpressionPrecedence(std::make_shared<CallExpression>()) ||
            HasCallExpression(node->callee)) {
            Write(u"(");
            TraverseNode(node->callee);
            Write(u")");
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
            Write(u"(");
            TraverseNode(node->callee);
            Write(u")");
        } else {
            TraverseNode(node->callee);
        }
        FormatSequence(node->arguments);
    }

    void CodeGen::Traverse(const Sp<Identifier> &node) {
        if (config_.minify && node->name == u"undefined") {
            Write(u"void 0");
        } else {
            Write(node->name, node);
        }
    }

    void CodeGen::Traverse(const Sp<Literal> &lit) {
        if (config_.minify && lit->ty == Literal::Ty::Boolean) {
            if (lit->raw == u"true") {
                Write(u"!0");
            } else {
                Write(u"!1");
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
        Write(config_.minify ? u"{" : u"{ ");
        for (std::size_t i = 0; ;) {
            TraverseNode(node->properties[i]);
            if (++i < node->properties.size()) {
                Write(u", ");
            } else {
                break;
            }
        }
        Write(config_.minify ? u"}" : u" }");
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
                    Write(u"/*");
                    Write(top->value_);
                    Write(u"*/");
                } else {
                    WriteIndent();
                    Write(u"//");
                    Write(top->value_);
                }
                ordered_comments_.pop_front();
            } else {
                break;
            }
        }
    }

}

