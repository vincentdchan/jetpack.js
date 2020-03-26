//
// Created by Duzhong Chen on 2019/9/8.
//
#pragma once
#include "NodeTypes.h"
#include "../macros.h"
#include "../tokenizer/Token.h"
#include "../tokenizer/Comment.h"

template <typename T>
using Sp = std::shared_ptr<T>;

typedef double JS_Number;
typedef UString JS_RegExp;

namespace rocket_bundle {

    enum class TSAccessibility {
        Private,
        Public,
        Protected,
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
        virtual bool IsTSType() const { return false; }

        virtual ~SyntaxNode() = default;

    };

    class TSType: virtual public SyntaxNode {
    public:
        TSType() = default;

        [[nodiscard]] bool IsTSType() const override { return true; }

    };

    class Expression: virtual public SyntaxNode {
    public:
        Expression() = default;

        [[nodiscard]] bool IsExpression() const override { return true; }
        virtual bool IsGenerator() const { return false; }
        virtual bool IsAsync() const { return false; }
        virtual bool IsComputed() const { return false; }

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

}
