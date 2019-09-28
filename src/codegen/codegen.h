//
// Created by Duzhong Chen on 2019/9/27.
//

#pragma once

#include <memory>
#include <string>
#include <sstream>
#include <cinttypes>
#include "../utils.h"
#include "node_traverser.h"

class CodeGen: public NodeTraverser {
private:
    struct State {
        std::int32_t line = 1;
        std::int32_t column = 0;
        std::uint32_t indent_level = 0;
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
    CodeGen(const Config& config);

    inline void Write(char ch) {
        output << ch;
    }

    inline void Write(const char* c_str) {
        output << c_str;
    }

    inline void Write(const std::string& str, Sp<SyntaxNode> node = nullptr) {
        output << str;
    }

    inline void Write(const UString& w_str, Sp<SyntaxNode> node = nullptr) {
        Write(utils::To_UTF8(w_str), node);
    }

    inline void WriteLineEnd() {
        output << config_.line_end;
        state_.line++;
    }

    inline void WriteIndent() {
        for (std::uint32_t i = 0; i < state_.indent_level; i++) {
            Write(config_.indent);
        }
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
    void Literal(const Sp<Literal>& lit);

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

private:
    Config config_;

    State state_;

    std::stringstream output;

};
