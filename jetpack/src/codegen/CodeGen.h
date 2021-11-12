//
// Created by Duzhong Chen on 2019/9/27.
//

#pragma once

#include <memory>
#include <string>
#include <sstream>
#include <cinttypes>
#include <deque>
#include "NodeTraverser.h"
#include "AutoNodeTraverser.h"
#include "utils/string/UString.h"
#include "utils/Common.h"
#include "codegen/CodeGenConfig.h"
#include "sourcemap/MappingCollector.h"

namespace jetpack {

    struct CodeGenResult {
    public:
        int32_t lines_count;
        int32_t last_line_column;
        std::string content;

    };

    /**
     * Reference: https://github.com/davidbonnet/astring/blob/master/src/astring.js
     */
    class CodeGen: public NodeTraverser {
    private:
        struct State {
            int32_t  line = 1;
            int32_t  column = 0;
            uint32_t indent_level = 0;
        };

        class HasCallExpressionTraverser: public AutoNodeTraverser {
        public:
            bool has_call = false;

            bool TraverseBefore(CallExpression* node) override  {
                has_call = true;
                return false;
            }

        };

    public:
        explicit CodeGen(
                 const CodeGenConfig& config,
                 Sp<MappingCollector> sourceMapGenerator = nullptr);

        [[nodiscard]]
        inline CodeGenResult GetResult() const {
            return {
                    state_.line,
                    state_.column,
                    output,
            };
        }

        void AddSnippet(const std::string& content);

    private:
        inline void Write(char ch) {
            output += ch;
            state_.column += 1;
        }

        void Write(const std::string& str);

        void Write(const std::string& str, SyntaxNode& node);

        void WriteLineEnd();

        void WriteIndent();

        inline void WriteIndentWith(const std::string& str) {
            WriteIndent();
            Write(str);
        }

        static const int needs_parentheses = 17;

        static int ExpressionPrecedence(SyntaxNode& node);

        void FormatVariableDeclaration(VariableDeclaration& node);
        void FormatSequence(NodeList<SyntaxNode>& params);
        void FormatBinaryExpression(Expression& expr, BinaryExpression& parent, bool is_right);
        bool HasCallExpression(SyntaxNode* node);
        bool ExpressionNeedsParenthesis(Expression& node, BinaryExpression& parent, bool is_right);

    public:
        void Traverse(Script& node) override;
        void Traverse(Module& node) override;
        void Traverse(Literal& lit) override;
        void Traverse(RegexLiteral& lit) override;
        void Traverse(ArrayExpression& node) override;
        void Traverse(BlockStatement& node) override;
        void Traverse(EmptyStatement& node) override;
        void Traverse(ExpressionStatement& node) override;
        void Traverse(IfStatement& node) override;
        void Traverse(LabeledStatement& node) override;
        void Traverse(BreakStatement& node) override;
        void Traverse(ContinueStatement& node) override;
        void Traverse(WithStatement& node) override;
        void Traverse(SwitchStatement& node) override;
        void Traverse(ReturnStatement& node) override;
        void Traverse(ThrowStatement& node) override;
        void Traverse(TryStatement& node) override;
        void Traverse(WhileStatement& node) override;
        void Traverse(DoWhileStatement& node) override;
        void Traverse(ForStatement& node) override;
        void Traverse(ForInStatement& node) override;
        void Traverse(ForOfStatement& node) override;
        void Traverse(DebuggerStatement& node) override;
        void Traverse(FunctionDeclaration& node) override;
        void Traverse(FunctionExpression& node) override;
        void Traverse(VariableDeclaration& node) override;
        void Traverse(VariableDeclarator& node) override;
        void Traverse(ClassDeclaration& node) override;
        void Traverse(ClassBody& node) override;
        void Traverse(ImportDeclaration& node) override;
        void Traverse(ExportDefaultDeclaration& node) override;
        void Traverse(ExportNamedDeclaration& node) override;
        void Traverse(ExportAllDeclaration& node) override;
        void Traverse(MethodDefinition& node) override;
        void Traverse(ArrowFunctionExpression& node) override;
        void Traverse(ObjectExpression& node) override;
        void Traverse(ThisExpression& node) override;
        void Traverse(Super& node) override;
        void Traverse(RestElement& node) override;
        void Traverse(SpreadElement& node) override;
        void Traverse(YieldExpression& node) override;
        void Traverse(AwaitExpression& node) override;
        void Traverse(TemplateLiteral& node) override;
        void Traverse(TaggedTemplateExpression& node) override;
        void Traverse(Property& node) override;
        void Traverse(SequenceExpression& node) override;
        void Traverse(UnaryExpression& node) override;
        void Traverse(AssignmentExpression& node) override;
        void Traverse(AssignmentPattern& node) override;
        void Traverse(BinaryExpression& node) override;
        void Traverse(ConditionalExpression& node) override;
        void Traverse(NewExpression& node) override;
        void Traverse(CallExpression& node) override;
        void Traverse(MemberExpression& node) override;
        void Traverse(Identifier& node) override;
        void Traverse(UpdateExpression& node) override;
        void Traverse(ObjectPattern& node) override;

        [[nodiscard]]
        inline Sp<MappingCollector> SourcemapCollector() {
            return mapping_collector_;
        }

    private:

        inline void WriteCommentBefore(SyntaxNode& node) {
            if (!config_.comments) return;

            WriteTopCommentBefore_(node);
        }

        void WriteTopCommentBefore_(SyntaxNode& node);

        std::deque<Sp<Comment>> ordered_comments_;
        void SortComments(std::vector<Sp<Comment>> comments);

        CodeGenConfig config_;

        State state_;

        // nullable
        Sp<MappingCollector> mapping_collector_;

        std::string output;

    };

}
