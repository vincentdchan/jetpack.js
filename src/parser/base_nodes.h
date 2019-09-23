//
// Created by Duzhong Chen on 2019/9/8.
//
#pragma once
#include "node_types.h"
#include "../macros.h"
#include "../tokenizer/token.h"

template <typename T>
using Sp = std::shared_ptr<T>;

typedef double JS_Number;
typedef UString JS_RegExp;

enum class VarKind {
    Invalid = 0,
    Var,
    Let,
    Const,
};

class SyntaxNode {
public:
    SyntaxNodeType type = SyntaxNodeType::Invalid;
    SyntaxNode() = default;

    std::pair<std::uint32_t, std::uint32_t> range;
    SourceLocation location;

    virtual bool IsPattern() const { return false; }
    virtual bool IsDeclaration() const { return false; }
    virtual bool IsExpression() const { return false; }
    virtual bool IsStatement() const { return false; }

};

class Expression: virtual public SyntaxNode {
public:
    Expression() = default;

    [[nodiscard]] bool IsExpression() const override { return true; }

};

class Statement: virtual public SyntaxNode {
public:
    Statement() = default;

    [[nodiscard]] bool IsStatement() const override { return true; }

};

class Declaration: virtual public Statement {
public:
    Declaration() = default;

    [[nodiscard]] bool IsDeclaration() const override { return true; }

};

class Pattern: virtual public SyntaxNode {
public:
    Pattern() = default;

    [[nodiscard]] bool IsPattern() const override { return true; }

};
