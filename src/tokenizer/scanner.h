//
// Created by Duzhong Chen on 2019/9/3.
//
#pragma once

#include <memory>
#include <vector>
#include <stack>
#include "../parse_error_handler.h"
#include "../utils.h"
#include "token.h"

struct Comment {
    bool multi_line_;
    std::pair<std::uint32_t, std::uint32_t> slice_;
    std::pair<std::uint32_t, std::uint32_t> range_;
    SourceLocation loc_;
};

class Scanner final: ParseErrorHandler {
public:
    Scanner(std::shared_ptr<std::u16string> source);
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

    void UnexpectedToken();

    inline bool IsEnd() const {
        return index_ >= Length();
    }

    bool SkipSingleLineComment(std::uint32_t offset, std::vector<Comment>& result);
    bool SkipMultiLineComment(std::vector<Comment>& result);
    bool ScanComments(std::vector<Comment>& result);
    static bool IsFutureReservedWord(const UString& str_);
    static bool IsStrictModeReservedWord(const UString& str_);
    static bool IsRestrictedWord(const UString& str_);
    static bool IsKeyword(const UString& str_);
    bool ScanHexEscape(char16_t ch, char32_t& result);
    bool ScanUnicodeCodePointEscape(char32_t& ch);
    bool GetIdentifier(UString& result);
    bool GetComplexIdentifier(UString& result);
    bool OctalToDecimal(char16_t ch, std::uint32_t& result);

    bool ScanIdentifier(Token& tok);
    bool ScanPunctuator(Token& tok);
    bool ScanHexLiteral(std::uint32_t index, Token& tok);
    bool ScanBinaryLiteral(std::uint32_t index, Token& tok);
    bool ScanOctalLiteral(char16_t prefix, std::uint32_t index, Token& tok);
    bool IsImplicitOctalLiteral();
    bool ScanNumericLiteral(Token& tok);
    bool ScanStringLiteral(Token& tok);
    bool ScanTemplate(Token& tok);
    bool TestRegExp(const UString& pattern, const UString& flags, UString& regex);
    bool ScanRegExpBody(UString& result);
    bool ScanRegExpFlags(UString& result);
    bool ScanRegExp(Token& tok);
    bool Lex(Token& tok);

    char32_t CodePointAt(std::uint32_t index, std::uint32_t* size_ = nullptr) const;

private:
    std::stack<UString> curly_stack_;

    std::uint32_t index_ = 0u;
    std::uint32_t line_number_ = 0u;
    std::uint32_t line_start_ = 0u;

    std::shared_ptr<std::u16string> source_;
    bool track_comment_ = false;
    bool is_module_ = false;

};
