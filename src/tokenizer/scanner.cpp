//
// Created by Duzhong Chen on 2019/9/3.
//

#include <iostream>
#include "scanner.h"
#include "../macros.h"
#include "../parser/error_message.h"

using namespace std;

#define DO(EXPR) \
    if (!(EXPR)) return false;

Scanner::Scanner(std::shared_ptr<std::u16string> source, std::shared_ptr<parser::ParseErrorHandler> error_handler):
source_(std::move(source)), error_handler_(std::move(error_handler)) {

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

void Scanner::SkipSingleLineComment(std::uint32_t offset, std::vector<Comment> &result) {
    std::uint32_t start = 0;
    SourceLocation loc;

    if (track_comment_) {
        start = index_ - offset;
        loc.start_.line_ = line_number_;
        loc.start_.column_ = index_ - line_start_ - offset;
    }

    while (!IsEnd()) {
        char32_t ch = CodePointAt(index_);
        index_++;

        if (utils::IsLineTerminator(ch)) {
            if (track_comment_) {
                loc.end_ = Position { line_number_, index_ - line_start_ - 1 };
                Comment comment {
                    false,
                    make_pair(start + offset, index_ - 1),
                    make_pair(start, index_ - 1),
                    loc
                };
                result.push_back(comment);
            }
            if (ch == 13 && CodePointAt(index_) == 10) {
                ++index_;
            }
            ++line_number_;
            line_start_ = index_;
            return;
        }

    }

    if (track_comment_) {
        loc.end_ = Position { line_number_, index_ - line_start_ };
        Comment comment {
            false,
            make_pair(start + offset, index_),
            make_pair(start, index_),
            loc,
        };
        result.push_back(comment);
    }
}

void Scanner::SkipMultiLineComment(std::vector<Comment> &result) {
    std::uint32_t start = 0;
    SourceLocation loc;

    if (track_comment_) {
        start = index_ - 2;
        loc.start_ = Position {
            line_number_,
            index_ - line_start_ - 2,
        };
        loc.end_ = Position { 0, 0 };
    }

    while (!IsEnd()) {
        char32_t ch = CodePointAt(index_);
        if (utils::IsLineTerminator(ch)) {
            if (ch == 0x0D && CodePointAt(index_ + 1) == 0x0A) {
                ++index_;
            }
            ++line_number_;
            ++index_;
            line_start_ = index_;
        } else if (ch == 0x2A) {
            if (CodePointAt(index_ + 1) == 0x2F) {
                index_ += 2;
                if (track_comment_) {
                    loc.end_ = Position {
                        line_number_,
                        index_ - line_start_,
                    };
                    Comment comment {
                        true,
                        make_pair(start + 2, index_ -2),
                        make_pair(start, index_),
                        loc,
                    };
                    result.push_back(comment);
                }
                return;
            }

            ++index_;
        } else {
            ++index_;
        }
    }

    if (track_comment_) {
        loc.end_ = Position {
            line_number_,
            index_ - line_start_,
        };
        Comment comment {
            true,
            make_pair(start + 2, index_),
            make_pair(start, index_),
            loc,
        };
        result.push_back(comment);
    }

    TolerateUnexpectedToken();
}

void Scanner::ScanComments(std::vector<Comment> &result) {
    bool start = index_ == 0;

    while (!IsEnd()) {
        char32_t ch = CodePointAt(index_);

        if (utils::IsWhiteSpace(ch)) {
            ++index_;
        } else if (utils::IsLineTerminator(ch)) {
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
                vector<Comment> comments;
                SkipSingleLineComment(2, comments);
                if (track_comment_) {
                    result.insert(result.end(), comments.begin(), comments.end());
                }
                start = true;
            } else if (ch == 0x2A) {  // U+002A is '*'
                index_ += 2;
                vector<Comment> comments;
                SkipMultiLineComment(comments);
                if (track_comment_) {
                    result.insert(result.end(), comments.begin(), comments.end());
                }
            } else if (start && ch == 0x2D) { // U+002D is '-'
                // U+003E is '>'
                if ((CodePointAt(index_ + 1) == 0x2D) && (CodePointAt(index_ + 2) == 0x3E)) {
                    // '-->' is a single-line comment
                    index_ += 3;
                    vector<Comment> comments;
                    SkipSingleLineComment(3, comments);
                    if (track_comment_) {
                        result.insert(result.end(), comments.begin(), comments.end());
                    }
                } else {
                    break;
                }
            } else if (ch == 0x3C && !is_module_) { // U+003C is '<'
                if (source_->substr(index_ + 1, index_ + 4) == u"!--") {
                    index_ += 4; // `<!--`
                    vector<Comment> comments;
                    SkipSingleLineComment(4, comments);
                    if (track_comment_) {
                        result.insert(result.end(), comments.begin(), comments.end());
                    }
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

char32_t Scanner::CodePointAt(std::uint32_t index, std::uint32_t* size) const {
    char32_t cp = (*source_)[index];

    if (cp >= 0xD800 && cp <= 0xDBFF) {
        char32_t second = (*source_)[index + 1];
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

JsTokenType Scanner::IsStrictModeReservedWord(const UString &str_) {
    if (str_ == u"implements") return JsTokenType::KS_Implements;
    if (str_ == u"interface") return JsTokenType::KS_Interface;
    if (str_ == u"package") return JsTokenType::KS_Package;
    if (str_ == u"private") return JsTokenType::KS_Private;
    if (str_ == u"protected") return JsTokenType::KS_Protected;
    if (str_ == u"public") return JsTokenType::KS_Public;
    if (str_ == u"static") return JsTokenType::KS_Static;
    if (str_ == u"yield") return JsTokenType::K_Yield;
    if (str_ == u"let") return JsTokenType::K_Let;
    return JsTokenType::Invalid;
}

bool Scanner::IsRestrictedWord(const UString &str_) {
    MAYBE_WORD(u"eval")
    MAYBE_WORD(u"arguments")
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

bool Scanner::ScanHexEscape(char16_t ch, char32_t& code) {
    std::uint32_t len = (ch == 'u') ? 4 : 2;

    for (std::uint32_t i = 0; i < len; ++i) {
        if (!IsEnd() && utils::IsHexDigit(CodePointAt(index_))) {
            code = code * 16 + (CodePointAt(index_++) - '0');
        } else {
            return false;
        }
    }

    return true;
}

char32_t Scanner::ScanUnicodeCodePointEscape() {
    char16_t ch = (*source_)[index_];
    char32_t code = 0;

    if (ch == '}') {
        ThrowUnexpectedToken();
    }

    while (!IsEnd()) {
        ch = (*source_)[index_++];
        if (!utils::IsHexDigit(ch)) {
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
    std::uint32_t start = index_++;
    UString result;

    while (!IsEnd()) {
        auto ch = (*source_)[index_];
        if (ch == 0x5C) {
            // Blackslash (U+005C) marks Unicode escape sequence.
            index_ = start;
            return GetComplexIdentifier();
        } else if (ch >= 0xD800 && ch < 0xDFFF) {
            // Need to handle surrogate pairs.
            index_ = start;
            return GetComplexIdentifier();
        }
        if (utils::IsIdentifierPart(ch)) {
            ++index_;
        } else {
            break;
        }
    }

    result.insert(result.end(), source_->begin() + start, source_->begin() + index_);
    return result;
}

UString Scanner::GetComplexIdentifier() {
    std::uint32_t cp_size_;
    auto cp = CodePointAt(index_, &cp_size_);
    index_ += cp_size_;

    UString result;

    // '\u' (U+005C, U+0075) denotes an escaped character.
    char32_t ch = 0;
    if (cp == 0x5C) {
        if ((*source_)[index_] != 0x75) {
            ThrowUnexpectedToken();
        }
        ++index_;
        if ((*source_)[index_] == '{') {
            ++index_;
            ch = ScanUnicodeCodePointEscape();
        } else {
            if (!ScanHexEscape('u', ch) || ch == '\\' || !utils::IsIdentifierStart(ch)) {
                ThrowUnexpectedToken();
            }
        }
        result.push_back(ch);
    }

    while (!IsEnd()) {
        cp = CodePointAt(index_);
        if (!utils::IsIdentifierPart(cp)) {
            break;
        }

        UString ch_ = utils::FromCodePoint(cp);

        result.insert(result.end(), ch_.begin(), ch_.end());

        std::cout << index_ << std::endl;
        index_ += ch_.size();

        // '\u' (U+005C, U+0075) denotes an escaped character.
        if (cp == 0x5C) {
            result = result.substr(0, result.size() - 1);
            if ((*source_)[index_] != 0x75) {
                ThrowUnexpectedToken();
            }
            ++index_;
            if ((*source_)[index_] == '{') {
                ++index_;
                ch = ScanUnicodeCodePointEscape();
            } else {
                if (!ScanHexEscape('u', ch) || ch == '\\' || !utils::IsIdentifierPart(ch)) {
                    ThrowUnexpectedToken();
                }
            }
            result.push_back(ch);
        }
    }

    return result;
}

bool Scanner::OctalToDecimal(char16_t ch, std::uint32_t &result) {
    bool octal = (ch != '0');
    result = ch - '0';

    if (!IsEnd() && utils::IsOctalDigit((*source_)[index_])) {
        octal = true;
        result = result * 8 + ((*source_)[index_] - '0');

        // 3 digits are only allowed when string starts
        // with 0, 1, 2, 3
        if (ch - '0' && !IsEnd() && utils::IsOctalDigit((*source_)[index_])) {
            result = result * 8 + ((*source_)[index_] - '0');
        }
    }

    return octal;
}

Token Scanner::ScanIdentifier() {
    auto start = index_;
    Token tok;

    UString id;
    if ((*source_)[start] == 0x5C) {
        id = GetComplexIdentifier();
    } else {
        id = GetIdentifier();
    }

    if (id.size() == 1) {
        tok.type_ = JsTokenType::Identifier;
    } else if ((tok.type_ = ToKeyword(id)) != JsTokenType::Invalid) {
        // nothing
    } else if (id == u"null") {
        tok.type_ = JsTokenType::NullLiteral;
    } else if (id == u"true" || id == u"false") {
        tok.type_ = JsTokenType::BooleanLiteral;
    } else {
        tok.type_ = JsTokenType::Identifier;
    }

    if (tok.type_ != JsTokenType::Identifier && (start + id.size() != index_)) {
        auto restore = index_;
        index_ = start;
        TolerateUnexpectedToken(ParseMessages::InvalidEscapedReservedWord);
        index_ = restore;
    }

    tok.value_ = id;
    tok.range_ = make_pair(start, index_);
    tok.line_number_ = line_number_;
    tok.line_start_ = line_start_;

    return tok;
}

Token Scanner::ScanPunctuator() {
    auto start = index_;

    char16_t ch = (*source_)[index_];
    UString str;
    str.push_back(ch);

    JsTokenType t;
    switch (ch) {
        case '(':
            t = JsTokenType::LeftParen;
            if (ch == '{') {
                curly_stack_.push(u"{");
            }
            ++index_;
            break;

        case '{':
            t = JsTokenType::LeftBracket;
            if (ch == '{') {
                curly_stack_.push(u"{");
            }
            ++index_;
            break;

        case '.':
            t = JsTokenType::Dot;
            ++index_;
            if ((*source_)[index_] == '.' && (*source_)[index_ + 1] == '.') {
                // Spread operator: ...
                t = JsTokenType::Spread;
                index_ += 2;
                str = u"...";
            }
            break;

        case '}':
            t = JsTokenType::RightBracket;
            ++index_;
            curly_stack_.pop();
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
            if ((*source_)[index_] == '<') { // <<
                index_ ++;
                if ((*source_)[index_] == '=') { // <<=
                    index_ ++;
                    t = JsTokenType::LeftShiftAssign;
                } else {
                    t = JsTokenType::LeftShift;
                }
            } else if ((*source_)[index_] == '=') { // <=
                index_++;
                t = JsTokenType::LessEqual;
            } else {
                t = JsTokenType::LessThan;
            }
            break;

        case '>':
            ++index_;
            if ((*source_)[index_] == '>') { // >>
                index_++;
                if ((*source_)[index_] == '>') { // >>>
                    index_++;
                    if ((*source_)[index_] == '=') {
                        index_++;
                        t = JsTokenType::ZeroFillRightShiftAssign;
                    } else {
                        t = JsTokenType::ZeroFillRightShift;
                    }
                } else {
                    t = JsTokenType::RightShift;
                }
            } else if ((*source_)[index_] == '=') {
                index_++;
                t = JsTokenType::GreaterEqual;
            } else {
                t = JsTokenType::GreaterThan;
            }
            break;

        case '=':
            ++index_;
            if ((*source_)[index_] == '=') {
                ++index_;
                if ((*source_)[index_] == '=') {
                    ++index_;
                    t = JsTokenType::StrictEqual;
                } else {
                    t = JsTokenType::Equal;
                }
            } else {
                t = JsTokenType::Assign;
            }
            break;

        case '!':
            ++index_;
            if ((*source_)[index_] == '=') {
                ++index_;
                if ((*source_)[index_] == '=') {
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
            if ((*source_)[index_] == '=') {
                ++index_;
                t = JsTokenType::PlusAssign;
            } else if ((*source_)[index_] == '+') {
                ++index_;
                t = JsTokenType::Increase;
            } else {
                t = JsTokenType::Plus;
            }
            break;

        case '-':
            ++index_;
            if ((*source_)[index_] == '=') {
                ++index_;
                t = JsTokenType::MinusAssign;
            } else if ((*source_)[index_] == '-') {
                ++index_;
                t = JsTokenType::Decrease;
            } else {
                t = JsTokenType::Minus;
            }
            break;

        case '*':
            ++index_;
            if ((*source_)[index_] == '=') {
                ++index_;
                t = JsTokenType::MulAssign;
            } else if ((*source_)[index_] == '*') {
                ++index_;
                if ((*source_)[index_] == '=') {
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
            if ((*source_)[index_] == '=') {
                ++index_;
                t = JsTokenType::ModAssign;
            } else {
                t = JsTokenType::Mod;
            }
            break;

        case '/':
            ++index_;
            if ((*source_)[index_] == '=') {
                ++index_;
                t = JsTokenType::DivAssign;
            } else {
                t = JsTokenType::Div;
            }
            break;

        case '^':
            ++index_;
            if ((*source_)[index_] == '=') {
                ++index_;
                t = JsTokenType::BitXorAssign;
            } else {
                t = JsTokenType::Xor;
            }
            break;

        case '&':
            ++index_;
            if ((*source_)[index_] == '&') {
                ++index_;
                t = JsTokenType::And;
            } else {
                t = JsTokenType::BitAnd;
            }
            break;

        case '|':
            ++index_;
            if ((*source_)[index_] == '|') {
                ++index_;
                t = JsTokenType::Or;
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
        str,
        SourceLocation(),
        line_number_,
        line_start_,
        make_pair(start, index_)
    };
}

Token Scanner::ScanHexLiteral(std::uint32_t start) {
    UString num;
    Token tok;

    while (!IsEnd()) {
        if (!utils::IsHexDigit(source_->at(index_))) {
            break;
        }
        num += source_->at(index_++);
    }

    if (num.size() == 0) {
        ThrowUnexpectedToken();
    }

    if (utils::IsIdentifierStart(source_->at(index_))) {
        ThrowUnexpectedToken();
    }

    tok.type_ = JsTokenType::NumericLiteral;
    tok.value_ = UString(u"0x") + num;
    tok.line_start_ = line_start_;
    tok.line_number_ = line_number_;
    tok.range_ = make_pair(start, index_);

    return tok;
}

Token Scanner::ScanBinaryLiteral(std::uint32_t start) {
    UString num;
    char16_t ch;

    while (!IsEnd()) {
        ch = (*source_)[index_];
        if (ch != '0' && ch != '1') {
            break;
        }
        num.push_back((*source_)[index_++]);
    }

    if (num.empty()) {
        // only 0b or 0B
        ThrowUnexpectedToken();
    }

    if (!IsEnd()) {
        ch = (*source_)[index_++];
        /* istanbul ignore else */
        if (utils::IsIdentifierStart(ch) || utils::IsDecimalDigit(ch)) {
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

Token Scanner::ScanOctalLiteral(char16_t prefix, std::uint32_t start) {
    UString num;
    bool octal = false;

    if (utils::IsOctalDigit(prefix)) {
        octal = true;
        num = '0' + (*source_)[index_++];
    } else {
        ++index_;
    }

    while (!IsEnd()) {
        if (!utils::IsOctalDigit((*source_)[index_])) {
            break;
        }
        num.push_back((*source_)[index_++]);
    }

    if (!octal && num.size() == 0) {
        // only 0o or 0O
        ThrowUnexpectedToken();
    }

    if (utils::IsIdentifierStart((*source_)[index_]) || utils::IsDecimalDigit((*source_)[index_])) {
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
    for (std::uint32_t i = index_ + 1; i < Length(); ++i) {
        char16_t ch = (*source_)[i];
        if (ch == '8' || ch == '9') {
            return false;
        }
        if (!utils::IsOctalDigit(ch)) {
            return true;
        }
    }

    return true;
}

Token Scanner::ScanNumericLiteral() {
    auto start = index_;
    char16_t ch = (*source_)[start];
    if (!(utils::IsDecimalDigit(ch) || (ch == '.'))) {
        auto err = error_handler_->CreateError("Numeric literal must start with a decimal digit or a decimal point", index_, line_number_, index_ - line_start_);
        throw err;
    }

    UString num;
    if (ch != '.') {
        num.push_back((*source_)[index_++]);
        ch = (*source_)[index_];

        // Hex number starts with '0x'.
        // Octal number starts with '0'.
        // Octal number in ES6 starts with '0o'.
        // Binary number in ES6 starts with '0b'.
        if (num[0] == '0') {
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

            if (ch && utils::IsOctalDigit(ch)) {
                if (IsImplicitOctalLiteral()) {
                    return ScanOctalLiteral(ch, start);
                }
            }
        }

        while (utils::IsDecimalDigit((*source_)[index_])) {
            num.push_back((*source_)[index_++]);
        }
        ch = (*source_)[index_];
    }

    if (ch == '.') {
        num.push_back((*source_)[index_++]);
        while (utils::IsDecimalDigit((*source_)[index_])) {
            num.push_back((*source_)[index_++]);
        }
        ch = (*source_)[index_];
    }

    if (ch == 'e' || ch == 'E') {
        num.push_back(source_->at(index_++));

        ch = source_->at(index_);
        if (ch == '+' || ch == '-') {
            num.push_back(source_->at(index_++));
        }
        if (utils::IsDecimalDigit(source_->at(index_))) {
            while (utils::IsDecimalDigit(source_->at(index_))) {
                num.push_back(source_->at(index_++));
            }
        } else {
            ThrowUnexpectedToken();
        }
    }

    if (utils::IsIdentifierStart(source_->at(index_))) {
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
    char16_t quote = source_->at(start);

    if (!(quote == '\'' || quote == '"')) {
        auto err = error_handler_->CreateError("String literal must starts with a quote", index_, line_number_, index_ - line_start_);
        throw err;
    }

    ++index_;
    bool octal = false;
    UString str;

    while (!IsEnd()) {
        char16_t ch = source_->at(index_++);

        if (ch == quote) {
            quote = 0;
            break;
        } else if (ch == '\\') {
            ch = source_->at(index_++);
            if (!ch || !utils::IsLineTerminator(ch)) {
                char32_t unescaped = 0;
                switch (ch) {
                    case 'u':
                        if (source_->at(index_) == '{') {
                            ++index_;
                            char32_t tmp = ScanUnicodeCodePointEscape();

                            utils::AddU32ToUtf16(str, tmp);
                        } else {
                            if (!ScanHexEscape(ch, unescaped)) {
                                ThrowUnexpectedToken();
                            }

                            utils::AddU32ToUtf16(str, unescaped);
                        }
                        break;

                    case 'x':
                        if (!ScanHexEscape(ch, unescaped)) {
                            ThrowUnexpectedToken();
                        }
                        utils::AddU32ToUtf16(str, unescaped);
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
                        if (ch && utils::IsOctalDigit(ch)) {
                            std::uint32_t octToDec;
                            octal = OctalToDecimal(ch, octToDec);

                            utils::AddU32ToUtf16(str, octToDec);
                        } else {
                            str += ch;
                        }
                        break;
                }
            } else {
                ++line_number_;
                if (ch == '\r' && source_->at(index_) == '\n') {
                    ++index_;
                }
                line_start_ = index_;
            }
        } else if (utils::IsLineTerminator(ch)) {
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
    tok.type_ = JsTokenType::StringLiteral;
    tok.value_ = move(str);
    tok.octal_ = octal;
    tok.line_number_ = line_number_;
    tok.line_start_ = line_start_;
    tok.range_ = make_pair(start, index_);

    return tok;
}

Token Scanner::ScanTemplate() {
    char16_t cooked;
    bool terminated = false;
    std::uint32_t start = index_;

    bool head = ((*source_)[start] == '`');
    bool tail = false;
    std::uint32_t rawOffset = 2;

    ++index_;

    while (!IsEnd()) {
        char16_t ch = (*source_)[index_++];
        if (ch == '`') {
            rawOffset = 1;
            tail = true;
            terminated = true;
            break;
        } else if (ch == '$') {
            if ((*source_)[index_]== '{') {
                curly_stack_.push(u"${");
                ++index_;
                terminated = true;
                break;
            }
            cooked += ch;
        } else if (ch == '\\') {
            ch = (*source_)[index_++];
            if (!utils::IsLineTerminator(ch)) {
                switch (ch) {
                    case 'n':
                        cooked += '\n';
                        break;
                    case 'r':
                        cooked += '\r';
                        break;
                    case 't':
                        cooked += '\t';
                        break;
                    case 'u':
                        if ((*source_)[index_] == '{') {
                            ++index_;
                            char32_t tmp = ScanUnicodeCodePointEscape();
                            cooked = tmp;
                        } else {
                            auto restore = index_;
                            char32_t unescapedChar;
                            if (ScanHexEscape(ch, unescapedChar)) {
                                cooked = unescapedChar;
                            } else {
                                index_= restore;
                                cooked = ch;
                            }
                        }
                        break;
                    case 'x':
                        char32_t unescaped;
                        if (!ScanHexEscape(ch, unescaped)) {
                            ThrowUnexpectedToken();
                        }
                        cooked = unescaped;
                        break;
                    case 'b':
                        cooked += '\b';
                        break;
                    case 'f':
                        cooked += '\f';
                        break;
                    case 'v':
                        cooked += '\v';
                        break;

                    default:
                        if (ch == '0') {
                            if (utils::IsDecimalDigit(source_->at(index_))) {
                                // Illegal: \01 \02 and so on
                                ThrowUnexpectedToken();
                            }
                            cooked += '\0';
                        } else if (utils::IsOctalDigit(ch)) {
                            // Illegal: \1 \2
                            ThrowUnexpectedToken();
                        } else {
                            cooked = ch;
                        }
                        break;
                }
            } else {
                ++line_number_;
                if (ch == '\r' && source_->at(index_) == '\n') {
                    ++index_;
                }
                line_start_ = index_;
            }
        } else if (utils::IsLineTerminator(ch)) {
            ++line_number_;
            if (ch == '\r' && source_->at(index_) == '\n') {
                ++index_;
            }
            line_start_  = index_;
            cooked = '\n';
        } else {
            cooked = ch;
        }
    }

    if (!terminated) {
        ThrowUnexpectedToken();
    }

    if (!head) {
        curly_stack_.pop();
    }

    Token tok;
    tok.type_ = JsTokenType::Template;
    tok.value_ = source_->substr(start + 1, index_ - rawOffset);
    tok.line_number_ = line_number_;
    tok.line_start_ = line_start_;
    tok.range_ = make_pair(start, index_);
    tok.cooked_ = cooked;
    tok.head_ = head;
    tok.tail_ = tail;

    return tok;
}

UString Scanner::ScanRegExpBody() {
    char16_t ch = (*source_)[index_];
    if (ch != u'/') {
        ThrowUnexpectedToken("Regular expression literal must start with a slash");
    }

    UString str;
    str.push_back((*source_)[index_++]);
    bool class_marker = false;
    bool terminated = false;

    while (!IsEnd()) {
        ch = (*source_)[index_++];
        str.push_back(ch);
        if (ch == u'\\') {
            ch = (*source_)[index_++];
            if (utils::IsLineTerminator(ch)) {
                ThrowUnexpectedToken(ParseMessages::UnterminatedRegExp);
            }
            str.push_back(ch);
        } else if (utils::IsLineTerminator(ch)) {
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

    return str.substr(1, str.size() - 2);
}

UString Scanner::ScanRegExpFlags() {
    UString str;
    UString flags;
    while (!IsEnd()) {
        char16_t ch = (*source_)[index_];
        if (!utils::IsIdentifierPart(ch)) {
            break;
        }

        ++index_;
        if (ch == u'\\' && !IsEnd()) {
            ch = (*source_)[index_];
            if (ch == u'u') {
                ++index_;
                auto restore = index_;
                char32_t char_;
                if (ScanHexEscape(u'u', char_)) {
                    flags.push_back(char_);
                    for (str += u"\\u"; restore < index_; ++ restore) {
                        str.push_back((*source_)[restore]);
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
//    auto start = index_;

    auto pattern = ScanRegExpBody();
    auto flags = ScanRegExpFlags();

    Token token;
    token.type_ = JsTokenType::RegularExpression;
    token.line_number_ = line_number_;
    token.line_start_ = line_start_;
    token.value_ = UString(u"/") + pattern + u"/" + flags;

    return token;
}

Token Scanner::Lex() {
    if (IsEnd()) {
        Token tok;
        tok.type_ = JsTokenType::EOF_;
        tok.line_number_ = line_number_;
        tok.line_start_ = line_start_;
        tok.range_ = make_pair(index_, index_);
        return tok;
    }

    char16_t cp = (*source_)[index_];

    if (utils::IsIdentifierStart(cp)) {
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
        if (utils::IsDecimalDigit((*source_)[index_ + 1])) {
            return ScanNumericLiteral();
        }
        return ScanPunctuator();
    }

    if (utils::IsDecimalDigit(cp)) {
        return ScanNumericLiteral();
    }

    // Template literals start with ` (U+0060) for template head
    // or } (U+007D) for template middle or template tail.
    if (cp == 0x60 || (cp == 0x7D && curly_stack_.top() == u"${")) {
        return ScanTemplate();
    }

    // Possible identifier start in a surrogate pair.
    if (cp >= 0xD800 && cp < 0xDFFF) {
        if (utils::IsIdentifierStart(CodePointAt(index_))) {
            return ScanIdentifier();
        }
    }


    return ScanPunctuator();
}
