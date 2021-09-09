//
// Created by Duzhong Chen on 2019/9/3.
//

#include <iostream>
#include "Scanner.h"
#include "utils/Common.h"
#include "utils/string/UChar.h"
#include "parser/ErrorMessage.h"

static int read_codepoint_from_utf8(const uint8_t buf[], uint32_t *idx, size_t strlen, uint32_t *cp)
{
    int remunits;
    uint8_t nxt, msk;
    if (*idx >= strlen)
        return -1;
    nxt = buf[(*idx)++];
    if (nxt & 0x80) {
        msk = 0xe0;
        for (remunits = 1; (nxt & msk) != (msk << 1); ++remunits)
            msk = (msk >> 1) | 0x80;
    } else {
        remunits = 0;
        msk = 0;
    }
    *cp = nxt ^ msk;
    while (remunits-- > 0) {
        *cp <<= 6;
        if (*idx >= strlen)
            return -1;
        *cp |= buf[(*idx)++] & 0x3f;
    }
    return 0;
}

namespace jetpack {

    using namespace std;

#define DO(EXPR) \
    if (!(EXPR)) return false;

    Scanner::Scanner(const Sp<StringWithMapping>& source, std::shared_ptr<parser::ParseErrorHandler> error_handler):
            source_(source), error_handler_(std::move(error_handler)) {

    }

    Scanner::ScannerState Scanner::SaveState() {
        ScannerState state {
                index_, line_number_, line_start_,
        };
        return state;
    }

    void Scanner::RestoreState(const Scanner::ScannerState &state) {
        index_ = state.index_;
        line_number_ = state.line_number_;
        line_start_ = state.line_start_;
    }

    void Scanner::ThrowUnexpectedToken() {
        ThrowUnexpectedToken(ParseMessages::UnexpectedTokenIllegal);
    }

    void Scanner::ThrowUnexpectedToken(const std::string &message) {
        throw error_handler_->CreateError(message, index_, line_number_, index_ - line_start_ + 1);
    }

    void Scanner::TolerateUnexpectedToken() {
        TolerateUnexpectedToken(ParseMessages::UnexpectedTokenIllegal);
    }

    void Scanner::TolerateUnexpectedToken(const std::string &message) {
        auto error = error_handler_->CreateError(message, index_, line_number_, index_ - line_start_ + 1);
        error_handler_->TolerateError(error);
    }

    std::vector<std::shared_ptr<Comment>> Scanner::SkipSingleLineComment(uint32_t offset) {
        std::vector<std::shared_ptr<Comment>> result;
        uint32_t start = 0;
        SourceLocation loc;

        start = index_ - offset;
        loc.start.line = line_number_;
        loc.start.column = index_ - line_start_ - offset;

        while (!IsEnd()) {
            char32_t ch;
            int32_t len = PreReadCharFromBuffer(ch);
            if (len < 0) {
                break;
            }
            index_ += len;

            if (UChar::IsLineTerminator(ch)) {
                loc.end = Position {line_number_, index_ - line_start_ - 1 };
                auto comment = new Comment {
                        false,
                        source_->ConstData().substr(start + offset, index_ - start - offset),
                        make_pair(start, index_ - 1),
                        loc
                };
                result.emplace_back(comment);
                char32_t tmp_ch;
                int32_t tmp_len = PreReadCharFromBuffer(tmp_ch);
                if (tmp_len < 0) {
                    break;
                }
                if (ch == 13 && tmp_ch == 10) {
                    index_ += tmp_len;
                }
                ++line_number_;
                line_start_ = index_;
                return result;
            }

        }

        loc.end = Position {line_number_, index_ - line_start_ };
        auto comment = new Comment {
                false,
                source_->ConstData().substr(start + offset, index_ - start - offset),
                make_pair(start, index_),
                loc,
        };
        result.emplace_back(comment);

        return result;
    }

    std::vector<std::shared_ptr<Comment>> Scanner::SkipMultiLineComment() {
        std::vector<std::shared_ptr<Comment>> result;
        uint32_t start = 0;
        SourceLocation loc;

        start = index_ - 2;
        loc.start = Position {
                line_number_,
                index_ - line_start_ - 2,
        };
        loc.end = Position {0, 0 };

        while (!IsEnd()) {
            char ch = CharAt(index_);
            if (UChar::IsLineTerminator(ch)) {
                if (ch == '\r' && CharAt(index_ + 1) == '\n') {
                    ++index_;
                }
                ++line_number_;
                ++index_;
                line_start_ = index_;
            } else if (ch == '*') {
                if (CharAt(index_ + 1) == '/') {
                    index_ += 2;
                    loc.end = Position {
                            line_number_,
                            index_ - line_start_,
                    };
                    auto comment = new Comment {
                            true,
                            source_->ConstData().substr(start + 2, index_ - start - 4),
                            make_pair(start, index_),
                            loc,
                    };
                    result.emplace_back(comment);
                    return result;
                }

                ++index_;
            } else {
                ++index_;
            }
        }

        loc.end = Position {
                line_number_,
                index_ - line_start_,
        };
        auto comment = new Comment {
                true,
                source_->ConstData().substr(start + 2, index_ - start - 2),
                make_pair(start, index_),
                loc,
        };
        result.emplace_back(comment);

        TolerateUnexpectedToken();
        return result;
    }

