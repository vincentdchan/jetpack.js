//
// Created by Duzhong Chen on 2019/9/8.
//
#pragma once

#include <memory>
#include <cstddef>
#include <vector>
#include "NodeTypes.h"
#include "Slice.h"
#include "utils/Common.h"
#include "macros.h"
#include "tokenizer/Token.h"
#include "tokenizer/Comment.h"

typedef double JS_Number;

namespace jetpack {

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
        SyntaxNode*    next = nullptr;

        virtual bool IsPattern() const { return false; }
        virtual bool IsDeclaration() const { return false; }
        virtual bool IsExpression() const { return false; }
        virtual bool IsStatement() const { return false; }
        virtual bool IsTSType() const { return false; }

        virtual ~SyntaxNode() = default;

        template <typename T>
        inline T* As() {
            return reinterpret_cast<T*>(this);
        }

    };

    template<typename T, typename = typename std::enable_if<std::is_base_of<SyntaxNode, T>::value>::type>
    class NodeList {
    public:
        constexpr NodeList() = default;
        constexpr NodeList(const NodeList& that):
            begin_(that.begin_), end_(that.end_), len_(that.len_) {}

        constexpr NodeList& operator=(const NodeList& that) {
            begin_ = that.begin_;
            end_ = that.end_;
            len_ = that.len_;
            return *this;
        }

        struct Iter {
        public:
            constexpr Iter() {}
            constexpr Iter(T* node): node_(node) {}

            constexpr bool operator!=(const Iter& that) {
                return node_ != that.node_;
            }

            T* operator*() {
                return node_;
            }

            Iter& operator++() {
                node_ = node_->next;
                return *this;
            }

        private:
            T* node_ = nullptr;

        };

        constexpr Iter begin() const { return Iter(begin_); }
        constexpr Iter end() const { return Iter(); }
        constexpr size_t size() { return len_; }
        constexpr bool empty() { return len_ == 0; }

        inline void push_back(T* child) {
            child->next = nullptr;
            if (len_ == 0) {
                begin_ = end_ = child;
                len_ = 1;
            } else {
                end_->next = child;
                end_ = child;
                len_++;
            }
        }

        constexpr void clear() {
            begin_ = end_ = nullptr;
            len_ = 0;
        }

        inline std::vector<T*> to_vec() const {
            std::vector<T*> result;
            for (auto child : *this) {
                result.push_back(child);
            }
            return result;
        }

    private:
        T* begin_ = nullptr;
        T* end_ = nullptr;
        size_t len_ = 0;

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
