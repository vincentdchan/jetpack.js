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

#define IS_MINIFY (flags_.testFlag(JETPACK_MINIFY))
#define S_COMMA (IS_MINIFY ? "," : ", ")

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
            JetpackFlags flags,
            Sp<MappingCollector> mc):
            flags_(flags), mappingCollector(std::move(mc)) {}

    CodeGenResult CodeGen::CodeGenModule(Module &node, JetpackFlags flags,
                                         Sp<MappingCollector> sourcemap_generator) {
        CodeGen inst(flags, std::move(sourcemap_generator));
        inst.Traverse(node);
        return inst.GetResult();
    }

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

    void CodeGen::Write(const std::string& str, SyntaxNode& node) {
        if (likely(mappingCollector)) {
            mappingCollector->AddMapping(str, node.location, state_.column);
        }
        Write(str);
    }

    void CodeGen::WriteLineEnd() {
        if (likely(mappingCollector)) {
            mappingCollector->EndLine();
        }
        if (!IS_MINIFY) {
            output += line_end_;
            state_.line++;
            state_.column = 0;
        }
    }

    void CodeGen::WriteIndent() {
        if (IS_MINIFY) return;
        for (std::uint32_t i = 0; i < state_.indent_level; i++) {
            Write(indent_);
        }
    }

    int CodeGen::ExpressionPrecedence(SyntaxNode& node) {
        switch (node.type) {
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
                auto bin_expr = dynamic_cast<BinaryExpression*>(&node);

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

    void CodeGen::FormatVariableDeclaration(VariableDeclaration& node) {
        switch (node.kind) {
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

        if (!node.declarations.empty()) {
            for (std::size_t i = 0; i < node.declarations.size(); i++) {
                this->Traverse(*node.declarations[i]);
                if (i < node.declarations.size() - 1) {
                    Write(S_COMMA);
                }
            }
        }
    }

    void CodeGen::FormatBinaryExpression(Expression &expr, BinaryExpression& parent, bool is_right) {
        if (ExpressionNeedsParenthesis(expr, parent, is_right)) {
            Write("(");
            TraverseNode(expr);
            Write(")");
        } else {
            TraverseNode(expr);
        }
    }

    bool CodeGen::HasCallExpression(SyntaxNode* node) {
        HasCallExpressionTraverser traverser;
        traverser.TraverseNode(node);
        return traverser.has_call;
    }

    void CodeGen::FormatSequence(std::vector<SyntaxNode*> &params) {
        Write("(");
        for (std::size_t i = 0; i < params.size(); i++) {
            TraverseNode(*params[i]);
            if (i < params.size() - 1) {
                Write(S_COMMA);
            }
        }
        Write(")");
    }

    bool CodeGen::ExpressionNeedsParenthesis(Expression& node, BinaryExpression &parent,
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
                     parent.operator_ == "**") ||
                    prec < parent_prec
            );
        }
        if (prec != 13 && prec != 14) {
            return false;
        }
        if (node.type != SyntaxNodeType::BinaryExpression) {
            return false;
        }
        auto cb = dynamic_cast<BinaryExpression*>(&node);
        if (cb->operator_ == "**" && parent.operator_ == "**") {
            return !is_right;
        }
//        if (is_right) {
//            return BinaryStrPrecedence(cb->operator_) <= BinaryStrPrecedence(parent->operator_);
//        }
        return BinaryStrPrecedence(cb->operator_) < BinaryStrPrecedence(parent.operator_);
    }

    void CodeGen::Traverse(Script& node) {
        SortComments(node.comments);

        for (auto& stmt : node.body) {
            WriteIndent();
            TraverseNode(*stmt);
            WriteLineEnd();
        }

        ordered_comments_.clear();
        ordered_comments_.shrink_to_fit();
    }

    void CodeGen::Traverse(Module& node) {
        auto module_scope = node.scope->CastToModule();
        SortComments(node.comments);

        for (auto& stmt : node.body) {
            WriteIndent();
            TraverseNode(*stmt);
            WriteLineEnd();
        }

        ordered_comments_.clear();
        ordered_comments_.shrink_to_fit();
    }

    void CodeGen::Traverse(ArrayExpression& node) {
        Write("[");
        std::size_t count = 0;
        for (auto& elem : node.elements) {
            if (elem.has_value()) {
                TraverseNode(**elem);
            }
            if (count++ < node.elements.size() - 1) {
                Write(S_COMMA);
            } else if (!elem.has_value()) {
                Write(S_COMMA);
            }
        }
        Write("]");
    }

    void CodeGen::Traverse(BlockStatement& node) {
        Write("{");
        state_.indent_level++;

        if (!node.body.empty()) {
            if (!IS_MINIFY) {
                WriteLineEnd();
            }
            for (auto& elem : node.body) {
                WriteCommentBefore(*elem);

                WriteIndent();

                TraverseNode(*elem);
                WriteLineEnd();
            }
        }

        state_.indent_level--;
        WriteIndentWith("}");
    }

    void CodeGen::Traverse(EmptyStatement& node) {
        Write(';');
    }

    void CodeGen::Traverse(ExpressionStatement& node) {
        int precedence = ExpressionPrecedence(node);
        if (
                (precedence == needs_parentheses) ||
                (precedence == 3 && node.expression->type == SyntaxNodeType::ObjectPattern)) {
            Write('(');
            TraverseNode(*node.expression);
            Write(')');
        } else {
            TraverseNode(*node.expression);
        }
        Write(u';');
    }

    void CodeGen::Traverse(IfStatement& node) {
        Write(IS_MINIFY ? "if(" : "if (");
        TraverseNode(*node.test);
        Write(IS_MINIFY ? ")" : ") ");
        TraverseNode(*node.consequent);
        if (node.alternate.has_value()) {
//            Write(IS_MINIFY ? "else" : " else ");
            Write(" else ");
            TraverseNode(**node.alternate);
        }
    }

    void CodeGen::Traverse(LabeledStatement& node) {
        TraverseNode(*node.label);
        Write(": ");
        TraverseNode(*node.body);
    }

    void CodeGen::Traverse(BreakStatement& node) {
        Write("break");
        if (node.label.has_value()) {
            Write(" ");
            TraverseNode(**node.label);
        }
        Write(";");
    }

    void CodeGen::Traverse(ContinueStatement& node) {
        Write("continue");
        if (node.label.has_value()) {
            Write(" ");
            TraverseNode(**node.label);
        }
        Write(";");
    }

    void CodeGen::Traverse(WithStatement& node) {
        Write(IS_MINIFY ? "with(" : "with (");
        TraverseNode(*node.object);
        Write(IS_MINIFY ? ")" : ") ");
        TraverseNode(*node.body);
    }

    void CodeGen::Traverse(SwitchStatement& node) {
        Write(IS_MINIFY ? "switch(" : "switch (");
        TraverseNode(*node.discrimiant);
        Write(IS_MINIFY ? "){" : ") {");
        WriteLineEnd();
        state_.indent_level++;

        for (auto& case_ : node.cases) {

            WriteIndent();
            if (case_->test.has_value()) {
                Write("case ");
                TraverseNode(**case_->test);
                Write(":");
                WriteLineEnd();
            } else {
                Write("default:");
                WriteLineEnd();
            }

            for (auto& cons : case_->consequent) {
                WriteIndent();
                TraverseNode(*cons);
                WriteLineEnd();
            }

        }

        state_.indent_level--;
        WriteIndentWith("}");
    }

    void CodeGen::Traverse(ReturnStatement& node) {
        Write("return");
        if (node.argument.has_value()) {
            Write(" ");
            TraverseNode(**node.argument);
        }
        Write(";");
    }

    void CodeGen::Traverse(ThrowStatement& node) {
        Write("throw ");
        TraverseNode(*node.argument);
        Write(";");
    }

    void CodeGen::Traverse(TryStatement& node) {
        Write(IS_MINIFY ? "try" : "try ");
        TraverseNode(*node.block);

        if (node.handler.has_value()) {
            auto handler = *node.handler;
            Write(IS_MINIFY ? "catch(" : " catch (");
            TraverseNode(*handler->param);
            Write(")");
            TraverseNode(*handler->body);
        }

        if (node.finalizer.has_value()) {
            Write(IS_MINIFY ? "finally" : " finally ");
            TraverseNode(**node.finalizer);
        }
    }

    void CodeGen::Traverse(WhileStatement& node) {
        Write(IS_MINIFY ? "while(" : "while (");
        TraverseNode(*node.test);
        Write(IS_MINIFY ? ")" : ") ");
        TraverseNode(*node.body);
    }

    void CodeGen::Traverse(DoWhileStatement& node) {
        Write(IS_MINIFY ? "do" : "do ");
        TraverseNode(*node.body);
        Write(IS_MINIFY ? "while(" : " while (");
        TraverseNode(*node.test);
        Write(");");
    }

    void CodeGen::Traverse(ForStatement& node) {
        Write(IS_MINIFY ? "for(" : "for (");
        if (node.init.has_value()) {
            auto init = *node.init;
            if (init->type == SyntaxNodeType::VariableDeclaration) {
                auto decl = dynamic_cast<VariableDeclaration*>(init);
                FormatVariableDeclaration(*decl);
            } else {
                TraverseNode(*init);
            }
        }
        Write(IS_MINIFY ? ";" : "; ");
        if (node.test.has_value()) {
            TraverseNode(**node.test);
        }
        Write(IS_MINIFY ? ";" : "; ");
        if (node.update.has_value()) {
            TraverseNode(**node.update);
        }
        Write(IS_MINIFY ? ")" : ") ");
        TraverseNode(*node.body);
    }

    void CodeGen::Traverse(ForInStatement& node) {
        Write(IS_MINIFY ? "for(" : "for (");
        if (node.left->type == SyntaxNodeType::VariableDeclaration) {
            auto decl = dynamic_cast<VariableDeclaration*>(node.left);
            FormatVariableDeclaration(*decl);
        } else {
            TraverseNode(*node.left);
        }
        Write(" in ");
        TraverseNode(*node.right);
        Write(IS_MINIFY ? ")" : ") ");
        TraverseNode(*node.body);
    }

    void CodeGen::Traverse(ForOfStatement & node) {
        Write(IS_MINIFY ? "for(" : "for (");
        if (node.left->type == SyntaxNodeType::VariableDeclaration) {
            auto decl = dynamic_cast<VariableDeclaration*>(node.left);
            FormatVariableDeclaration(*decl);
        } else {
            TraverseNode(*node.left);
        }
        Write(" of ");
        TraverseNode(*node.right);
        Write(IS_MINIFY ? ")" : ") ");
        TraverseNode(*node.body);
    }

    void CodeGen::Traverse(DebuggerStatement& node) {
        Write("debugger;");
        WriteLineEnd();
    }

    void CodeGen::Traverse(FunctionDeclaration& node) {
        if (node.async) {
            Write("async ");
        }

        if (node.generator) {
            Write("function* ");
        } else {
            Write("function ");
        }

        if (node.id.has_value()) {
            Write((*node.id)->name);
        }

        FormatSequence(node.params);
        if (!IS_MINIFY) {
            Write(" ");
        }
        TraverseNode(*node.body);
    }

    void CodeGen::Traverse(FunctionExpression& node) {
        if (node.async) {
            Write("async ");
        }

        if (node.generator) {
            Write("function*");
        } else {
            Write("function");
        }

        if (node.id.has_value()) {
            Write(" ");
            Write((*node.id)->name);
        }

        FormatSequence(node.params);
        if (!IS_MINIFY) {
            Write(" ");
        }
        TraverseNode(*node.body);
    }

    void CodeGen::Traverse(VariableDeclaration& node) {
        FormatVariableDeclaration(node);
        Write(";");
    }

    void CodeGen::Traverse(VariableDeclarator& node) {
        TraverseNode(*node.id);
        if (node.init.has_value()) {
            Write(IS_MINIFY ? "=" : " = ");
            TraverseNode(**node.init);
        }
    }

    void CodeGen::Traverse(ClassDeclaration& node) {
        Write("class ");
        if (node.id.has_value()) {
            Write((*node.id)->name);
            Write(" ");
        }
        if (node.super_class.has_value()) {
            Write("extends ");
            TraverseNode(**node.super_class);
            if (!IS_MINIFY) {
                Write(" ");
            }
        }
        TraverseNode(*node.body);
    }

    void CodeGen::Traverse(ClassBody& node) {
        Write("{");
        state_.indent_level++;

        if (!node.body.empty()) {
            WriteLineEnd();
            for (auto& elem : node.body) {
                WriteCommentBefore(*elem);
                WriteIndent();
                TraverseNode(*elem);
                WriteLineEnd();
            }
        }

        state_.indent_level--;
        WriteIndentWith("}");
    }

    void CodeGen::Traverse(ImportDeclaration& node) {
        Write("import ");

        std::uint32_t i = 0;
        if (node.specifiers.size() > 0) {
            for (auto& spec : node.specifiers) {
                if (i > 0) {
                    Write(IS_MINIFY ? "," : ", ");
                }
                if (spec->type == SyntaxNodeType::ImportDefaultSpecifier) {
                    auto default_ = dynamic_cast<ImportDefaultSpecifier*>(spec);
                    Write(default_->local->name, *default_);
                    i++;
                } else if (spec->type == SyntaxNodeType::ImportNamespaceSpecifier) {
                    auto namespace_ = dynamic_cast<ImportNamespaceSpecifier*>(spec);
                    std::string temp = "* as " + namespace_->local->name;
                    Write(temp, *namespace_);
                    i++;
                } else {
                    break;
                }
            }
            if (i < node.specifiers.size()) {
                Write(IS_MINIFY ? "{" : "{ ");
                while (true) {
                    auto spec = node.specifiers[i];
                    auto import_ = dynamic_cast<ImportSpecifier*>(spec);
                    Write(import_->imported->name, *spec);
                    if (import_->imported->name != import_->local->name) {
                        std::string temp = " as " + import_->local->name;
                        Write(temp);
                    }
                    if (++i < node.specifiers.size()) {
                        Write(S_COMMA);
                    } else {
                        break;
                    }
                }
                Write(IS_MINIFY ? "}" : " }");
            }
            Write(" from ");
        }

        this->Traverse(*node.source);
        Write(";");
    }

    void CodeGen::Traverse(ExportDefaultDeclaration& node) {
        Write("export default ");
        TraverseNode(*node.declaration);
        if (ExpressionPrecedence(*node.declaration) > 0 &&
            node.declaration->type == SyntaxNodeType::FunctionExpression) {
            Write(";");
        }
    }

    void CodeGen::Traverse(ExportNamedDeclaration& node) {
        Write("export ");
        if (node.declaration.has_value()) {
            TraverseNode(**node.declaration);
        } else {
            Write(IS_MINIFY ? "{" : "{ ");
            if (node.specifiers.size() > 0) {
                std::uint32_t i = 0;
                for (auto& spec : node.specifiers) {
                    Write(spec->local->name, *spec);
                    if (spec->local->name != spec->exported->name) {
                        std::string temp = " as " + spec->exported->name;
                        Write(temp);
                    }
                    if (++i < node.specifiers.size()) {
                        Write(S_COMMA);
                    } else {
                        break;
                    }
                }
            }
            Write(IS_MINIFY ? "}" : " }");
            if (node.source.has_value()) {
                Write(" from ");
                this->Traverse(**node.source);
            }
            Write(";");
        }
    }

    void CodeGen::Traverse(ExportAllDeclaration& node) {
        Write("export * from ");
        this->Traverse(*node.source);
        Write(";");
    }

    void CodeGen::Traverse(MethodDefinition& node) {
        if (node.static_) {
            Write("static ");
        }
        switch (node.kind) {
            case VarKind::Get:
                Write("get ");
                break;

            case VarKind::Set:
                Write("set ");
                break;

            default:
                break;

        }

        if (node.key.has_value() && node.value.has_value()) {
            auto fun_expr = dynamic_cast<FunctionExpression*>(*node.value);
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
                TraverseNode(**node.key);
                Write(']');
            } else {
                TraverseNode(**node.key);
            }
            FormatSequence(fun_expr->params);
            if (!IS_MINIFY) {
                Write(" ");
            }
            TraverseNode(*fun_expr->body);
        }
    }

    void CodeGen::Traverse(ArrowFunctionExpression& node) {
        if (node.async) {
            Write("async ", node);
        }
        auto& params = node.params;
        if (!params.empty()) {
            if (params.size() == 1 && params[0]->type == SyntaxNodeType::Identifier) {
                auto id = dynamic_cast<Identifier*>(params[0]);
                Write(id->name, *id);
            } else {
                FormatSequence(params);
            }
        } else {
            Write("()");
        }
        Write(IS_MINIFY ? "=>" : " => ");
        if (node.body->type == SyntaxNodeType::ObjectExpression) {
            Write("(");
            auto oe = dynamic_cast<ObjectExpression*>(node.body);
            this->Traverse(*oe);
            Write(")");
        } else {
            TraverseNode(*node.body);
        }
    }

    void CodeGen::Traverse(ThisExpression& node) {
        Write("this", node);
    }

    void CodeGen::Traverse(Super& node) {
        Write("super", node);
    }

    void CodeGen::Traverse(RestElement& node) {
        Write("...");
        TraverseNode(*node.argument);
    }

    void CodeGen::Traverse(SpreadElement& node) {
        Write("...");
        TraverseNode(*node.argument);
    }

    void CodeGen::Traverse(YieldExpression& node) {
        if (node.delegate) {
            Write("yield*");
        } else {
            Write("yield");
        }

        if (node.argument.has_value()) {
            Write(" ");
            TraverseNode(**node.argument);
        }
    }

    void CodeGen::Traverse(AwaitExpression& node) {
        Write("await ");
        TraverseNode(*node.argument);
    }

    void CodeGen::Traverse(TemplateLiteral& node) {
        Write("`");
        for (std::size_t i = 0; i < node.expressions.size(); i++) {
            auto expr = node.expressions[i];
            Write(node.quasis[i]->cooked);
            Write("${");
            TraverseNode(*expr);
            Write("}");
        }
        Write(node.quasis[node.quasis.size() - 1]->cooked);
        Write("`");
    }

    void CodeGen::Traverse(TaggedTemplateExpression& node) {
        TraverseNode(*node.tag);
        TraverseNode(*node.quasi);
    }

    void CodeGen::Traverse(ObjectExpression& node) {
        state_.indent_level++;
        Write("{");
        if (!node.properties.empty()) {
            WriteLineEnd();
            std::string comma = "," + line_end_;
            std::size_t i = 0;
            while (true) {
                auto prop = node.properties[i];
                WriteIndent();
                TraverseNode(*prop);
                if (++i < node.properties.size()) {
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

    void CodeGen::Traverse(Property& node) {
        switch (node.kind) {
            case VarKind::Get: {
                Write("get ");
                TraverseNode(*node.key);
                if (!node.value.has_value()) return;
                auto fun = dynamic_cast<FunctionExpression*>(*node.value);
                if (fun == nullptr) return;

                FormatSequence(fun->params);
                if (!IS_MINIFY) {
                    Write(" ");
                }
                TraverseNode(*fun->body);
                break;
            }

            case VarKind::Set: {
                Write("set ");
                TraverseNode(*node.key);
                if (!node.value.has_value()) return;
                auto fun = dynamic_cast<FunctionExpression*>(*node.value);
                if (fun == nullptr) return;

                FormatSequence(fun->params);
                if (!IS_MINIFY) {
                    Write(" ");
                }
                TraverseNode(*fun->body);
                break;
            }

            default: {
                bool shorthand = node.shorthand;
                if (node.value.has_value()
                    && node.key->type == SyntaxNodeType::Identifier
                    && (*node.value)->type == SyntaxNodeType::Identifier) {
                    auto key_id = dynamic_cast<Identifier*>(node.key);
                    auto val_id = dynamic_cast<Identifier*>(*node.value);
                    shorthand = key_id->name == val_id->name;
                }
                if (!shorthand) {
                    if (node.computed) {
                        Write("[");
                        TraverseNode(*node.key);
                        Write("]");
                    } else {
                        TraverseNode(*node.key);
                    }
                }
                if (node.value.has_value()) {
                    if (!shorthand) {
                        Write(IS_MINIFY ? ":" : ": ");
                    }
                    TraverseNode(**node.value);
                }
                break;
            }
        }
    }

    void CodeGen::Traverse(SequenceExpression& node) {
        std::vector<SyntaxNode*> nodes;
        for (auto& i : node.expressions) {
            nodes.push_back(i);
        }
        FormatSequence(nodes);
    }

    void CodeGen::Traverse(UnaryExpression& node) {
        if (node.prefix) {
            Write(node.operator_);
            if (node.operator_.size() > 1) {
                Write(" ");
            }
            UnaryExpression unaryExpression;
            if (ExpressionPrecedence(*node.argument) <
                ExpressionPrecedence(unaryExpression)
                    ) {
                Write("(");
                TraverseNode(*node.argument);
                Write(")");
            } else {
                TraverseNode(*node.argument);
            }
        } else {
            TraverseNode(*node.argument);
            Write(node.operator_);
        }
    }

    void CodeGen::Traverse(AssignmentExpression& node) {
        TraverseNode(*node.left);
        if (IS_MINIFY) {
            Write(node.operator_);
        } else {
            Write(std::string(" ") + node.operator_ + " ");
        }
        TraverseNode(*node.right);
    }

    void CodeGen::Traverse(AssignmentPattern& node) {
        TraverseNode(*node.left);
        Write(IS_MINIFY ? "=" : " = ");
        TraverseNode(*node.right);
    }

    void CodeGen::Traverse(BinaryExpression& node) {
        bool is_in = node.operator_ == "in";
        if (is_in) {
            Write("(");
        }
        FormatBinaryExpression(*node.left, node, false);
        if (IS_MINIFY && node.operator_ != "in" && node.operator_ != "instanceof") {
            Write(node.operator_);
        } else {
            Write(" " + node.operator_ + " ");
        }
        FormatBinaryExpression(*node.right, node, true);
        if (is_in) {
            Write(")");
        }
    }

    void CodeGen::Traverse(ConditionalExpression& node) {
        ConditionalExpression conditionalExpression;
        if (ExpressionPrecedence(*node.test) >
            ExpressionPrecedence(conditionalExpression)) {
            TraverseNode(*node.test);
        } else {
            Write("(");
            TraverseNode(*node.test);
            Write(")");
        }
        Write(IS_MINIFY ? "?" : " ? ");
        TraverseNode(*node.consequent);
        Write(IS_MINIFY ? ":" : " : ");
        TraverseNode(*node.alternate);
    }

    void CodeGen::Traverse(NewExpression& node) {
        Write("new ");
        CallExpression callExpression;
        if (ExpressionPrecedence(*node.callee) <
            ExpressionPrecedence(callExpression) ||
            HasCallExpression(node.callee)) {
            Write("(");
            TraverseNode(*node.callee);
            Write(")");
        } else {
            TraverseNode(*node.callee);
        }
        FormatSequence(node.arguments);
    }

    void CodeGen::Traverse(MemberExpression& node) {
        MemberExpression memberExpression;
        if (ExpressionPrecedence(*node.object) < ExpressionPrecedence(memberExpression)) {
            Write('(');
            TraverseNode(*node.object);
            Write(')');
        } else {
            TraverseNode(*node.object);
        }
        if (node.computed) {
            Write('[');
            TraverseNode(*node.property);
            Write(']');
        } else {
            Write('.');
            TraverseNode(*node.property);
        }
    }

    void CodeGen::Traverse(CallExpression& node) {
        CallExpression callExpression;
        if (ExpressionPrecedence(*node.callee) <
            ExpressionPrecedence(callExpression)) {
            Write("(");
            TraverseNode(*node.callee);
            Write(")");
        } else {
            TraverseNode(*node.callee);
        }
        FormatSequence(node.arguments);
    }

    void CodeGen::Traverse(Identifier& node) {
        if (IS_MINIFY && node.name == "undefined") {
            Write("void 0");
        } else {
            Write(node.name, node);
        }
    }

    void CodeGen::Traverse(Literal& lit) {
        if (IS_MINIFY && lit.ty == Literal::Ty::Boolean) {
            if (lit.raw == "true") {
                Write("!0");
            } else {
                Write("!1");
            }
        } else {
            Write(lit.raw, lit);
        }
    }

    void CodeGen::Traverse(RegexLiteral &lit) {
        Write(lit.value, lit);
    }

    void CodeGen::Traverse(UpdateExpression& update) {
        if (update.prefix) {
            Write(update.operator_);
            TraverseNode(*update.argument);
        } else {
            TraverseNode(*update.argument);
            Write(update.operator_);
        }
    }

    void CodeGen::Traverse(ObjectPattern& node) {
        Write(IS_MINIFY ? "{" : "{ ");
        for (std::size_t i = 0; ;) {
            TraverseNode(*node.properties[i]);
            if (++i < node.properties.size()) {
                Write(", ");
            } else {
                break;
            }
        }
        Write(IS_MINIFY ? "}" : " }");
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

    void CodeGen::WriteTopCommentBefore_(SyntaxNode& node) {
        while (!ordered_comments_.empty()) {
            auto& top = ordered_comments_.front();
            if (top->range_.second < node.range.first) {
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