    void Scanner::ScanComments(std::vector<std::shared_ptr<Comment>> &result) {
        bool start = index_ == 0;

        while (!IsEnd()) {
            char ch = CharAt(index_);

            if (UChar::IsWhiteSpace(ch)) {
                ++index_;
            } else if (UChar::IsLineTerminator(ch)) {
                ++index_;

                if (ch == '\r' && CharAt(index_) == '\n') {
                    ++index_;
                }
                ++line_number_;
                line_start_ = index_;
                start = true;
            } else if (ch == '/') {
                ch = CharAt(index_ + 1);
                if (ch == '/') {
                    index_ += 2;
                    auto comments = SkipSingleLineComment(2);
                    result.insert(result.end(), comments.begin(), comments.end());
                    start = true;
                } else if (ch == '*') {
                    index_ += 2;
                    auto comments = SkipMultiLineComment();
                    result.insert(result.end(), comments.begin(), comments.end());
                } else if (start && ch == '-') {
                    if ((CharAt(index_ + 1) == '-') && (CharAt(index_ + 2) == '>')) {
                        index_ += 3;
                        auto comments = SkipSingleLineComment(3);
                        result.insert(result.end(), comments.begin(), comments.end());
                    } else {
                        break;
                    }
                } else if (ch == '<' && !is_module_) { // U+003C is '<'
                    if (source_->ConstData().substr(index_ + 1, index_ + 4) == "!--") {
                        index_ += 4; // `<!--`
                        auto comments = SkipSingleLineComment(4);
                        result.insert(result.end(), comments.begin(), comments.end());
                    } else {
                        break;
                    }
                } else {
                    break;
                }
            } else {
                break;
            }

        }
    }

    bool Scanner::ReadCharFromBuffer(char32_t &ch) {
        uint32_t code;
        if (read_codepoint_from_utf8(
                reinterpret_cast<const uint8_t *>(source_->ConstData().c_str()),
                &index_,
                source_->ConstData().size(),
                &code) != 0) {
            return false;
        }
        ch = code;
        return true;
    }

    int32_t Scanner::PreReadCharFromBuffer(char32_t &ch) {
        uint32_t code;
        uint32_t pre_saved_index = index_;
        if (read_codepoint_from_utf8(
                reinterpret_cast<const uint8_t *>(source_->ConstData().c_str()),
                &pre_saved_index,
                source_->ConstData().size(),
                &code) != 0) {
            return -1;
        }
        ch = code;
        return static_cast<int32_t>(pre_saved_index - index_);
    }

#define MAYBE_WORD(WORD) \
    if (str_ == WORD) return true;

    bool Scanner::IsFutureReservedWord(JsTokenType t) {
        return t == JsTokenType::K_Enum ||
               t == JsTokenType::K_Export ||
               t == JsTokenType::K_Import ||
               t == JsTokenType::K_Super;
    }

    JsTokenType Scanner::IsStrictModeReservedWord(std::string_view str_) {
        if (str_ == "implements") return JsTokenType::KS_Implements;
        if (str_ == "interface") return JsTokenType::KS_Interface;
        if (str_ == "package") return JsTokenType::KS_Package;
        if (str_ == "private") return JsTokenType::KS_Private;
        if (str_ == "protected") return JsTokenType::KS_Protected;
        if (str_ == "public") return JsTokenType::KS_Public;
        if (str_ == "static") return JsTokenType::KS_Static;
        if (str_ == "yield") return JsTokenType::K_Yield;
        if (str_ == "let") return JsTokenType::K_Let;
        return JsTokenType::Invalid;
    }

    bool Scanner::IsRestrictedWord(std::string_view str_) {
        MAYBE_WORD("eval")
        MAYBE_WORD("arguments")
        return false;
    }

