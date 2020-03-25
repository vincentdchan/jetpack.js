//
// Created by Duzhong Chen on 2019/9/27.
//

#pragma once

#include <memory>
#include <string>
#include <sstream>
#include <cinttypes>
#include <deque>
#include "Utils.h"
#include "NodeTraverser.h"

namespace rocket_bundle {

    /**
     * Reference: https://github.com/davidbonnet/astring/blob/master/src/astring.js
     */
    class CodeGen: public NodeTraverser {
    private:
        struct State {
            std::int32_t line = 1;
            std::int32_t column = 0;
            std::uint32_t indent_level = 0;
        };

        class HasCallExpressionTraverser: public AutoNodeTraverser {
        public:
            bool has_call = false;

            bool TraverseBefore(const Sp<CallExpression>& node) override  {
                has_call = true;
                return false;
            }

        };

    public:
        struct Config {
            std::uint32_t start_indent_level = 0;
            std::string indent = "  ";
            std::string line_end = "\n";
            bool source_map = false;
            bool comments = true;
        };

        CodeGen();
        CodeGen(const Config& config, std::ostream& output_stream);

    private:
        inline void Write(char ch) {
            output << ch;
#ifdef DEBUG
            output.flush();
#endif
        }

        inline void Write(const char* c_str) {
            output << c_str;
#ifdef DEBUG
            output.flush();
#endif
        }

        inline void Write(const std::string& str, Sp<SyntaxNode> node = nullptr) {
            output << str;
#ifdef DEBUG
            output.flush();
#endif
        }

        inline void Write(const UString& w_str, Sp<SyntaxNode> node = nullptr) {
            Write(utils::To_UTF8(w_str), node);
#ifdef DEBUG
            output.flush();
#endif
        }

        inline void WriteLineEnd() {
            output << config_.line_end;
            state_.line++;
#ifdef DEBUG
            output.flush();
#endif
        }

        inline void WriteIndent() {
            for (std::uint32_t i = 0; i < state_.indent_level; i++) {
                Write(config_.indent);
            }
#ifdef DEBUG
            output.flush();
#endif
        }

        inline void WriteIndentWith(const char* c_str) {
            WriteIndent();
            Write(c_str);
        }

        inline void WriteIndentWith(const std::string& str) {
            WriteIndent();
            Write(str);
        }

        static const int needs_parentheses = 17;

        static int ExpressionPrecedence(SyntaxNodeType t);

        void FormatVariableDeclaration(const Sp<VariableDeclaration>& node);
        void FormatSequence(std::vector<Sp<SyntaxNode>>& params);
        void FormatBinaryExpression(const Sp<Expression>& expr, const Sp<BinaryExpression>& parent, bool is_right);
        bool HasCallExpression(const Sp<SyntaxNode>&);
        bool ExpressionNeedsParenthesis(const Sp<Expression>& node, const Sp<BinaryExpression>& parent, bool is_right);

    public:
        void Traverse(const Sp<Script>& node) override;
        void Traverse(const Sp<Module>& node) override;
        void Traverse(const Sp<Literal>& lit) override;
        void Traverse(const Sp<RegexLiteral>& lit) override;
        void Traverse(const Sp<ArrayExpression>& node) override;
        void Traverse(const Sp<BlockStatement>& node) override;
        void Traverse(const Sp<EmptyStatement>& node) override;
        void Traverse(const Sp<ExpressionStatement>& node) override;
        void Traverse(const Sp<IfStatement>& node) override;
        void Traverse(const Sp<LabeledStatement>& node) override;
        void Traverse(const Sp<BreakStatement>& node) override;
        void Traverse(const Sp<ContinueStatement>& node) override;
        void Traverse(const Sp<WithStatement>& node) override;
        void Traverse(const Sp<SwitchStatement>& node) override;
        void Traverse(const Sp<ReturnStatement>& node) override;
        void Traverse(const Sp<ThrowStatement>& node) override;
        void Traverse(const Sp<TryStatement>& node) override;
        void Traverse(const Sp<WhileStatement>& node) override;
        void Traverse(const Sp<DoWhileStatement>& node) override;
        void Traverse(const Sp<ForStatement>& node) override;
        void Traverse(const Sp<ForInStatement>& node) override;
        void Traverse(const Sp<ForOfStatement>& node) override;
        void Traverse(const Sp<DebuggerStatement>& node) override;
        void Traverse(const Sp<FunctionDeclaration>& node) override;
        void Traverse(const Sp<FunctionExpression>& node) override;
        void Traverse(const Sp<VariableDeclaration>& node) override;
        void Traverse(const Sp<VariableDeclarator>& node) override;
        void Traverse(const Sp<ClassDeclaration>& node) override;
        void Traverse(const Sp<ClassBody>& node) override;
        void Traverse(const Sp<ImportDeclaration>& node) override;
        void Traverse(const Sp<ExportDefaultDeclaration>& node) override;
        void Traverse(const Sp<ExportNamedDeclaration>& node) override;
        void Traverse(const Sp<ExportAllDeclaration>& node) override;
        void Traverse(const Sp<MethodDefinition>& node) override;
        void Traverse(const Sp<ArrowFunctionExpression>& node) override;
        void Traverse(const Sp<ObjectExpression>& node) override;
        void Traverse(const Sp<ThisExpression>& node) override;
        void Traverse(const Sp<Super>& node) override;
        void Traverse(const Sp<RestElement>& node) override;
        void Traverse(const Sp<SpreadElement>& node) override;
        void Traverse(const Sp<YieldExpression>& node) override;
        void Traverse(const Sp<AwaitExpression>& node) override;
        void Traverse(const Sp<TemplateLiteral>& node) override;
        void Traverse(const Sp<TaggedTemplateExpression>& node) override;
        void Traverse(const Sp<Property>& node) override;
        void Traverse(const Sp<SequenceExpression>& node) override;
        void Traverse(const Sp<UnaryExpression>& node) override;
        void Traverse(const Sp<AssignmentExpression>& node) override;
        void Traverse(const Sp<AssignmentPattern>& node) override;
        void Traverse(const Sp<BinaryExpression>& node) override;
        void Traverse(const Sp<ConditionalExpression>& node) override;
        void Traverse(const Sp<NewExpression>& node) override;
        void Traverse(const Sp<CallExpression>& node) override;
        void Traverse(const Sp<MemberExpression>& node) override;
        void Traverse(const Sp<Identifier>& node) override;
        void Traverse(const Sp<UpdateExpression>& node) override;
        void Traverse(const Sp<ObjectPattern>& node) override;

        inline std::ostream& Stream() {
            return output;
        }

    private:

        inline void WriteCommentBefore(const Sp<SyntaxNode>& node) {
            if (!config_.comments) return;

            WriteTopCommentBefore_(node);
        }

        void WriteTopCommentBefore_(const Sp<SyntaxNode>& node);

        std::deque<Sp<Comment>> ordered_comments_;
        void SortComments(std::vector<Sp<Comment>> comments);

        Config config_;

        State state_;

        std::ostream& output;

    };

}
