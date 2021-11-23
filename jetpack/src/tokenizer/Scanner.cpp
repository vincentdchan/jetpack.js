//
// Created by Duzhong Chen on 2019/9/3.
//

#include <iostream>
#include "Scanner.h"
#include "utils/Common.h"
#include "utils/string/UChar.h"
#include "parser/ErrorMessage.h"

namespace jetpack {

    using namespace std;

#define DO(EXPR) \
    if (!(EXPR)) return false;

    Scanner::Scanner(const Sp<MemoryViewOwner>& source, std::shared_ptr<parser::ParseErrorHandler> error_handler):
            source_(source), error_handler_(std::move(error_handler)) {

        u16_mapping_.resize(source_->View().size(), 0);
    }

    Scanner::ScannerState Scanner::SaveState() {
        ScannerState state {
                cursor_, line_number_, line_start_,
        };
        return state;
    }

    void Scanner::RestoreState(const Scanner::ScannerState &state) {
        cursor_ = state.cursor_;
        line_number_ = state.line_number_;
        line_start_ = state.line_start_;
    }

    void Scanner::ThrowUnexpectedToken() {
        ThrowUnexpectedToken(ParseMessages::UnexpectedTokenIllegal);
    }

    void Scanner::ThrowUnexpectedToken(const std::string &message) {
        throw error_handler_->CreateError(
                message,
                cursor_.u16,
                line_number_,
                cursor_.u16 - line_start_ + 1);
    }

    void Scanner::TolerateUnexpectedToken() {
        TolerateUnexpectedToken(ParseMessages::UnexpectedTokenIllegal);
    }

    void Scanner::TolerateUnexpectedToken(const std::string &message) {
        auto error = error_handler_->CreateError(
                message,
                cursor_.u16,
                line_number_, cursor_.u16 - line_start_ + 1);
        error_handler_->TolerateError(error);
    }

    std::vector<Sp<Comment>> Scanner::SkipSingleLineComment(uint32_t u8_offset) {
        std::vector<Sp<Comment>> result;
        SourceLocation loc;

        const uint32_t start = cursor_.u8 - u8_offset;
        loc.start.line = line_number_;
        loc.start.column = cursor_.u16 - line_start_ - u8_offset;

        int32_t count = 0;

        while (!IsEnd()) {
            char32_t ch = NextUtf32();
            if (ch == 0) {
                break;
            }

            if (UChar::IsLineTerminator(ch)) {
                loc.end = Position {
                    line_number_,
                    cursor_.u16 - line_start_ - 1
                };
                auto comment = new Comment {
                        false,
                        std::string(source_->View().substr(start + u8_offset, cursor_.u8 - start - u8_offset)),
                        make_pair(start, cursor_.u8 - 1),
                        loc
                };
                result.emplace_back(comment);
                uint32_t tmp_len = 0;
                char32_t tmp_ch = PeekUtf32(&tmp_len);
                if (tmp_ch == 0) {
                    break;
                }
                if (ch == '\r' && tmp_ch == '\n') {
                    PlusCursor(tmp_len);
                }
                ++line_number_;
                line_start_ = cursor_.u16;
                return result;
            }

        }

        loc.end = Position {line_number_, cursor_.u16 - line_start_ };
        auto comment = new Comment {
                false,
                std::string(source_->View().substr(start + u8_offset, cursor_.u8 - start - u8_offset)),
                make_pair(start, cursor_.u8),
                loc,
        };
        result.emplace_back(comment);

        return result;
    }