    JsTokenType Scanner::ToKeyword(const std::string &str_) {
        switch (str_.size()) {
            case 2:
                if (str_ == "if") return JsTokenType::K_If;
                if (str_ == "in") return JsTokenType::K_In;
                if (str_ == "do") return JsTokenType::K_Do;
                return JsTokenType::Invalid;

            case 3:
                if (str_ == "var") return JsTokenType::K_Var;
                if (str_ == "for") return JsTokenType::K_For;
                if (str_ == "new") return JsTokenType::K_New;
                if (str_ == "try") return JsTokenType::K_Try;
                if (str_ == "let") return JsTokenType::K_Let;
                return JsTokenType::Invalid;

            case 4:
                if (str_ == "this") return JsTokenType::K_This;
                if (str_ == "else") return JsTokenType::K_Else;
                if (str_ == "case") return JsTokenType::K_Case;
                if (str_ == "void") return JsTokenType::K_Void;
                if (str_ == "with") return JsTokenType::K_With;
                if (str_ == "enum") return JsTokenType::K_Enum;
                return JsTokenType::Invalid;

            case 5:
                if (str_ == "while") return JsTokenType::K_While;
                if (str_ == "break") return JsTokenType::K_Break;
                if (str_ == "catch") return JsTokenType::K_Catch;
                if (str_ == "throw") return JsTokenType::K_Throw;
                if (str_ == "const") return JsTokenType::K_Const;
                if (str_ == "yield") return JsTokenType::K_Yield;
                if (str_ == "class") return JsTokenType::K_Class;
                if (str_ == "super") return JsTokenType::K_Super;
                return JsTokenType::Invalid;

            case 6:
                if (str_ == "return") return JsTokenType::K_Return;
                if (str_ == "typeof") return JsTokenType::K_Typeof;
                if (str_ == "delete") return JsTokenType::K_Delete;
                if (str_ == "switch") return JsTokenType::K_Switch;
                if (str_ == "export") return JsTokenType::K_Export;
                if (str_ == "import") return JsTokenType::K_Import;
                return JsTokenType::Invalid;

            case 7:
                if (str_ == "default") return JsTokenType::K_Default;
                if (str_ == "finally") return JsTokenType::K_Finally;
                if (str_ == "extends") return JsTokenType::K_Extends;
                return JsTokenType::Invalid;

            case 8:
                if (str_ == "function") return JsTokenType::K_Function;
                if (str_ == "continue") return JsTokenType::K_Continue;
                if (str_ == "debugger") return JsTokenType::K_Debugger;
                return JsTokenType::Invalid;

            case 10:
                if (str_ == "instanceof") return JsTokenType::K_Instanceof;
                return JsTokenType::Invalid;

            default:
                return JsTokenType::Invalid;
        }
    }

#undef MAYBE_WORD

    inline uint32_t HexValue(char32_t ch) {
        if (ch >= 'A' && ch <= 'F') {
            ch = 'a' + (ch - 'A');
        }
        return string("0123456789abcdef").find(ch);
    }

    bool Scanner::ScanHexEscape(char ch, char32_t& code) {
        uint32_t len = (ch == 'u') ? 4 : 2;

        for (uint32_t i = 0; i < len; ++i) {
            if (!IsEnd() && UChar::IsHexDigit(CharAt(index_))) {
                code = code * 16 + HexValue(CharAt(index_++));
            } else {
                return false;
            }
        }

        return true;
    }

    char32_t Scanner::ScanUnicodeCodePointEscape() {
        char ch = source_->ConstData().at(index_);
        char32_t code = 0;

        if (ch == '}') {
            ThrowUnexpectedToken();
        }

        while (!IsEnd()) {
            ch = source_->ConstData().at(index_++);
            if (!UChar::IsHexDigit(ch)) {
                break;
            }
            code = code * 16 + (ch - '0');
        }

        if (code > 0x10FFFF || ch != '}') {
            ThrowUnexpectedToken();
        }

        return code;
    }

    std::string Scanner::GetIdentifier(int32_t start_char_len) {
        uint32_t start = index_;
        index_ += start_char_len;
        std::string result;

        while (!IsEnd()) {
            char32_t ch;
            int32_t len = PreReadCharFromBuffer(ch);
            if (len < 0) {
                break;
            }
            if (ch == '\\') {
                index_ = start;
                return GetComplexIdentifier();
            } else if (ch >= 0xD800 && ch < 0xDFFF) {
                // Need to handle surrogate pairs.
                index_ = start;
                return GetComplexIdentifier();
            }
            if (UChar::IsIdentifierPart(ch)) {
                index_ += len;
            } else {
                break;
            }
        }

        result.append(source_->ConstData().substr(start, index_ - start));
        return result;
    }

    std::string Scanner::GetComplexIdentifier() {
        std::string result;

        // '\u' (U+005C, U+0075) denotes an escaped character.
        char32_t ch = 0;
        char32_t cp = 0;
        if (!ReadCharFromBuffer(cp)) {
            return "";
        }
        if (cp == '\\') {
            if (CharAt(index_) != 'u') {
                ThrowUnexpectedToken();
            }
            ++index_;
            if (CharAt(index_) == '{') {
                ++index_;
                ch = ScanUnicodeCodePointEscape();
            } else {
                if (!ScanHexEscape('u', ch) || ch == '\\' || !UChar::IsIdentifierStart(ch)) {
                    ThrowUnexpectedToken();
                }
            }
            result += StringFromUtf32(&ch, 1);
        }

        while (!IsEnd()) {
            int32_t ch_len = PreReadCharFromBuffer(cp);
            if (ch_len < 0) {
                break;
            }
            if (!UChar::IsIdentifierPart(cp)) {
                break;
            }

            std::string ch_ = StringFromCodePoint(cp);

            result.append(ch_);

            index_ += ch_len;

            // '\u' (U+005C, U+0075) denotes an escaped character.
            if (cp == '\\') {
                result = result.substr(0, result.size() - 1);
                if (source_->ConstData().at(index_) != 'u') {
                    ThrowUnexpectedToken();
                }
                ++index_;
                if (CharAt(index_) == '{') {
                    ++index_;
                    ch = ScanUnicodeCodePointEscape();
                } else {
                    if (!ScanHexEscape('u', ch) || ch == '\\' || !UChar::IsIdentifierPart(ch)) {
                        ThrowUnexpectedToken();
                    }
                }
                result += StringFromUtf32(&ch, 1);
            }
        }

        return result;
    }

