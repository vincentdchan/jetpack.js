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

    class Scanner final {
    public:
        Scanner(const Sp<StringWithMapping>& source, Sp<parser::ParseErrorHandler> error_handler);
        Scanner(const Scanner&) = delete;
        Scanner(Scanner&&) = delete;

        Scanner& operator=(const Scanner&) = delete;
        Scanner& operator=(Scanner&&) = delete;

        struct ScannerState {
        public:
            uint32_t index_ = 0;
            uint32_t line_number_ = 0;
            uint32_t line_start_ = 0;
        };

        [[nodiscard]]
        inline int32_t Length() const {
            return source_->size();
        }

        ScannerState SaveState();
        void RestoreState(const ScannerState& state);

        void ThrowUnexpectedToken();
        void ThrowUnexpectedToken(const std::string& message);

        void TolerateUnexpectedToken();
        void TolerateUnexpectedToken(const std::string& message);

        [[nodiscard]]
        inline bool IsEnd() const {
            return index_ >= Length();
        }

        [[nodiscard]]
        inline uint32_t LineNumber() const {
            return line_number_;
        }

        inline void SetLineNumber(uint32_t ln) {
            line_number_ = ln;
        }

        [[nodiscard]]
        inline uint32_t Index() const {
            return index_;
        }

        inline void SetIndex(uint32_t index) {
            index_ = index;
        }

        inline void IncreaseIndex() {
            index_++;
        }

        [[nodiscard]]
        inline uint32_t Column() const {
            return index_ - line_start_;
        }

        [[nodiscard]]
        inline uint32_t LineStart() const {
            return line_start_;
        }

        inline void SetLineStart(uint32_t ls) {
            line_start_ = ls;
        }

        inline std::string_view View(uint32_t start, uint32_t end) {
            return std::string_view (source_->data_).substr(start, end - start);
        }

        std::vector<std::shared_ptr<Comment>> SkipSingleLineComment(uint32_t offset);
        std::vector<std::shared_ptr<Comment>> SkipMultiLineComment();
        void ScanComments(std::vector<std::shared_ptr<Comment>>& result);
        static bool IsFutureReservedWord(JsTokenType t);
        static JsTokenType IsStrictModeReservedWord(std::string_view str);
        static bool IsRestrictedWord(std::string_view str_);
        static JsTokenType ToKeyword(const std::string& str_);
        bool ScanHexEscape(char ch, char32_t& result);
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

        char32_t CodePointAt(uint32_t index, uint32_t* size_ = nullptr) const;

        [[nodiscard]]
        inline char CharAt(uint32_t index) const {
            if (unlikely(index >= source_->size())) return u'\0';
            return source_->data_.at(index);
        }

        [[nodiscard]]
        Sp<StringWithMapping> Source() const {
            return source_;
        }

    private:
        bool ReadCharFromBuffer(char32_t& ch);
        int32_t PreReadCharFromBuffer(char32_t& ch);

        std::stack<std::string> curly_stack_;

        uint32_t index_ = 0u;
        uint32_t line_number_ = 1u;
        uint32_t line_start_ = 0u;

        Sp<parser::ParseErrorHandler> error_handler_;
        Sp<StringWithMapping> source_;
        bool is_module_ = false;

    };

}
