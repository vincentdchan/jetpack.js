//
// Created by Duzhong Chen on 2019/9/3.
//
#pragma once

#include <memory>
#include <vector>
#include "parser/ParseErrorHandler.h"
#include "utils/Common.h"
#include "utils/MemoryViewOwner.h"
#include "Token.h"
#include "Comment.h"

namespace jetpack {

    struct Cursor {
    public:
        uint32_t u8 = 0;
        uint32_t u16 = 0;

        bool operator==(const Cursor& that) const {
            return u8 == that.u8;
        }

    };

    class ScannerImpl;

    struct ScannerImplDeleter {

        void operator()(ScannerImpl* d);

    };

    class Scanner {
    public:
        Scanner(MemoryViewOwner& source, parser::ParseErrorHandler& error_handler);
        Scanner(const Scanner&) = delete;
        Scanner(Scanner&&) = delete;

        Scanner& operator=(const Scanner&) = delete;
        Scanner& operator=(Scanner&&) = delete;

        struct ScannerState {
        public:
            Cursor cursor_;
            uint32_t line_number_ = 0;
            uint32_t line_start_ = 0;
        };

        [[nodiscard]]
        int32_t Length() const;

        ScannerState SaveState();
        void RestoreState(const ScannerState& state);

        [[nodiscard]]
        bool IsEnd() const;

        [[nodiscard]]
        uint32_t LineNumber() const;

        void SetLineNumber(uint32_t ln);

        Cursor Index() const;

        void SetIndex(Cursor index);

        [[nodiscard]]
        uint32_t Column() const;

        [[nodiscard]]
        uint32_t LineStart() const;

        void SetLineStart(uint32_t ls);

        std::string_view View(uint32_t start, uint32_t end);

        void ScanComments(std::vector<Sp<Comment>>& result);
        static bool IsFutureReservedWord(JsTokenType t);
        static JsTokenType IsStrictModeReservedWord(std::string_view str);
        static bool IsRestrictedWord(std::string_view str_);
        static JsTokenType ToKeyword(const std::string& str_);

        Token Lex();

        [[nodiscard]]
        char CharAt(uint32_t index) const;

        [[nodiscard]]
        MemoryViewOwner& Source() const;

        [[nodiscard]]
        char Peek() const;

        [[nodiscard]]
        char Peek(uint32_t offset) const;

        char NextChar();

        [[nodiscard]]
        Token ScanRegExp();
    private:
        std::unique_ptr<ScannerImpl, ScannerImplDeleter> d_;

    };

}
