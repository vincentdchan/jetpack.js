//
// Created by Duzhong Chen on 2019/9/3.
//
#pragma once

#include <memory>
#include <vector>
#include <stack>
#include "../parser/ParseErrorHandler.h"
#include "../utils.h"
#include "Token.h"
#include "Comment.h"

namespace rocket_bundle {

    class Scanner final {
    public:
        Scanner(std::shared_ptr<std::u16string> source, std::shared_ptr<parser::ParseErrorHandler> error_handler);
        Scanner(const Scanner&) = delete;
        Scanner(Scanner&&) = delete;

        Scanner& operator=(const Scanner&) = delete;
        Scanner& operator=(Scanner&&) = delete;

        struct ScannerState {
            std::uint32_t index_ = 0;
            std::uint32_t line_number_ = 0;
            std::uint32_t line_start_ = 0;
        };

        inline std::int32_t Length() const {
            return source_->size();
        }

        ScannerState SaveState();
        void RestoreState(const ScannerState& state);

        void ThrowUnexpectedToken();
        void ThrowUnexpectedToken(const std::string& message);

        void TolerateUnexpectedToken();
        void TolerateUnexpectedToken(const std::string& message);

        inline bool IsEnd() const {
            return index_ >= Length();
        }

        inline std::uint32_t LineNumber() const {
            return line_number_;
        }

        inline void SetLineNumber(std::uint32_t ln) {
            line_number_ = ln;
        }

        inline std::uint32_t Index() const {
            return index_;
        }

        inline void SetIndex(std::uint32_t index) {
            index_ = index;
        }

        inline void IncreaseIndex() {
            index_++;
        }

        inline std::uint32_t Column() const {
            return index_ - line_start_;
        }

        inline std::uint32_t LineStart() const {
            return line_start_;
        }

        inline void SetLineStart(std::uint32_t ls) {
            line_start_ = ls;
        }

        std::vector<std::shared_ptr<Comment>> SkipSingleLineComment(std::uint32_t offset);
        std::vector<std::shared_ptr<Comment>> SkipMultiLineComment();
        void ScanComments(std::vector<std::shared_ptr<Comment>>& result);
        static bool IsFutureReservedWord(JsTokenType t);
        static JsTokenType IsStrictModeReservedWord(const UString& str_);
        static bool IsRestrictedWord(const UString& str_);
        static JsTokenType ToKeyword(const UString& str_);
        bool ScanHexEscape(char16_t ch, char32_t& result);
        char32_t ScanUnicodeCodePointEscape();
        UString GetIdentifier();
        UString GetComplexIdentifier();
        bool OctalToDecimal(char16_t ch, std::uint32_t& result);

        Token ScanIdentifier();
        Token ScanPunctuator();
        Token ScanHexLiteral(std::uint32_t index);
        Token ScanBinaryLiteral(std::uint32_t index);
        Token ScanOctalLiteral(char16_t prefix, std::uint32_t index);
        bool IsImplicitOctalLiteral();
        Token ScanNumericLiteral();
        Token ScanStringLiteral();
        Token ScanTemplate();
        UString TestRegExp(const UString& pattern, const UString& flags);

        UString ScanRegExpBody();
        UString ScanRegExpFlags();
        Token ScanRegExp();
        Token Lex();

        char32_t CodePointAt(std::uint32_t index, std::uint32_t* size_ = nullptr) const;

        inline char16_t CharAt(std::uint32_t index) {
            if (index >= source_->size()) return u'\0';
            return (*source_)[index];
        }

        [[nodiscard]] std::shared_ptr<std::u16string> Source() const {
            return source_;
        }

    private:
        std::stack<UString> curly_stack_;

        std::uint32_t index_ = 0u;
        std::uint32_t line_number_ = 0u;
        std::uint32_t line_start_ = 0u;

        std::shared_ptr<parser::ParseErrorHandler> error_handler_;
        std::shared_ptr<std::u16string> source_;
        bool is_module_ = false;

    };

}