    std::vector<Sp<Comment>> Scanner::SkipMultiLineComment() {
        std::vector<Sp<Comment>> result;
        uint32_t start = 0;
        SourceLocation loc;

        start = cursor_.u8 - 2;
        loc.start = Position {
                line_number_,
                cursor_.u16 - line_start_ - 2,
        };
        loc.end = Position {0, 0 };

        while (!IsEnd()) {
            char ch = Peek();
            if (UChar::IsLineTerminator(ch)) {
                if (ch == '\r' && Peek(1) == '\n') {
                    NextChar();
                }
                ++line_number_;
                NextChar();
                line_start_ = cursor_.u16;
            } else if (ch == '*') {
                if (Peek(1) == '/') {
                    PlusCursor(2);
                    loc.end = Position {
                            line_number_,
                            cursor_.u16 - line_start_,
                    };
                    auto comment = new Comment {
                            true,
                            std::string(source_->View().substr(start + 2, cursor_.u8 - start - 4)),
                            make_pair(start, cursor_.u8),
                            loc,
                    };
                    result.emplace_back(comment);
                    return result;
                }

                NextChar();
            } else {
                NextUtf32();
            }
        }

        loc.end = Position {
                line_number_,
                cursor_.u16 - line_start_,
        };
        auto comment = new Comment {
                true,
                std::string(source_->View().substr(start + 2, cursor_.u8 - start - 2)),
                make_pair(start, cursor_.u8),
                loc,
        };
        result.emplace_back(comment);

        TolerateUnexpectedToken();
        return result;
    }

