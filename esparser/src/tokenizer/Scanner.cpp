//
// Created by Duzhong Chen on 2019/9/3.
//

#include <iostream>
#include "Scanner.h"
#include "Utils.h"
#include "string/UChar.h"
#include "parser/ErrorMessage.h"

namespace jetpack {

    using namespace std;

#define DO(EXPR) \
    if (!(EXPR)) return false;

    Scanner::Scanner(const UString& source, std::shared_ptr<parser::ParseErrorHandler> error_handler):
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
            char32_t ch = CodePointAt(index_);
            index_++;

            if (UChar::IsLineTerminator(ch)) {
                loc.end = Position {line_number_, index_ - line_start_ - 1 };
                auto comment = new Comment {
                        false,
                        source_.mid(start + offset, index_ - start - offset),
                        make_pair(start, index_ - 1),
                        loc
                };
                result.emplace_back(comment);
                if (ch == 13 && CodePointAt(index_) == 10) {
                    ++index_;
                }
                ++line_number_;
                line_start_ = index_;
                return result;
            }

        }

        loc.end = Position {line_number_, index_ - line_start_ };
        auto comment = new Comment {
                false,
                source_.mid(start + offset, index_ - start - offset),
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
            char32_t ch = CodePointAt(index_);
            if (UChar::IsLineTerminator(ch)) {
                if (ch == 0x0D && CodePointAt(index_ + 1) == 0x0A) {
                    ++index_;
                }
                ++line_number_;
                ++index_;
                line_start_ = index_;
            } else if (ch == 0x2A) {
                if (CodePointAt(index_ + 1) == 0x2F) {
                    index_ += 2;
                    loc.end = Position {
                            line_number_,
                            index_ - line_start_,
                    };
                    auto comment = new Comment {
                            true,
                            source_.mid(start + 2, index_ - start - 4),
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
                source_.mid(start + 2, index_ - start - 2),
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
            char32_t ch = CodePointAt(index_);

            if (UChar::IsWhiteSpace(ch)) {
                ++index_;
            } else if (UChar::IsLineTerminator(ch)) {
                ++index_;

                if (ch == 0x0D && CodePointAt(index_) == 0x0A) {
                    ++index_;
                }
                ++line_number_;
                line_start_ = index_;
                start = true;
            } else if (ch == 0x2F) {
                ch = CodePointAt(index_ + 1);
                if (ch == 0x2F) {
                    index_ += 2;
                    auto comments = SkipSingleLineComment(2);
                    result.insert(result.end(), comments.begin(), comments.end());
                    start = true;
                } else if (ch == 0x2A) {  // U+002A is '*'
                    index_ += 2;
                    auto comments = SkipMultiLineComment();
                    result.insert(result.end(), comments.begin(), comments.end());
                } else if (start && ch == 0x2D) { // U+002D is '-'
                    // U+003E is '>'
                    if ((CodePointAt(index_ + 1) == 0x2D) && (CodePointAt(index_ + 2) == 0x3E)) {
                        // '-->' is a single-line comment
                        index_ += 3;
                        auto comments = SkipSingleLineComment(3);
                        result.insert(result.end(), comments.begin(), comments.end());
                    } else {
                        break;
                    }
                } else if (ch == 0x3C && !is_module_) { // U+003C is '<'
                    if (source_.mid(index_ + 1, index_ + 4) == u"!--") {
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

    char32_t Scanner::CodePointAt(uint32_t index, uint32_t* size) const {
        char32_t cp = source_.at(index);

        if (cp >= 0xD800 && cp <= 0xDBFF) {
            char32_t second = source_.at(index + 1);
            if (second >= 0xDC00 && second <= 0xDFFF) {
                char32_t first = cp;
                cp = (first - 0xD800) * 0x400 + second - 0xDC00 + 0x10000;
                if (size) {
                    *size = 2;
                }
            }
        }

        if (size) {
            *size = 1;
        }

        return cp;
    }

#define MAYBE_WORD(WORD) \
    if (str_ == WORD) return true;

    bool Scanner::IsFutureReservedWord(JsTokenType t) {
        return t == JsTokenType::K_Enum ||
               t == JsTokenType::K_Export ||
               t == JsTokenType::K_Import ||
               t == JsTokenType::K_Super;
    }

    JsTokenType Scanner::IsStrictModeReservedWord(UStringView str_) {
        if (str_ == UStr("implements")) return JsTokenType::KS_Implements;
        if (str_ == UStr("interface")) return JsTokenType::KS_Interface;
        if (str_ == UStr("package")) return JsTokenType::KS_Package;
        if (str_ == UStr("private")) return JsTokenType::KS_Private;
        if (str_ == UStr("protected")) return JsTokenType::KS_Protected;
        if (str_ == UStr("public")) return JsTokenType::KS_Public;
        if (str_ == UStr("static")) return JsTokenType::KS_Static;
        if (str_ == UStr("yield")) return JsTokenType::K_Yield;
        if (str_ == UStr("let")) return JsTokenType::K_Let;
        return JsTokenType::Invalid;
    }

    bool Scanner::IsRestrictedWord(UStringView str_) {
        MAYBE_WORD(UStr("eval"))
        MAYBE_WORD(UStr("arguments"))
        return false;
    }

    JsTokenType Scanner::ToKeyword(const UString &str_) {
        switch (str_.size()) {
            case 2:
                if (str_ == u"if") return JsTokenType::K_If;
                if (str_ == u"in") return JsTokenType::K_In;
                if (str_ == u"do") return JsTokenType::K_Do;
                return JsTokenType::Invalid;

            case 3:
                if (str_ == u"var") return JsTokenType::K_Var;
                if (str_ == u"for") return JsTokenType::K_For;
                if (str_ == u"new") return JsTokenType::K_New;
                if (str_ == u"try") return JsTokenType::K_Try;
                if (str_ == u"let") return JsTokenType::K_Let;
                return JsTokenType::Invalid;

            case 4:
                if (str_ == u"this") return JsTokenType::K_This;
                if (str_ == u"else") return JsTokenType::K_Else;
                if (str_ == u"case") return JsTokenType::K_Case;
                if (str_ == u"void") return JsTokenType::K_Void;
                if (str_ == u"with") return JsTokenType::K_With;
                if (str_ == u"enum") return JsTokenType::K_Enum;
                return JsTokenType::Invalid;

            case 5:
                if (str_ == u"while") return JsTokenType::K_While;
                if (str_ == u"break") return JsTokenType::K_Break;
                if (str_ == u"catch") return JsTokenType::K_Catch;
                if (str_ == u"throw") return JsTokenType::K_Throw;
                if (str_ == u"const") return JsTokenType::K_Const;
                if (str_ == u"yield") return JsTokenType::K_Yield;
                if (str_ == u"class") return JsTokenType::K_Class;
                if (str_ == u"super") return JsTokenType::K_Super;
                return JsTokenType::Invalid;

            case 6:
                if (str_ == u"return") return JsTokenType::K_Return;
                if (str_ == u"typeof") return JsTokenType::K_Typeof;
                if (str_ == u"delete") return JsTokenType::K_Delete;
                if (str_ == u"switch") return JsTokenType::K_Switch;
                if (str_ == u"export") return JsTokenType::K_Export;
                if (str_ == u"import") return JsTokenType::K_Import;
                return JsTokenType::Invalid;

            case 7:
                if (str_ == u"default") return JsTokenType::K_Default;
                if (str_ == u"finally") return JsTokenType::K_Finally;
                if (str_ == u"extends") return JsTokenType::K_Extends;
                return JsTokenType::Invalid;

            case 8:
                if (str_ == u"function") return JsTokenType::K_Function;
                if (str_ == u"continue") return JsTokenType::K_Continue;
                if (str_ == u"debugger") return JsTokenType::K_Debugger;
                return JsTokenType::Invalid;

            case 10:
                if (str_ == u"instanceof") return JsTokenType::K_Instanceof;
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

    bool Scanner::ScanHexEscape(char16_t ch, char32_t& code) {
        uint32_t len = (ch == 'u') ? 4 : 2;

        for (uint32_t i = 0; i < len; ++i) {
            if (!IsEnd() && UChar::IsHexDigit(CodePointAt(index_))) {
                code = code * 16 + HexValue(CodePointAt(index_++));
            } else {
                return false;
            }
        }

        return true;
    }

    char32_t Scanner::ScanUnicodeCodePointEscape() {
        char16_t ch = source_.at(index_);
        char32_t code = 0;

        if (ch == '}') {
            ThrowUnexpectedToken();
        }

        while (!IsEnd()) {
            ch = source_.at(index_++);
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

    UString Scanner::GetIdentifier() {
        uint32_t start = index_++;
        UString result;

        while (!IsEnd()) {
            auto ch = source_.at(index_);
            if (ch == 0x5C) {
                // Blackslash (U+005C) marks Unicode escape sequence.
                index_ = start;
                return GetComplexIdentifier();
            } else if (ch >= 0xD800 && ch < 0xDFFF) {
                // Need to handle surrogate pairs.
                index_ = start;
                return GetComplexIdentifier();
            }
            if (UChar::IsIdentifierPart(ch)) {
                ++index_;
            } else {
                break;
            }
        }

        result.append(source_.mid(start, index_ - start));
        return result;
    }

    UString Scanner::GetComplexIdentifier() {
        uint32_t cp_size_;
        auto cp = CodePointAt(index_, &cp_size_);
        index_ += cp_size_;

        UString result;

        // '\u' (U+005C, U+0075) denotes an escaped character.
        char32_t ch = 0;
        if (cp == 0x5C) {
            if (CharAt(index_) != 0x75) {
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
            result.push_back(ch);
        }

        while (!IsEnd()) {
            cp = CodePointAt(index_);
            if (!UChar::IsIdentifierPart(cp)) {
                break;
            }

            UString ch_ = FromCodePoint(cp);

            result.append(ch_);

            std::cout << index_ << std::endl;
            index_ += ch_.size();

            // '\u' (U+005C, U+0075) denotes an escaped character.
            if (cp == 0x5C) {
                result = result.mid(0, result.size() - 1);
                if (source_.at(index_) != 0x75) {
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
                result.push_back(ch);
            }
        }

        return result;
    }

    bool Scanner::OctalToDecimal(char16_t ch, uint32_t &result) {
        bool octal = (ch != '0');
        result = ch - '0';

        if (!IsEnd() && UChar::IsOctalDigit(source_.at(index_))) {
            octal = true;
            result = result * 8 + (source_.at(index_) - u'0');

            // 3 digits are only allowed when string starts
            // with 0, 1, 2, 3
            if (ch - u'0' && !IsEnd() && UChar::IsOctalDigit(source_.at(index_))) {
                result = result * 8 + (source_.at(index_) - '0');
            }
        }

        return octal;
    }

    Token Scanner::ScanIdentifier() {
        auto start = index_;
        Token tok;

        UString id;
        if (source_.at(start) == 0x5C) {
            id = GetComplexIdentifier();
        } else {
            id = GetIdentifier();
        }

        if (id.size() == 1) {
            tok.type = JsTokenType::Identifier;
        } else if ((tok.type = ToKeyword(id)) != JsTokenType::Invalid) {
            // nothing
        } else if (id == u"null") {
            tok.type = JsTokenType::NullLiteral;
        } else if (id == u"true") {
            tok.type = JsTokenType::TrueLiteral;
        } else if (id == u"false") {
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

        char16_t ch = source_.at(index_);
        UString str;

        JsTokenType t;
        switch (ch) {
            case u'(':
                t = JsTokenType::LeftParen;
                if (ch == '{') {
                    curly_stack_.push(u"{");
                }
                ++index_;
                break;

            case u'{':
                t = JsTokenType::LeftBracket;
                if (ch == '{') {
                    curly_stack_.push(u"{");
                }
                ++index_;
                break;

            case u'.':
                t = JsTokenType::Dot;
                ++index_;
                if (CharAt(index_) == '.' && CharAt(index_ + 1) == '.') {
                    // Spread operator: ...
                    t = JsTokenType::Spread;
                    index_ += 2;
                    str = u"...";
                }
                break;

            case u'}':
                t = JsTokenType::RightBracket;
                ++index_;
                if (!curly_stack_.empty()) {
                    curly_stack_.pop();
                }
                break;

            case u')':
                t = JsTokenType::RightParen;
                ++index_;
                break;

            case u';':
                t = JsTokenType::Semicolon;
                ++index_;
                break;

            case u',':
                t = JsTokenType::Comma;
                ++index_;
                break;

            case u'[':
                t = JsTokenType::LeftBrace;
                ++index_;
                break;

            case u']':
                t = JsTokenType::RightBrace;
                ++index_;
                break;

            case u':':
                t = JsTokenType::Colon;
                ++index_;
                break;

            case u'?':
                t = JsTokenType::Ask;
                ++index_;
                break;

            case u'~':
                t = JsTokenType::Wave;
                ++index_;
                break;

            case u'<':
                ++index_;
                if (CharAt(index_) == u'<') { // <<
                    index_ ++;
                    if (CharAt(index_) == u'=') { // <<=
                        index_ ++;
                        t = JsTokenType::LeftShiftAssign;
                    } else {
                        t = JsTokenType::LeftShift;
                    }
                } else if (CharAt(index_) == u'=') { // <=
                    index_++;
                    t = JsTokenType::LessEqual;
                } else {
                    t = JsTokenType::LessThan;
                }
                break;

            case u'>':
                ++index_;
                if (CharAt(index_) == u'>') { // >>
                    index_++;
                    if (CharAt(index_) == u'>') { // >>>
                        index_++;
                        if (CharAt(index_) == u'=') {
                            index_++;
                            t = JsTokenType::ZeroFillRightShiftAssign;
                        } else {
                            t = JsTokenType::ZeroFillRightShift;
                        }
                    } else {
                        t = JsTokenType::RightShift;
                    }
                } else if (CharAt(index_) == u'=') {
                    index_++;
                    t = JsTokenType::GreaterEqual;
                } else {
                    t = JsTokenType::GreaterThan;
                }
                break;

            case u'=':
                ++index_;
                if (CharAt(index_) == u'=') {
                    ++index_;
                    if (CharAt(index_) == u'=') {
                        ++index_;
                        t = JsTokenType::StrictEqual;
                    } else {
                        t = JsTokenType::Equal;
                    }
                } else if (CharAt(index_) == u'>') {
                    ++index_;
                    t = JsTokenType::Arrow;
                } else {
                    t = JsTokenType::Assign;
                }
                break;

            case u'!':
                ++index_;
                if (CharAt(index_) == u'=') {
                    ++index_;
                    if (CharAt(index_) == u'=') {
                        ++index_;
                        t = JsTokenType::StrictNotEqual;
                    } else {
                        t = JsTokenType::NotEqual;
                    }
                } else {
                    t = JsTokenType::Not;
                }
                break;

            case u'+':
                ++index_;
                if (CharAt(index_) == u'=') {
                    ++index_;
                    t = JsTokenType::PlusAssign;
                } else if (CharAt(index_) == u'+') {
                    ++index_;
                    t = JsTokenType::Increase;
                } else {
                    t = JsTokenType::Plus;
                }
                break;

            case u'-':
                ++index_;
                if (CharAt(index_) == u'=') {
                    ++index_;
                    t = JsTokenType::MinusAssign;
                } else if (CharAt(index_) == u'-') {
                    ++index_;
                    t = JsTokenType::Decrease;
                } else {
                    t = JsTokenType::Minus;
                }
                break;

            case u'*':
                ++index_;
                if (CharAt(index_) == u'=') {
                    ++index_;
                    t = JsTokenType::MulAssign;
                } else if (CharAt(index_) == u'*') {
                    ++index_;
                    if (CharAt(index_) == u'=') {
                        ++index_;
                        t = JsTokenType::PowAssign;
                    } else {
                        t = JsTokenType::Pow;
                    }
                } else {
                    t = JsTokenType::Mul;
                }
                break;

            case u'%':
                ++index_;
                if (CharAt(index_) == u'=') {
                    ++index_;
                    t = JsTokenType::ModAssign;
                } else {
                    t = JsTokenType::Mod;
                }
                break;

            case u'/':
                ++index_;
                if (CharAt(index_) == u'=') {
                    ++index_;
                    t = JsTokenType::DivAssign;
                } else {
                    t = JsTokenType::Div;
                }
                break;

            case u'^':
                ++index_;
                if (CharAt(index_) == u'=') {
                    ++index_;
                    t = JsTokenType::BitXorAssign;
                } else {
                    t = JsTokenType::Xor;
                }
                break;

            case u'&':
                ++index_;
                if (CharAt(index_) == u'&') {
                    ++index_;
                    t = JsTokenType::And;
                } else if (CharAt(index_) == u'=') {
                    ++index_;
                    t = JsTokenType::BitAndAssign;
                } else {
                    t = JsTokenType::BitAnd;
                }
                break;

            case u'|':
                ++index_;
                if (CharAt(index_) == u'|') {
                    ++index_;
                    t = JsTokenType::Or;
                } else if (CharAt(index_) == u'=') {
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

        str = source_.mid(start, index_ - start);

        return {
                t,
                str,
                SourceLocation(),
                line_number_,
                line_start_,
                make_pair(start, index_)
        };
    }

    Token Scanner::ScanHexLiteral(uint32_t start) {
        UString num;
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
        tok.value = UString(u"0x") + num;
        tok.lineStart = line_start_;
        tok.lineNumber = line_number_;
        tok.range = make_pair(start, index_);

        return tok;
    }

    Token Scanner::ScanBinaryLiteral(uint32_t start) {
        UString num;
        char16_t ch;

        while (!IsEnd()) {
            ch = source_.at(index_);
            if (ch != '0' && ch != '1') {
                break;
            }
            num.push_back(source_.at(index_++));
        }

        if (num.isEmpty()) {
            // only 0b or 0B
            ThrowUnexpectedToken();
        }

        if (!IsEnd()) {
            ch = source_.at(index_++);
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
        UString num;
        bool octal = false;

        if (UChar::IsOctalDigit(prefix)) {
            octal = true;
            num = u'0' + source_.at(index_++);
        } else {
            ++index_;
        }

        while (!IsEnd()) {
            if (!UChar::IsOctalDigit(source_.at(index_))) {
                break;
            }
            num.push_back(source_.at(index_++));
        }

        if (!octal && num.size() == 0) {
            // only 0o or 0O
            ThrowUnexpectedToken();
        }

        if (UChar::IsIdentifierStart(source_.at(index_)) || UChar::IsDecimalDigit(source_.at(index_))) {
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
            char16_t ch = source_.at(i);
            if (ch == u'8' || ch == u'9') {
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
        char16_t ch = CharAt(start);
        if (!(UChar::IsDecimalDigit(ch) || (ch == '.'))) {
            auto err = error_handler_->CreateError("Numeric literal must start with a decimal digit or a decimal point", index_, line_number_, index_ - line_start_);
            throw err;
        }

        UString num;
        if (ch != '.') {
            num.push_back(CharAt(index_++));
            ch = CharAt(index_);

            // Hex number starts with '0x'.
            // Octal number starts with '0'.
            // Octal number in ES6 starts with '0o'.
            // Binary number in ES6 starts with '0b'.
            if (num.at(0) == u'0') {
                if (ch == u'x' || ch == u'X') {
                    ++index_;
                    return ScanHexLiteral(start);
                }
                if (ch == u'b' || ch == u'B') {
                    ++index_;
                    return ScanBinaryLiteral(start);
                }
                if (ch == u'o' || ch == u'O') {
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

        if (ch == u'.') {
            num.push_back(CharAt(index_++));
            while (UChar::IsDecimalDigit(CharAt(index_))) {
                num.push_back(CharAt(index_++));
            }
            ch = CharAt(index_);
        }

        if (ch == u'e' || ch == u'E') {
            num.push_back(CharAt(index_++));

            ch = CharAt(index_);
            if (ch == u'+' || ch == u'-') {
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
        char16_t quote = CharAt(start);

        if (!(quote == '\'' || quote == '"')) {
            throw error_handler_->CreateError(
                    "String literal must starts with a quote",
                    index_, line_number_, index_ - line_start_);
        }

        ++index_;
        bool octal = false;
        UString str;

        while (!IsEnd()) {
            char16_t ch = CharAt(index_++);

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

                                str.append(UString::fromUtf32(&tmp, 1));
                            } else {
                                if (!ScanHexEscape(ch, unescaped)) {
                                    ThrowUnexpectedToken();
                                }

                                str.append(UString::fromUtf32(&unescaped, 1));
                            }
                            break;

                        case 'x':
                            if (!ScanHexEscape(ch, unescaped)) {
                                ThrowUnexpectedToken();
                            }
                            str.append(UString::fromUtf32(&unescaped, 1));
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

                                str.append(UString::fromUtf32(&unescaped, 1));
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
        UString cooked;
        bool terminated = false;
        uint32_t start = index_;

        bool head = (source_.at(start) == '`');
        bool tail = false;
        uint32_t rawOffset = 2;

        ++index_;

        while (!IsEnd()) {
            char16_t ch = source_.at(index_++);
            if (ch == '`') {
                rawOffset = 1;
                tail = true;
                terminated = true;
                break;
            } else if (ch == '$') {
                if (source_.at(index_)== '{') {
                    curly_stack_.push(u"${");
                    ++index_;
                    terminated = true;
                    break;
                }
                cooked.push_back(ch);
            } else if (ch == '\\') {
                ch = source_.at(index_++);
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
                            if (source_.at(index_) == '{') {
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
        tok.value = source_.mid(start + 1, index_ - rawOffset);
        tok.lineNumber = line_number_;
        tok.lineStart = line_start_;
        tok.range = make_pair(start, index_);
        tok.cooked = std::move(cooked);
        tok.head = head;
        tok.tail = tail;

        return tok;
    }

    UString Scanner::ScanRegExpBody() {
        char16_t ch = source_.at(index_);
        if (ch != u'/') {
            ThrowUnexpectedToken("Regular expression literal must start with a slash");
        }

        UString str;
        str.push_back(source_.at(index_++));
        bool class_marker = false;
        bool terminated = false;

        while (!IsEnd()) {
            ch = source_.at(index_++);
            str.push_back(ch);
            if (ch == u'\\') {
                ch = CharAt(index_++);
                if (UChar::IsLineTerminator(ch)) {
                    ThrowUnexpectedToken(ParseMessages::UnterminatedRegExp);
                }
                str.push_back(ch);
            } else if (UChar::IsLineTerminator(ch)) {
                ThrowUnexpectedToken(ParseMessages::UnterminatedRegExp);
            } else if (class_marker) {
                if (ch == u']') {
                    class_marker = false;
                }
            } else {
                if (ch == u'/') {
                    terminated = true;
                    break;
                } else if (ch == u'[') {
                    class_marker = true;
                }
            }
        }

        if (!terminated) {
            ThrowUnexpectedToken(ParseMessages::UnterminatedRegExp);
        }

        return str.mid(1, str.size() - 2);
    }

    UString Scanner::ScanRegExpFlags() {
        UString str;
        UString flags;
        while (!IsEnd()) {
            char16_t ch = source_.at(index_);
            if (!UChar::IsIdentifierPart(ch)) {
                break;
            }

            ++index_;
            if (ch == u'\\' && !IsEnd()) {
                ch = CharAt(index_);
                if (ch == u'u') {
                    ++index_;
                    auto restore = index_;
                    char32_t char_;
                    if (ScanHexEscape(u'u', char_)) {
                        flags.push_back(char_);
                        for (str += u"\\u"; restore < index_; ++ restore) {
                            str.push_back(CharAt(restore));
                        }
                    } else {
                        index_ = restore;
                        flags += u"u";
                        str += u"\\u";
                    }
                    TolerateUnexpectedToken();
                } else {
                    str += u"\\";
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
        token.value = UString(u"/") + pattern + u"/" + flags;

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

        char16_t cp = source_.at(index_);

        if (UChar::IsIdentifierStart(cp)) {
            return ScanIdentifier();
        }
        // Very common: ( and ) and ;
        if (cp == 0x28 || cp == 0x29 || cp == 0x3B) {
            return ScanPunctuator();
        }

        // String literal starts with single quote (U+0027) or double quote (U+0022).
        if (cp == 0x27 || cp == 0x22) {
            return ScanStringLiteral();
        }

        // Dot (.) U+002E can also start a floating-point number, hence the need
        // to check the next character.
        if (cp == 0x2E) {
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
        if (cp == 0x60 || (cp == 0x7D && !curly_stack_.empty() && curly_stack_.top() == u"${")) {
            return ScanTemplate();
        }

        // Possible identifier start in a surrogate pair.
        if (cp >= 0xD800 && cp < 0xDFFF) {
            if (UChar::IsIdentifierStart(CodePointAt(index_))) {
                return ScanIdentifier();
            }
        }

        return ScanPunctuator();
    }

}
