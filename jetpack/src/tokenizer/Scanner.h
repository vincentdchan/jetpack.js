//
// Created by Duzhong Chen on 2019/9/3.
//
#pragma once

#include <memory>
#include <vector>
#include <stack>
#include "parser/ParseErrorHandler.h"
#include "utils/Common.h"
#include "Token.h"
#include "StringWithMapping.h"
#include "Comment.h"

namespace jetpack {

    class Scanner {
    public:
        Scanner(const Sp<StringWithMapping>& source, Sp<parser::ParseErrorHandler> error_handler);
        Scanner(const Scanner&) = delete;
        Scanner(Scanner&&) = delete;

        Scanner& operator=(const Scanner&) = delete;
        Scanner& operator=(Scanner&&) = delete;

        struct Cursor {
        public:
            uint32_t u8 = 0;
            uint32_t u16 = 0;

            bool operator==(const Cursor& that) const {
                return u8 == that.u8;
            }

        };

        struct ScannerState {
        public:
            Cursor cursor_;
            uint32_t line_number_ = 0;
            uint32_t line_start_ = 0;
        };

        [[nodiscard]]
        inline int32_t Length() const {
            return source_->size();
        }

        ScannerState SaveState();
        void RestoreState(const ScannerState& state);

        [[nodiscard]]
        inline bool IsEnd() const {
            return cursor_.u8 >= Length();
        }

        [[nodiscard]]
        inline uint32_t LineNumber() const {
            return line_number_;
        }

        inline void SetLineNumber(uint32_t ln) {
            line_number_ = ln;
        }

        [[nodiscard]]
        inline Cursor Index() const {
            return cursor_;
        }

        inline void SetIndex(Cursor index) {
            cursor_ = index;
        }

        [[nodiscard]]
        inline uint32_t Column() const {
            return cursor_.u16 - line_start_;
        }

        [[nodiscard]]
        inline uint32_t LineStart() const {
            return line_start_;
        }

        inline void SetLineStart(uint32_t ls) {
            line_start_ = ls;
        }

        inline std::string_view View(uint32_t start, uint32_t end) {
            return source_->Data().substr(start, end - start);
        }

        std::vector<Sp<Comment>> SkipSingleLineComment(uint32_t offset);
        std::vector<Sp<Comment>> SkipMultiLineComment();
        void ScanComments(std::vector<Sp<Comment>>& result);
        static bool IsFutureReservedWord(JsTokenType t);
        static JsTokenType IsStrictModeReservedWord(std::string_view str);
        static bool IsRestrictedWord(std::string_view str_);
        static JsTokenType ToKeyword(const std::string& str_);
        bool ScanHexEscape(char32_t ch, char32_t& result);
        char32_t ScanUnicodeCodePointEscape();
        std::string GetIdentifier(int32_t start_char_len);
        std::string GetComplexIdentifier();
        bool OctalToDecimal(char16_t ch, uint32_t& result);

        Token ScanIdentifier(int32_t start_char_len);
        Token ScanPunctuator();
        Token ScanHexLiteral(uint32_t index);
        Token ScanBinaryLiteral(uint32_t index);
        Token ScanOctalLiteral(char16_t prefix, uint32_t index);
        bool IsImplicitOctalLiteral();
        Token ScanNumericLiteral();
        Token ScanStringLiteral();
        Token ScanTemplate();

        std::string ScanRegExpBody();
        std::string ScanRegExpFlags();
        Token ScanRegExp();
        Token Lex();

        [[nodiscard]]
        inline char CharAt(uint32_t index) const {
            if (unlikely(index >= source_->size())) return u'\0';
            return source_->Data().at(index);
        }

        [[nodiscard]]
        Sp<StringWithMapping> Source() const {
            return source_;
        }

        [[nodiscard]]
        inline char Peek() const {
            return CharAt(cursor_.u8);
        }

        [[nodiscard]]
        inline char Peek(uint32_t offset) const {
            return CharAt(cursor_.u8 + offset);
        }

        char32_t PeekUtf32(uint32_t* len = nullptr);
        char32_t NextUtf32();

        char NextChar();
    private:
        void ThrowUnexpectedToken();
        void ThrowUnexpectedToken(const std::string& message);

        void TolerateUnexpectedToken();
        void TolerateUnexpectedToken(const std::string& message);

        std::stack<std::string_view> curly_stack_;

        Cursor cursor_;
        uint32_t line_number_ = 1u;
        uint32_t line_start_ = 0u;  // u16 index

        Sp<parser::ParseErrorHandler> error_handler_;
        Sp<StringWithMapping> source_;
        bool is_module_ = false;

        void PlusCursor(uint32_t n);

    };

}