    bool Scanner::OctalToDecimal(char16_t ch, uint32_t &result) {
        bool octal = (ch != '0');
        result = ch - '0';

        if (!IsEnd() && UChar::IsOctalDigit(source_->ConstData().at(index_))) {
            octal = true;
            result = result * 8 + (source_->ConstData().at(index_) - u'0');

            // 3 digits are only allowed when string starts
            // with 0, 1, 2, 3
            if (ch - u'0' && !IsEnd() && UChar::IsOctalDigit(source_->ConstData().at(index_))) {
                result = result * 8 + (source_->ConstData().at(index_) - '0');
            }
        }

        return octal;
    }

    Token Scanner::ScanIdentifier(int32_t start_char_len) {
        auto start = index_;
        Token tok;

        std::string id;
        if (source_->ConstData().at(start) == 0x5C) {
            id = GetComplexIdentifier();
        } else {
            id = GetIdentifier(start_char_len);
        }

        if (id.size() == 1) {
            tok.type = JsTokenType::Identifier;
        } else if ((tok.type = ToKeyword(id)) != JsTokenType::Invalid) {
            // nothing
        } else if (id == "null") {
            tok.type = JsTokenType::NullLiteral;
        } else if (id == "true") {
            tok.type = JsTokenType::TrueLiteral;
        } else if (id == "false") {
            tok.type = JsTokenType::FalseLiteral;
        } else {
            tok.type = JsTokenType::Identifier;
        }

        if (tok.type != JsTokenType::Identifier && (start + id.size() != index_)) {
            auto restore = index_;
            index_ = start;
            TolerateUnexpectedToken(ParseMessages::InvalidEscapedReservedWord);
            index_ = restore;
        }

        tok.value = move(id);
        tok.range = make_pair(start, index_);
        tok.lineNumber = line_number_;
        tok.lineStart = line_start_;

        return tok;
    }

