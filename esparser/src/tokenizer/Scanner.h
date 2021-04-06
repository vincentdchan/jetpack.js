//
// Created by Duzhong Chen on 2019/9/3.
//
#pragma once

#include <memory>
#include <vector>
#include <stack>
#include <string/UStringView.h>
#include "parser/ParseErrorHandler.h"
#include "Utils.h"
#include "Token.h"
#include "Comment.h"

namespace jetpack {

    class Scanner final {
    public:
        Scanner(const UString& source, std::shared_ptr<parser::ParseErrorHandler> error_handler);
        Scanner(const Scanner&) = delete;
        Scanner(Scanner&&) = delete;

        Scanner& operator=(const Scanner&) = delete;
        Scanner& operator=(Scanner&&) = delete;

        struct ScannerState {
            uint32_t index_ = 0;
            uint32_t line_number_ = 0;
            uint32_t line_start_ = 0;
        };

        inline int32_t Length() const {
            return source_.size();
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

        inline UStringView View(uint32_t start, uint32_t end) {
            return UStringView(source_).mid(start, end - start);
        }

        std::vector<std::shared_ptr<Comment>> SkipSingleLineComment(uint32_t offset);
        std::vector<std::shared_ptr<Comment>> SkipMultiLineComment();
        void ScanComments(std::vector<std::shared_ptr<Comment>>& result);
        static bool IsFutureReservedWord(JsTokenType t);
        static JsTokenType IsStrictModeReservedWord(UStringView str);
        static bool IsRestrictedWord(UStringView str_);
        static JsTokenType ToKeyword(const UString& str_);
        bool ScanHexEscape(char16_t ch, char32_t& result);
        char32_t ScanUnicodeCodePointEscape();
        UString GetIdentifier();
        UString GetComplexIdentifier();
        bool OctalToDecimal(char16_t ch, uint32_t& result);

        Token ScanIdentifier();
        Token ScanPunctuator();
        Token ScanHexLiteral(uint32_t index);
        Token ScanBinaryLiteral(uint32_t index);
        Token ScanOctalLiteral(char16_t prefix, uint32_t index);
        bool IsImplicitOctalLiteral();
        Token ScanNumericLiteral();
        Token ScanStringLiteral();
        Token ScanTemplate();
        UString TestRegExp(const UString& pattern, const UString& flags);

        UString ScanRegExpBody();
        UString ScanRegExpFlags();
        Token ScanRegExp();
        Token Lex();

        char32_t CodePointAt(uint32_t index, uint32_t* size_ = nullptr) const;

        inline char16_t CharAt(uint32_t index) const {
            if (unlikely(index >= source_.size())) return u'\0';
            return source_.at(index);
        }

        [[nodiscard]] UString Source() const {
            return source_;
        }

        inline UString ValueBuffer() const {
            return value_buffer_;
        }

    private:
        std::stack<UString> curly_stack_;

        uint32_t index_ = 0u;
        uint32_t line_number_ = 1u;
        uint32_t line_start_ = 0u;

        Sp<parser::ParseErrorHandler> error_handler_;
        UString source_;
        UString value_buffer_;
        bool is_module_ = false;

    };

}