    void Scanner::ScanComments(std::vector<Sp<Comment>> &result) {
        bool start = cursor_.u8 == 0;

        while (!IsEnd()) {
            char ch = Peek();

            if (UChar::IsWhiteSpace(ch)) {
                NextChar();
            } else if (UChar::IsLineTerminator(ch)) {
                NextChar();

                if (ch == '\r' && Peek() == '\n') {
                    NextChar();
                }
                ++line_number_;
                line_start_ = cursor_.u16;
                start = true;
            } else if (ch == '/') {
                ch = Peek(1);
                if (ch == '/') {
                    PlusCursor(2);
                    auto comments = SkipSingleLineComment(2);
                    result.insert(result.end(), comments.begin(), comments.end());
                    start = true;
                } else if (ch == '*') {
                    PlusCursor(2);
                    auto comments = SkipMultiLineComment();
                    result.insert(result.end(), comments.begin(), comments.end());
                } else if (start && ch == '-') {
                    if ((Peek(1) == '-') && (Peek(2) == '>')) {
                        PlusCursor(3);
                        auto comments = SkipSingleLineComment(3);
                        result.insert(result.end(), comments.begin(), comments.end());
                    } else {
                        break;
                    }
                } else if (ch == '<' && !is_module_) { // U+003C is '<'
                    if (source_->View().substr(cursor_.u8 + 1, cursor_.u8 + 4) == "!--") {
                        PlusCursor(4); // `<!--`
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

    char32_t Scanner::NextUtf32() {
        uint32_t len = 0;
        char32_t result = PeekUtf32(&len);
        if (result != 0) {
            for (uint32_t i = 0; i < len; i++) {
                u16_mapping_[cursor_.u8 + i] = cursor_.u16;
            }
            cursor_.u8 += len;
            cursor_.u16 += std::max<uint32_t>(len / 2, 1);
        }
        return result;
    }

    char32_t Scanner::PeekUtf32(uint32_t* len) {
        uint32_t pre_saved_index = cursor_.u8;
        char32_t code = ReadCodepointFromUtf8(
                reinterpret_cast<const uint8_t *>(source_->View().data()),
                &pre_saved_index,
                source_->View().size());
        if (len != nullptr) {
            *len = pre_saved_index - cursor_.u8;
        }
        return code;
    }

    char Scanner::NextChar() {
        char ch = Peek();
        J_ASSERT((ch & 0x80) == 0);
        u16_mapping_[cursor_.u8] = cursor_.u16;
        cursor_.u8++;
        cursor_.u16++;
        return ch;
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

    bool Scanner::ScanHexEscape(char32_t ch, char32_t& code) {
        uint32_t len = (ch == 'u') ? 4 : 2;

        for (uint32_t i = 0; i < len; ++i) {
            if (!IsEnd() && UChar::IsHexDigit(Peek())) {
                code = code * 16 + HexValue(Peek());
                NextChar();
            } else {
                return false;
            }
        }

        return true;
    }

    char32_t Scanner::ScanUnicodeCodePointEscape() {
        char ch = Peek();
        char32_t code = 0;

        if (ch == '}') {
            ThrowUnexpectedToken();
        }

        while (!IsEnd()) {
            ch = NextChar();
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
        Cursor start = cursor_;
        PlusCursor(start_char_len);
        std::string result;

        while (!IsEnd()) {
            uint32_t len = 0;
            char32_t ch = PeekUtf32(&len);
            if (ch == 0) {
                break;
            }
            if (ch == '\\') {
                cursor_ = start;
                return GetComplexIdentifier();
            } else if (ch >= 0xD800 && ch < 0xDFFF) {
                // Need to handle surrogate pairs.
                cursor_ = start;
                return GetComplexIdentifier();
            }
            if (UChar::IsIdentifierPart(ch)) {
                PlusCursor(len);
            } else {
                break;
            }
        }

        result.append(source_->View().substr(start.u8, cursor_.u8 - start.u8));
        return result;
    }

    std::string Scanner::GetComplexIdentifier() {
        std::string result;

        // '\u' (U+005C, U+0075) denotes an escaped character.
        char32_t ch = 0;
        char32_t cp = NextUtf32();
        if (cp == 0) {
            return "";
        }
        if (cp == '\\') {
            if (Peek() != 'u') {
                ThrowUnexpectedToken();
            }
            NextChar();
            if (Peek() == '{') {
                NextChar();
                ch = ScanUnicodeCodePointEscape();
            } else {
                if (!ScanHexEscape('u', ch) || ch == '\\' || !UChar::IsIdentifierStart(ch)) {
                    ThrowUnexpectedToken();
                }
            }
            result += StringFromUtf32(&ch, 1);
        }

        while (!IsEnd()) {
            uint32_t ch_len = 0;
            cp = PeekUtf32(&ch_len);
            if (cp == 0) {
                break;
            }
            if (!UChar::IsIdentifierPart(cp)) {
                break;
            }

            std::string ch_ = StringFromCodePoint(cp);

            result.append(ch_);

            PlusCursor(ch_len);

            // '\u' (U+005C, U+0075) denotes an escaped character.
            if (cp == '\\') {
                result = result.substr(0, result.size() - 1);
                if (Peek() != 'u') {
                    ThrowUnexpectedToken();
                }
                NextChar();
                if (Peek() == '{') {
                    NextChar();
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

        if (!IsEnd() && UChar::IsOctalDigit(Peek())) {
            octal = true;
            result = result * 8 + (Peek() - u'0');

            // 3 digits are only allowed when string starts
            // with 0, 1, 2, 3
            if (ch - u'0' && !IsEnd() && UChar::IsOctalDigit(Peek())) {
                result = result * 8 + (Peek() - '0');
            }
        }

        return octal;
    }

    Token Scanner::ScanIdentifier(int32_t start_char_len) {
        auto start = cursor_;
        Token tok;

        std::string id;
        if (source_->View().at(start.u8) == '\\') {
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

        if (tok.type != JsTokenType::Identifier && (start.u8 + id.size() != cursor_.u8)) {
            auto restore = cursor_;
            cursor_ = start;
            TolerateUnexpectedToken(ParseMessages::InvalidEscapedReservedWord);
            cursor_ = restore;
        }

        tok.value = move(id);
        tok.range = make_pair(start.u8, cursor_.u8);
        tok.line_number = line_number_;
        tok.line_start = line_start_;

        return tok;
    }

    Token Scanner::ScanPunctuator() {
        auto start = cursor_;

        char ch = Peek();

        JsTokenType t;
        switch (ch) {
            case '(':
                t = JsTokenType::LeftParen;
                NextChar();
                break;

            case '{':
                t = JsTokenType::LeftBracket;
                curly_stack_.push("{");
                NextChar();
                break;

            case '.':
                t = JsTokenType::Dot;
                NextChar();
                if (Peek() == '.' && Peek(1) == '.') {
                    // Spread operator: ...
                    t = JsTokenType::Spread;
                    PlusCursor(2);
                }
                break;

            case '}':
                t = JsTokenType::RightBracket;
                NextChar();
                if (!curly_stack_.empty()) {
                    curly_stack_.pop();
                }
                break;

            case ')':
                t = JsTokenType::RightParen;
                NextChar();
                break;

            case ';':
                t = JsTokenType::Semicolon;
                NextChar();
                break;

            case ',':
                t = JsTokenType::Comma;
                NextChar();
                break;

            case '[':
                t = JsTokenType::LeftBrace;
                NextChar();
                break;

            case ']':
                t = JsTokenType::RightBrace;
                NextChar();
                break;

            case ':':
                t = JsTokenType::Colon;
                NextChar();
                break;

            case '?':
                t = JsTokenType::Ask;
                NextChar();
                break;

            case '~':
                t = JsTokenType::Wave;
                NextChar();
                break;

            case '<':
                NextChar();
                if (Peek() == '<') { // <<
                    NextChar();
                    if (Peek() == '=') { // <<=
                        NextChar();
                        t = JsTokenType::LeftShiftAssign;
                    } else {
                        t = JsTokenType::LeftShift;
                    }
                } else if (Peek() == '=') { // <=
                    NextChar();
                    t = JsTokenType::LessEqual;
                } else {
                    t = JsTokenType::LessThan;
                }
                break;

            case '>':
                NextChar();
                if (Peek() == '>') { // >>
                    NextChar();
                    if (Peek() == '>') { // >>>
                        NextChar();
                        if (Peek() == '=') {
                            NextChar();
                            t = JsTokenType::ZeroFillRightShiftAssign;
                        } else {
                            t = JsTokenType::ZeroFillRightShift;
                        }
                    } else {
                        t = JsTokenType::RightShift;
                    }
                } else if (Peek() == '=') {
                    NextChar();
                    t = JsTokenType::GreaterEqual;
                } else {
                    t = JsTokenType::GreaterThan;
                }
                break;

            case '=':
                NextChar();
                if (Peek() == '=') {
                    NextChar();
                    if (Peek() == '=') {
                        NextChar();
                        t = JsTokenType::StrictEqual;
                    } else {
                        t = JsTokenType::Equal;
                    }
                } else if (Peek() == '>') {
                    NextChar();
                    t = JsTokenType::Arrow;
                } else {
                    t = JsTokenType::Assign;
                }
                break;

            case '!':
                NextChar();
                if (Peek() == '=') {
                    NextChar();
                    if (Peek() == '=') {
                        NextChar();
                        t = JsTokenType::StrictNotEqual;
                    } else {
                        t = JsTokenType::NotEqual;
                    }
                } else {
                    t = JsTokenType::Not;
                }
                break;

            case '+':
                NextChar();
                if (Peek() == '=') {
                    NextChar();
                    t = JsTokenType::PlusAssign;
                } else if (Peek() == '+') {
                    NextChar();
                    t = JsTokenType::Increase;
                } else {
                    t = JsTokenType::Plus;
                }
                break;

            case '-':
                NextChar();
                if (Peek() == '=') {
                    NextChar();
                    t = JsTokenType::MinusAssign;
                } else if (Peek() == '-') {
                    NextChar();
                    t = JsTokenType::Decrease;
                } else {
                    t = JsTokenType::Minus;
                }
                break;

            case '*':
                NextChar();
                if (Peek() == '=') {
                    NextChar();
                    t = JsTokenType::MulAssign;
                } else if (Peek() == '*') {
                    NextChar();
                    if (Peek() == '=') {
                        NextChar();
                        t = JsTokenType::PowAssign;
                    } else {
                        t = JsTokenType::Pow;
                    }
                } else {
                    t = JsTokenType::Mul;
                }
                break;

            case '%':
                NextChar();
                if (Peek() == '=') {
                    NextChar();
                    t = JsTokenType::ModAssign;
                } else {
                    t = JsTokenType::Mod;
                }
                break;

            case '/':
                NextChar();
                if (Peek() == '=') {
                    NextChar();
                    t = JsTokenType::DivAssign;
                } else {
                    t = JsTokenType::Div;
                }
                break;

            case '^':
                NextChar();
                if (Peek() == '=') {
                    NextChar();
                    t = JsTokenType::BitXorAssign;
                } else {
                    t = JsTokenType::Xor;
                }
                break;

            case '&':
                NextChar();
                if (Peek() == '&') {
                    NextChar();
                    t = JsTokenType::And;
                } else if (Peek() == '=') {
                    NextChar();
                    t = JsTokenType::BitAndAssign;
                } else {
                    t = JsTokenType::BitAnd;
                }
                break;

            case '|':
                NextChar();
                if (Peek() == '|') {
                    NextChar();
                    t = JsTokenType::Or;
                } else if (Peek() == '=') {
                    NextChar();
                    t = JsTokenType::BitOrAssign;
                } else {
                    t = JsTokenType::BitOr;
                }
                break;

        }

        if (cursor_ == start) {
            ThrowUnexpectedToken();
        }

        return {
                t,
                string(),
                line_number_,
                line_start_,
                make_pair(start.u8, cursor_.u8)
        };
    }

    Token Scanner::ScanHexLiteral(uint32_t start) {
        std::string num;
        Token tok;

        while (!IsEnd()) {
            if (!UChar::IsHexDigit(Peek())) {
                break;
            }
            num += Peek();
            NextChar();
        }

        if (num.size() == 0) {
            ThrowUnexpectedToken();
        }

        if (UChar::IsIdentifierStart(Peek())) {
            ThrowUnexpectedToken();
        }

        tok.type = JsTokenType::NumericLiteral;
        tok.value = "0x" + num;
        tok.line_start = line_start_;
        tok.line_number = line_number_;
        tok.range = make_pair(start, cursor_.u8);

        return tok;
    }

    Token Scanner::ScanBinaryLiteral(uint32_t start) {
        std::string num;
        char16_t ch;

        while (!IsEnd()) {
            ch = Peek();
            if (ch != '0' && ch != '1') {
                break;
            }
            num.push_back(NextChar());
        }

        if (num.empty()) {
            // only 0b or 0B
            ThrowUnexpectedToken();
        }

        if (!IsEnd()) {
            ch = NextChar();
            /* istanbul ignore else */
            if (UChar::IsIdentifierStart(ch) || UChar::IsDecimalDigit(ch)) {
                ThrowUnexpectedToken();
            }
        }

        return {
                JsTokenType::NumericLiteral,
                num,
                line_number_,
                line_start_,
                make_pair(start, cursor_.u8),
        };
    }

    Token Scanner::ScanOctalLiteral(char16_t prefix, uint32_t start) {
        std::string num;
        bool octal = false;

        if (UChar::IsOctalDigit(prefix)) {
            octal = true;
            num = std::string("0") + NextChar();
        } else {
            NextChar();
        }

        while (!IsEnd()) {
            if (!UChar::IsOctalDigit(Peek())) {
                break;
            }
            num.push_back(NextChar());
        }

        if (!octal && num.empty()) {
            // only 0o or 0O
            ThrowUnexpectedToken();
        }

        if (UChar::IsIdentifierStart(Peek()) || UChar::IsDecimalDigit(Peek())) {
            ThrowUnexpectedToken();
        }

        return {
            JsTokenType::NumericLiteral,
            num,
            line_number_,
            line_start_,
            make_pair(start, cursor_.u8),
        };
    }

    bool Scanner::IsImplicitOctalLiteral() {
        // Implicit octal, unless there is a non-octal digit.
        // (Annex B.1.1 on Numeric Literals)
        for (uint32_t i = cursor_.u8 + 1; i < Length(); ++i) {
            char ch = source_->View().at(i);
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
        auto start = cursor_;
        char ch = CharAt(start.u8);
        if (!(UChar::IsDecimalDigit(ch) || (ch == '.'))) {
            auto err = error_handler_->CreateError("Numeric literal must start with a decimal digit or a decimal point", cursor_.u8, line_number_, cursor_.u16 - line_start_);
            throw err;
        }

        std::string num;
        if (ch != '.') {
            num.push_back(NextChar());
            ch = Peek();

            // Hex number starts with '0x'.
            // Octal number starts with '0'.
            // Octal number in ES6 starts with '0o'.
            // Binary number in ES6 starts with '0b'.
            if (num.at(0) == '0') {
                if (ch == 'x' || ch == 'X') {
                    NextChar();
                    return ScanHexLiteral(start.u8);
                }
                if (ch == 'b' || ch == 'B') {
                    NextChar();
                    return ScanBinaryLiteral(start.u8);
                }
                if (ch == 'o' || ch == 'O') {
                    return ScanOctalLiteral(ch, start.u8);
                }

                if (ch && UChar::IsOctalDigit(ch)) {
                    if (IsImplicitOctalLiteral()) {
                        return ScanOctalLiteral(ch, start.u8);
                    }
                }
            }

            while (UChar::IsDecimalDigit(Peek())) {
                num.push_back(NextChar());
            }
            ch = Peek();
        }

        if (ch == '.') {
            num.push_back(NextChar());
            while (UChar::IsDecimalDigit(Peek())) {
                num.push_back(NextChar());
            }
            ch = Peek();
        }

        if (ch == 'e' || ch == 'E') {
            num.push_back(NextChar());

            ch = Peek();
            if (ch == '+' || ch == '-') {
                num.push_back(NextChar());
            }
            if (UChar::IsDecimalDigit(Peek())) {
                while (UChar::IsDecimalDigit(Peek())) {
                    num.push_back(NextChar());
                }
            } else {
                ThrowUnexpectedToken();
            }
        }

        if (UChar::IsIdentifierStart(Peek())) {
            ThrowUnexpectedToken();
        }

        return {
            JsTokenType::NumericLiteral,
            num,
            line_number_,
            line_start_,
            make_pair(start.u8, cursor_.u8),
        };
    }

    Token Scanner::ScanStringLiteral() {
        auto start = cursor_;
        char quote = CharAt(start.u8);

        if (!(quote == '\'' || quote == '"')) {
            throw error_handler_->CreateError(
                    "String literal must starts with a quote",
                    cursor_.u8, line_number_, cursor_.u16 - line_start_);
        }

        NextChar();
        bool octal = false;
        std::string str;

        while (!IsEnd()) {
            char ch = NextChar();

            if (ch == quote) {
                quote = 0;
                break;
            } else if (ch == '\\') {
                ch = NextChar();
                if (!ch || !UChar::IsLineTerminator(ch)) {
                    char32_t unescaped = 0;
                    switch (ch) {
                        case 'u':
                            if (Peek() == '{') {
                                NextChar();
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
                    if (ch == '\r' && Peek() == '\n') {
                        NextChar();
                    }
                    line_start_ = cursor_.u16;
                }
            } else if (UChar::IsLineTerminator(ch)) {
                break;
            } else {
                str += ch;
            }
        }

        if (quote != 0) {
            cursor_ = start;
            ThrowUnexpectedToken();
        }

        Token tok;
        tok.type = JsTokenType::StringLiteral;
        tok.value = str;
        tok.octal = octal;
        tok.line_number = line_number_;
        tok.line_start = line_start_;
        tok.range = make_pair(start.u8, cursor_.u8);

        return tok;
    }

    Token Scanner::ScanTemplate() {
        std::string cooked;
        bool terminated = false;
        auto start = cursor_;

        bool head = (CharAt(start.u8) == '`');
        bool tail = false;

        // represent the truly offset of ending
        // if template endswith '${', offset is 2
        // if template endswith '`', offset is 1
        uint32_t raw_offset = 2;

        NextChar();

        while (!IsEnd()) {
            char32_t ch = NextUtf32();
            if (ch == '`') {
                raw_offset = 1;
                tail = true;
                terminated = true;
                break;
            } else if (ch == '$') {
                if (Peek() == '{') {
                    curly_stack_.push("${");
                    NextChar();
                    terminated = true;
                    break;
                }
                cooked += StringFromUtf32(&ch, 1);
            } else if (ch == '\\') {
                ch = NextChar();
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
                            if (Peek() == '{') {
                                NextChar();
                                char32_t tmp = ScanUnicodeCodePointEscape();
                                cooked += StringFromUtf32(&tmp, 1);
                            } else {
                                auto restore = cursor_;
                                char32_t unescapedChar = 0;
                                if (ScanHexEscape(ch, unescapedChar)) {
                                    cooked += StringFromUtf32(&unescapedChar, 1);
                                } else {
                                    cursor_= restore;
                                    cooked += StringFromUtf32(&ch, 1);
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
                                if (UChar::IsDecimalDigit(Peek())) {
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
                    if (ch == '\r' && Peek() == '\n') {
                        NextChar();
                    }
                    line_start_ = cursor_.u16;
                }
            } else if (UChar::IsLineTerminator(ch)) {
                ++line_number_;
                if (ch == '\r' && Peek() == '\n') {
                    NextChar();
                }
                line_start_  = cursor_.u16;
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
        tok.value = std::move(cooked);
        tok.line_number = line_number_;
        tok.line_start = line_start_;
        tok.range = make_pair(start.u8, cursor_.u8);
        tok.head = head;
        tok.tail = tail;

        return tok;
    }

    std::string Scanner::ScanRegExpBody() {
        char16_t ch = Peek();
        if (ch != u'/') {
            ThrowUnexpectedToken("Regular expression literal must start with a slash");
        }

        std::string str;
        str.push_back(NextChar());
        bool class_marker = false;
        bool terminated = false;

        while (!IsEnd()) {
            ch = NextChar();
            str.push_back(ch);
            if (ch == '\\') {
                ch = NextChar();
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
            char ch = Peek();
            if (!UChar::IsIdentifierPart(ch)) {
                break;
            }

            NextChar();
            if (ch == '\\' && !IsEnd()) {
                ch = Peek();
                if (ch == 'u') {
                    NextChar();
                    auto restore = cursor_;
                    char32_t char_;
                    if (ScanHexEscape('u', char_)) {
                        flags.push_back(char_);
                        for (str += "\\u"; restore.u8 < cursor_.u8;) {
                            str.push_back(CharAt(restore.u8));
                            ++restore.u8;
                            ++restore.u16;
                        }
                    } else {
                        cursor_ = restore;
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
        token.line_number = line_number_;
        token.line_start = line_start_;
        token.value = "/" + pattern + "/" + flags;

        return token;
    }

    Token Scanner::Lex() {
        if (IsEnd()) {
            Token tok;
            tok.type = JsTokenType::EOF_;
            tok.line_number = line_number_;
            tok.line_start = line_start_;
            tok.range = make_pair(cursor_.u8, cursor_.u8);
            return tok;
        }

        char cp = Peek();

        if (UChar::IsIdentifierStart(cp)) {
            // Possible identifier start in a surrogate pair.
            if (cp & 0x80) {  // is a word > 1 byte
                uint32_t char_len = 0;
                char32_t unicode = PeekUtf32(&char_len);
                return ScanIdentifier(char_len);
            }
            return ScanIdentifier(1);
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
            if (UChar::IsDecimalDigit(Peek(1))) {
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

        return ScanPunctuator();
    }

    void Scanner::PlusCursor(uint32_t n) {
        for (uint32_t i = 0; i < n; i++) {
            NextUtf32();
        }
    }

}