    Token Scanner::ScanPunctuator() {
        auto start = index_;

        char ch = source_->ConstData().at(index_);

        JsTokenType t;
        switch (ch) {
            case '(':
                t = JsTokenType::LeftParen;
                ++index_;
                break;

            case '{':
                t = JsTokenType::LeftBracket;
                curly_stack_.push("{");
                ++index_;
                break;

            case '.':
                t = JsTokenType::Dot;
                ++index_;
                if (CharAt(index_) == '.' && CharAt(index_ + 1) == '.') {
                    // Spread operator: ...
                    t = JsTokenType::Spread;
                    index_ += 2;
                }
                break;

            case '}':
                t = JsTokenType::RightBracket;
                ++index_;
                if (!curly_stack_.empty()) {
                    curly_stack_.pop();
                }
                break;

            case ')':
                t = JsTokenType::RightParen;
                ++index_;
                break;

            case ';':
                t = JsTokenType::Semicolon;
                ++index_;
                break;

            case ',':
                t = JsTokenType::Comma;
                ++index_;
                break;

            case '[':
                t = JsTokenType::LeftBrace;
                ++index_;
                break;

            case ']':
                t = JsTokenType::RightBrace;
                ++index_;
                break;

            case ':':
                t = JsTokenType::Colon;
                ++index_;
                break;

            case '?':
                t = JsTokenType::Ask;
                ++index_;
                break;

            case '~':
                t = JsTokenType::Wave;
                ++index_;
                break;

            case '<':
                ++index_;
                if (CharAt(index_) == '<') { // <<
                    index_ ++;
                    if (CharAt(index_) == '=') { // <<=
                        index_ ++;
                        t = JsTokenType::LeftShiftAssign;
                    } else {
                        t = JsTokenType::LeftShift;
                    }
                } else if (CharAt(index_) == '=') { // <=
                    index_++;
                    t = JsTokenType::LessEqual;
                } else {
                    t = JsTokenType::LessThan;
                }
                break;

            case '>':
                ++index_;
                if (CharAt(index_) == '>') { // >>
                    index_++;
                    if (CharAt(index_) == '>') { // >>>
                        index_++;
                        if (CharAt(index_) == '=') {
                            index_++;
                            t = JsTokenType::ZeroFillRightShiftAssign;
                        } else {
                            t = JsTokenType::ZeroFillRightShift;
                        }
                    } else {
                        t = JsTokenType::RightShift;
                    }
                } else if (CharAt(index_) == '=') {
                    index_++;
                    t = JsTokenType::GreaterEqual;
                } else {
                    t = JsTokenType::GreaterThan;
                }
                break;

            case '=':
                ++index_;
                if (CharAt(index_) == '=') {
                    ++index_;
                    if (CharAt(index_) == '=') {
                        ++index_;
                        t = JsTokenType::StrictEqual;
                    } else {
                        t = JsTokenType::Equal;
                    }
                } else if (CharAt(index_) == '>') {
                    ++index_;
                    t = JsTokenType::Arrow;
                } else {
                    t = JsTokenType::Assign;
                }
                break;

            case '!':
                ++index_;
                if (CharAt(index_) == '=') {
                    ++index_;
                    if (CharAt(index_) == '=') {
                        ++index_;
                        t = JsTokenType::StrictNotEqual;
                    } else {
                        t = JsTokenType::NotEqual;
                    }
                } else {
                    t = JsTokenType::Not;
                }
                break;

            case '+':
                ++index_;
                if (CharAt(index_) == '=') {
                    ++index_;
                    t = JsTokenType::PlusAssign;
                } else if (CharAt(index_) == '+') {
                    ++index_;
                    t = JsTokenType::Increase;
                } else {
                    t = JsTokenType::Plus;
                }
                break;

            case '-':
                ++index_;
                if (CharAt(index_) == '=') {
                    ++index_;
                    t = JsTokenType::MinusAssign;
                } else if (CharAt(index_) == '-') {
                    ++index_;
                    t = JsTokenType::Decrease;
                } else {
                    t = JsTokenType::Minus;
                }
                break;

            case '*':
                ++index_;
                if (CharAt(index_) == '=') {
                    ++index_;
                    t = JsTokenType::MulAssign;
                } else if (CharAt(index_) == '*') {
                    ++index_;
                    if (CharAt(index_) == '=') {
                        ++index_;
                        t = JsTokenType::PowAssign;
                    } else {
                        t = JsTokenType::Pow;
                    }
                } else {
                    t = JsTokenType::Mul;
                }
                break;

            case '%':
                ++index_;
                if (CharAt(index_) == '=') {
                    ++index_;
                    t = JsTokenType::ModAssign;
                } else {
                    t = JsTokenType::Mod;
                }
                break;

            case '/':
                ++index_;
                if (CharAt(index_) == '=') {
                    ++index_;
                    t = JsTokenType::DivAssign;
                } else {
                    t = JsTokenType::Div;
                }
                break;

            case '^':
                ++index_;
                if (CharAt(index_) == '=') {
                    ++index_;
                    t = JsTokenType::BitXorAssign;
                } else {
                    t = JsTokenType::Xor;
                }
                break;

            case '&':
                ++index_;
                if (CharAt(index_) == '&') {
                    ++index_;
                    t = JsTokenType::And;
                } else if (CharAt(index_) == '=') {
                    ++index_;
                    t = JsTokenType::BitAndAssign;
                } else {
                    t = JsTokenType::BitAnd;
                }
                break;

            case '|':
                ++index_;
                if (CharAt(index_) == '|') {
                    ++index_;
                    t = JsTokenType::Or;
                } else if (CharAt(index_) == '=') {
                    ++index_;
                    t = JsTokenType::BitOrAssign;
                } else {
                    t = JsTokenType::BitOr;
                }
                break;

        }

        if (index_ == start) {
            ThrowUnexpectedToken();
        }

        return {
                t,
                string(),
                SourceLocation(),
                line_number_,
                line_start_,
                make_pair(start, index_)
        };
    }

    Token Scanner::ScanHexLiteral(uint32_t start) {
        std::string num;
        Token tok;

        while (!IsEnd()) {
            if (!UChar::IsHexDigit(CharAt(index_))) {
                break;
            }
            num += CharAt(index_++);
        }

        if (num.size() == 0) {
            ThrowUnexpectedToken();
        }

        if (UChar::IsIdentifierStart(CharAt(index_))) {
            ThrowUnexpectedToken();
        }

        tok.type = JsTokenType::NumericLiteral;
        tok.value = "0x" + num;
        tok.lineStart = line_start_;
        tok.lineNumber = line_number_;
        tok.range = make_pair(start, index_);

        return tok;
    }

    Token Scanner::ScanBinaryLiteral(uint32_t start) {
        std::string num;
        char16_t ch;

        while (!IsEnd()) {
            ch = source_->ConstData().at(index_);
            if (ch != '0' && ch != '1') {
                break;
            }
            num.push_back(source_->ConstData().at(index_++));
        }

        if (num.empty()) {
            // only 0b or 0B
            ThrowUnexpectedToken();
        }

        if (!IsEnd()) {
            ch = source_->ConstData().at(index_++);
            /* istanbul ignore else */
            if (UChar::IsIdentifierStart(ch) || UChar::IsDecimalDigit(ch)) {
                ThrowUnexpectedToken();
            }
        }

        return {
                JsTokenType::NumericLiteral,
                num,
                SourceLocation(),
                line_number_,
                line_start_,
                make_pair(start, index_),
        };
    }

    Token Scanner::ScanOctalLiteral(char16_t prefix, uint32_t start) {
        std::string num;
        bool octal = false;

        if (UChar::IsOctalDigit(prefix)) {
            octal = true;
            num = std::string("0") + source_->ConstData().at(index_++);
        } else {
            ++index_;
        }

        while (!IsEnd()) {
            if (!UChar::IsOctalDigit(source_->ConstData().at(index_))) {
                break;
            }
            num.push_back(source_->ConstData().at(index_++));
        }

        if (!octal && num.empty()) {
            // only 0o or 0O
            ThrowUnexpectedToken();
        }

        if (UChar::IsIdentifierStart(source_->ConstData().at(index_)) || UChar::IsDecimalDigit(source_->ConstData().at(index_))) {
            ThrowUnexpectedToken();
        }

        return {
            JsTokenType::NumericLiteral,
            num,
            SourceLocation(),
            line_number_,
            line_start_,
            make_pair(start, index_),
        };
    }

    bool Scanner::IsImplicitOctalLiteral() {
        // Implicit octal, unless there is a non-octal digit.
        // (Annex B.1.1 on Numeric Literals)
        for (uint32_t i = index_ + 1; i < Length(); ++i) {
            char ch = source_->ConstData().at(i);
            if (ch == '8' || ch == '9') {
                return false;
            }
            if (!UChar::IsOctalDigit(ch)) {
                return true;
            }
        }

        return true;
    }

    Token Scanner::ScanNumericLiteral() {
        auto start = index_;
        char ch = CharAt(start);
        if (!(UChar::IsDecimalDigit(ch) || (ch == '.'))) {
            auto err = error_handler_->CreateError("Numeric literal must start with a decimal digit or a decimal point", index_, line_number_, index_ - line_start_);
            throw err;
        }

        std::string num;
        if (ch != '.') {
            num.push_back(CharAt(index_++));
            ch = CharAt(index_);

            // Hex number starts with '0x'.
            // Octal number starts with '0'.
            // Octal number in ES6 starts with '0o'.
            // Binary number in ES6 starts with '0b'.
            if (num.at(0) == '0') {
                if (ch == 'x' || ch == 'X') {
                    ++index_;
                    return ScanHexLiteral(start);
                }
                if (ch == 'b' || ch == 'B') {
                    ++index_;
                    return ScanBinaryLiteral(start);
                }
                if (ch == 'o' || ch == 'O') {
                    return ScanOctalLiteral(ch, start);
                }

                if (ch && UChar::IsOctalDigit(ch)) {
                    if (IsImplicitOctalLiteral()) {
                        return ScanOctalLiteral(ch, start);
                    }
                }
            }

            while (UChar::IsDecimalDigit(CharAt(index_))) {
                num.push_back(CharAt(index_++));
            }
            ch = CharAt(index_);
        }

        if (ch == '.') {
            num.push_back(CharAt(index_++));
            while (UChar::IsDecimalDigit(CharAt(index_))) {
                num.push_back(CharAt(index_++));
            }
            ch = CharAt(index_);
        }

        if (ch == 'e' || ch == 'E') {
            num.push_back(CharAt(index_++));

            ch = CharAt(index_);
            if (ch == '+' || ch == '-') {
                num.push_back(CharAt(index_++));
            }
            if (UChar::IsDecimalDigit(CharAt(index_))) {
                while (UChar::IsDecimalDigit(CharAt(index_))) {
                    num.push_back(CharAt(index_++));
                }
            } else {
                ThrowUnexpectedToken();
            }
        }

        if (UChar::IsIdentifierStart(CharAt(index_))) {
            ThrowUnexpectedToken();
        }

        return {
            JsTokenType::NumericLiteral,
            num,
            SourceLocation(),
            line_number_,
            line_start_,
            make_pair(start, index_),
        };
    }

    Token Scanner::ScanStringLiteral() {
        auto start = index_;
        char quote = CharAt(start);

        if (!(quote == '\'' || quote == '"')) {
            throw error_handler_->CreateError(
                    "String literal must starts with a quote",
                    index_, line_number_, index_ - line_start_);
        }

        ++index_;
        bool octal = false;
        std::string str;

        while (!IsEnd()) {
            char ch = CharAt(index_++);

            if (ch == quote) {
                quote = 0;
                break;
            } else if (ch == '\\') {
                ch = CharAt(index_++);
                if (!ch || !UChar::IsLineTerminator(ch)) {
                    char32_t unescaped = 0;
                    switch (ch) {
                        case 'u':
                            if (CharAt(index_) == '{') {
                                ++index_;
                                char32_t tmp = ScanUnicodeCodePointEscape();

                                str.append(StringFromUtf32(&tmp, 1));
                            } else {
                                if (!ScanHexEscape(ch, unescaped)) {
                                    ThrowUnexpectedToken();
                                }

                                str.append(StringFromUtf32(&unescaped, 1));
                            }
                            break;

                        case 'x':
                            if (!ScanHexEscape(ch, unescaped)) {
                                ThrowUnexpectedToken();
                            }
                            str.append(StringFromUtf32(&unescaped, 1));
                            break;

                        case 'n':
                            str += '\n';
                            break;
                        case 'r':
                            str += '\r';
                            break;
                        case 't':
                            str += '\t';
                            break;
                        case 'b':
                            str += '\b';
                            break;
                        case 'f':
                            str += '\f';
                            break;
                        case 'v':
                            str += '\x0B';
                            break;
                        case '8':
                        case '9':
                            str += ch;
                            TolerateUnexpectedToken();
                            break;

                        default:
                            if (ch && UChar::IsOctalDigit(ch)) {
                                uint32_t octToDec;
                                octal = OctalToDecimal(ch, octToDec);

                                str.append(StringFromUtf32(&unescaped, 1));
                            } else {
                                str += ch;
                            }
                            break;
                    }
                } else {
                    ++line_number_;
                    if (ch == '\r' && CharAt(index_) == '\n') {
                        ++index_;
                    }
                    line_start_ = index_;
                }
            } else if (UChar::IsLineTerminator(ch)) {
                break;
            } else {
                str += ch;
            }
        }

        if (quote != 0) {
            index_ = start;
            ThrowUnexpectedToken();
        }

        Token tok;
        tok.type = JsTokenType::StringLiteral;
        tok.value = str;
        tok.octal = octal;
        tok.lineNumber = line_number_;
        tok.lineStart = line_start_;
        tok.range = make_pair(start, index_);

        return tok;
    }

    Token Scanner::ScanTemplate() {
        std::string cooked;
        bool terminated = false;
        uint32_t start = index_;

        bool head = (source_->ConstData().at(start) == '`');
        bool tail = false;
        uint32_t rawOffset = 2;

        ++index_;

        while (!IsEnd()) {
            char16_t ch = source_->ConstData().at(index_++);
            if (ch == '`') {
                rawOffset = 1;
                tail = true;
                terminated = true;
                break;
            } else if (ch == '$') {
                if (source_->ConstData().at(index_)== '{') {
                    curly_stack_.push("${");
                    ++index_;
                    terminated = true;
                    break;
                }
                cooked.push_back(ch);
            } else if (ch == '\\') {
                ch = source_->ConstData().at(index_++);
                if (!UChar::IsLineTerminator(ch)) {
                    switch (ch) {
                        case 'n':
                            cooked.push_back('\n');
                            break;
                        case 'r':
                            cooked.push_back('\r');
                            break;
                        case 't':
                            cooked.push_back('\t');
                            break;
                        case 'u':
                            if (source_->ConstData().at(index_) == '{') {
                                ++index_;
                                char16_t tmp = ScanUnicodeCodePointEscape();
                                cooked.push_back(tmp);
                            } else {
                                auto restore = index_;
                                char32_t unescapedChar;
                                if (ScanHexEscape(ch, unescapedChar)) {
                                    cooked.push_back(unescapedChar);
                                } else {
                                    index_= restore;
                                    cooked.push_back(ch);
                                }
                            }
                            break;
                        case 'x':
                            char32_t unescaped;
                            if (!ScanHexEscape(ch, unescaped)) {
                                ThrowUnexpectedToken();
                            }
                            cooked.push_back(unescaped);
                            break;
                        case 'b':
                            cooked.push_back('\b');
                            break;
                        case 'f':
                            cooked.push_back('\f');
                            break;
                        case 'v':
                            cooked.push_back('\v');
                            break;

                        default:
                            if (ch == '0') {
                                if (UChar::IsDecimalDigit(CharAt(index_))) {
                                    // Illegal: \01 \02 and so on
                                    ThrowUnexpectedToken();
                                }
                                cooked.push_back('\0');
                            } else if (UChar::IsOctalDigit(ch)) {
                                // Illegal: \1 \2
                                ThrowUnexpectedToken();
                            } else {
                                cooked.push_back(ch);
                            }
                            break;
                    }
                } else {
                    ++line_number_;
                    if (ch == '\r' && CharAt(index_) == '\n') {
                        ++index_;
                    }
                    line_start_ = index_;
                }
            } else if (UChar::IsLineTerminator(ch)) {
                ++line_number_;
                if (ch == '\r' && CharAt(index_) == '\n') {
                    ++index_;
                }
                line_start_  = index_;
                cooked.push_back('\n');
            } else {
                cooked.push_back(ch);
            }
        }

        if (!terminated) {
            ThrowUnexpectedToken();
        }

        if (!head) {
            curly_stack_.pop();
        }

        Token tok;
        tok.type = JsTokenType::Template;
        tok.value = source_->ConstData().substr(start + 1, index_ - rawOffset);
        tok.lineNumber = line_number_;
        tok.lineStart = line_start_;
        tok.range = make_pair(start, index_);
        tok.cooked = std::move(cooked);
        tok.head = head;
        tok.tail = tail;

        return tok;
    }

    std::string Scanner::ScanRegExpBody() {
        char16_t ch = source_->ConstData().at(index_);
        if (ch != u'/') {
            ThrowUnexpectedToken("Regular expression literal must start with a slash");
        }

        std::string str;
        str.push_back(source_->ConstData().at(index_++));
        bool class_marker = false;
        bool terminated = false;

        while (!IsEnd()) {
            ch = source_->ConstData().at(index_++);
            str.push_back(ch);
            if (ch == '\\') {
                ch = CharAt(index_++);
                if (UChar::IsLineTerminator(ch)) {
                    ThrowUnexpectedToken(ParseMessages::UnterminatedRegExp);
                }
                str.push_back(ch);
            } else if (UChar::IsLineTerminator(ch)) {
                ThrowUnexpectedToken(ParseMessages::UnterminatedRegExp);
            } else if (class_marker) {
                if (ch == ']') {
                    class_marker = false;
                }
            } else {
                if (ch == '/') {
                    terminated = true;
                    break;
                } else if (ch == '[') {
                    class_marker = true;
                }
            }
        }

        if (!terminated) {
            ThrowUnexpectedToken(ParseMessages::UnterminatedRegExp);
        }

        return str.substr(1, str.size() - 2);
    }

    std::string Scanner::ScanRegExpFlags() {
        std::string str;
        std::string flags;
        while (!IsEnd()) {
            char16_t ch = source_->ConstData().at(index_);
            if (!UChar::IsIdentifierPart(ch)) {
                break;
            }

            ++index_;
            if (ch == '\\' && !IsEnd()) {
                ch = CharAt(index_);
                if (ch == 'u') {
                    ++index_;
                    auto restore = index_;
                    char32_t char_;
                    if (ScanHexEscape('u', char_)) {
                        flags.push_back(char_);
                        for (str += "\\u"; restore < index_; ++ restore) {
                            str.push_back(CharAt(restore));
                        }
                    } else {
                        index_ = restore;
                        flags += "u";
                        str += "\\u";
                    }
                    TolerateUnexpectedToken();
                } else {
                    str += "\\";
                    TolerateUnexpectedToken();
                }
            } else {
                flags.push_back(ch);
                str.push_back(ch);
            }
        }

        return flags;
    }

    Token Scanner::ScanRegExp() {
//        auto start = index_;

        auto pattern = ScanRegExpBody();
        auto flags = ScanRegExpFlags();

        Token token;
        token.type = JsTokenType::RegularExpression;
        token.lineNumber = line_number_;
        token.lineStart = line_start_;
        token.value = "/" + pattern + "/" + flags;

        return token;
    }

    Token Scanner::Lex() {
        if (IsEnd()) {
            Token tok;
            tok.type = JsTokenType::EOF_;
            tok.lineNumber = line_number_;
            tok.lineStart = line_start_;
            tok.range = make_pair(index_, index_);
            return tok;
        }

        char32_t cp = 0;
        int32_t char_len = PreReadCharFromBuffer(cp);
        if (char_len < 0) {
            Token tok;
            tok.type = JsTokenType::EOF_;
            tok.lineNumber = line_number_;
            tok.lineStart = line_start_;
            tok.range = make_pair(index_, index_);
            return tok;
        }

        if (UChar::IsIdentifierStart(cp)) {
            return ScanIdentifier(char_len);
        }
        if (cp == '(' || cp == ')' || cp == ';') {
            return ScanPunctuator();
        }

        if (cp == '\'' || cp == '"') {
            return ScanStringLiteral();
        }

        // Dot (.) U+002E can also start a floating-point number, hence the need
        // to check the next character.
        if (cp == '.') {
            if (UChar::IsDecimalDigit(CharAt(index_ + 1))) {
                return ScanNumericLiteral();
            }
            return ScanPunctuator();
        }

        if (UChar::IsDecimalDigit(cp)) {
            return ScanNumericLiteral();
        }

        // Template literals start with ` (U+0060) for template head
        // or } (U+007D) for template middle or template tail.
        if (cp == '`' || (cp == '}' && !curly_stack_.empty() && curly_stack_.top() == "${")) {
            return ScanTemplate();
        }

        // Possible identifier start in a surrogate pair.
        if (cp >= 0xD800 && cp < 0xDFFF) {
            char32_t tmp;
            if (PreReadCharFromBuffer(tmp) >= 0 && UChar::IsIdentifierStart(tmp)) {
                return ScanIdentifier(char_len);
            }
        }

        return ScanPunctuator();
    }

}
