//
// Created by Duzhong Chen on 2019/9/3.
//

#include "scanner.h"
#include "../macros.h"

using namespace std;

#define DO(EXPR) \
    if (!(EXPR)) return false;

Scanner::Scanner(std::shared_ptr<std::u16string> source, std::shared_ptr<ParseErrorHandler> error_handler):
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

void Scanner::UnexpectedToken() {
    error_handler_->CreateError("<Unexpected Token>", "", index_, line_number_, index_ - line_start_ + 1);
}

bool Scanner::SkipSingleLineComment(std::uint32_t offset, std::vector<Comment> &result) {
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
            return true;
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

    return true;
}

bool Scanner::SkipMultiLineComment(std::vector<Comment> &result) {
    std::uint32_t start = 0;
    SourceLocation loc;

    if (track_comment_) {
        start = index_ - 2;
        loc.start_ = Position {
            line_number_,
            index_ - line_start_ - 2,
        };
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
                    return true;
                }
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

    this->UnexpectedToken();
    return true;
}

bool Scanner::ScanComments(std::vector<Comment> &result) {
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
                DO(SkipSingleLineComment(2, comments));
                if (track_comment_) {
                    result.insert(result.end(), comments.begin(), comments.end());
                }
                start = true;
            } else if (ch == 0x2A) {  // U+002A is '*'
                index_ += 2;
                vector<Comment> comments;
                DO(SkipMultiLineComment(comments));
                if (track_comment_) {
                    result.insert(result.end(), comments.begin(), comments.end());
                }
            } else if (start && ch == 0x2D) { // U+002D is '-'
                // U+003E is '>'
                if ((CodePointAt(index_ + 1) == 0x2D) && (CodePointAt(index_ + 2) == 0x3E)) {
                    // '-->' is a single-line comment
                    index_ += 3;
                    vector<Comment> comments;
                    DO(SkipSingleLineComment(3, comments))
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
                    DO(SkipSingleLineComment(4, comments))
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

    return true;
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
    if (str_ == utils::To_UTF16(WORD)) return true;

bool Scanner::IsFutureReservedWord(const UString &str_) {
    MAYBE_WORD("enum")
    MAYBE_WORD("export")
    MAYBE_WORD("import")
    MAYBE_WORD("super")
    return false;
}

bool Scanner::IsStrictModeReservedWord(const UString &str_) {
    MAYBE_WORD("implements")
    MAYBE_WORD("interface")
    MAYBE_WORD("package")
    MAYBE_WORD("private")
    MAYBE_WORD("protected")
    MAYBE_WORD("public")
    MAYBE_WORD("static")
    MAYBE_WORD("yield")
    MAYBE_WORD("let")
    return false;
}

bool Scanner::IsRestrictedWord(const UString &str_) {
    MAYBE_WORD("eval")
    MAYBE_WORD("arguments")
    return false;
}

bool Scanner::IsKeyword(const UString &str_) {
    switch (str_.size()) {
        case 2:
            MAYBE_WORD("if")
            MAYBE_WORD("in")
            MAYBE_WORD("do")
            return false;

        case 3:
            MAYBE_WORD("var")
            MAYBE_WORD("for")
            MAYBE_WORD("new")
            MAYBE_WORD("try")
            MAYBE_WORD("let")
            return false;

        case 4:
            MAYBE_WORD("this")
            MAYBE_WORD("else")
            MAYBE_WORD("case")
            MAYBE_WORD("void")
            MAYBE_WORD("with")
            MAYBE_WORD("enum")
            return false;

        case 5:
            MAYBE_WORD("while")
            MAYBE_WORD("break")
            MAYBE_WORD("catch")
            MAYBE_WORD("throw")
            MAYBE_WORD("const")
            MAYBE_WORD("yield")
            MAYBE_WORD("class")
            MAYBE_WORD("super")
            return false;

        case 6:
            MAYBE_WORD("return")
            MAYBE_WORD("typeof")
            MAYBE_WORD("delete")
            MAYBE_WORD("switch")
            MAYBE_WORD("export")
            MAYBE_WORD("import")
            return false;

        case 7:
            MAYBE_WORD("default")
            MAYBE_WORD("finally")
            MAYBE_WORD("extends")
            return false;

        case 8:
            MAYBE_WORD("function")
            MAYBE_WORD("continue")
            MAYBE_WORD("debugger")
            return false;

        case 10:
            MAYBE_WORD("instanceof");
            return false;

        default:
            return false;
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

bool Scanner::ScanUnicodeCodePointEscape(char32_t& code) {
    char16_t ch = (*source_)[index_];

    if (ch == '}') {
        this->UnexpectedToken();
        return false;
    }

    while (!IsEnd()) {
        ch = (*source_)[index_];
        if (!utils::IsHexDigit(ch)) {
            break;
        }
        code = code * 16 + (ch - '0');
    }

    if (code > 0x10FFFF || ch != '}') {
        this->UnexpectedToken();
    }

    return true;
}

bool Scanner::GetIdentifier(UString &result) {
    std::uint32_t start = index_++;

    while (!IsEnd()) {
        auto ch = (*source_)[index_];
        if (ch == 0x5C) {
            // Blackslash (U+005C) marks Unicode escape sequence.
            index_ = start;
            return GetComplexIdentifier(result);
        } else if (ch >= 0xD800 && ch < 0xDFFF) {
            // Need to handle surrogate pairs.
            index_ = start;
            return GetComplexIdentifier(result);
        }
        if (utils::IsIdentifierPart(ch)) {
            ++index_;
        } else {
            break;
        }
    }

    result.insert(result.end(), source_->begin() + start, source_->begin() + index_);

    return true;
}

bool Scanner::GetComplexIdentifier(UString &result) {
    std::uint32_t cp_size_;
    auto cp = CodePointAt(index_, &cp_size_);
    index_ += cp_size_;

    // '\u' (U+005C, U+0075) denotes an escaped character.
    char32_t ch = 0;
    if (cp == 0x5C) {
        if ((*source_)[index_] != 0x75) {
            this->UnexpectedToken();
            return false;
        }
        ++index_;
        if ((*source_)[index_] == '{') {
            ++index_;
            DO(ScanUnicodeCodePointEscape(ch))
        } else {
            if (!ScanHexEscape('u', ch) || ch == '\\' || !utils::IsIdentifierStart(ch)) {
                this->UnexpectedToken();
                return false;
            }
        }
        result.push_back(ch);
    }

    while (!IsEnd()) {
        cp = CodePointAt(index_);
        if (!utils::IsIdentifierPart(cp)) {
            break;
        }
        std::u32string tmp;
        tmp.push_back(cp);

        auto utf8 = utils::To_UTF8(tmp);
        auto utf16 = utils::To_UTF16(utf8);

        result.insert(result.end(), utf16.begin(), utf16.end());

        result.push_back(ch);
        index_ += utf8.size();

        // '\u' (U+005C, U+0075) denotes an escaped character.
        if (cp == 0x5C) {
            result = result.substr(0, result.size() - 1);
            if ((*source_)[index_] != 0x75) {
                this->UnexpectedToken();
                return false;
            }
            ++index_;
            if ((*source_)[index_] == '{') {
                ++index_;
                DO(ScanUnicodeCodePointEscape(ch))
            } else {
                if (!ScanHexEscape('u', ch) || ch == '\\' || !utils::IsIdentifierPart(ch)) {
                    this->UnexpectedToken();
                    return false;
                }
            }
            result.push_back(ch);
        }
    }

    return true;
}

bool Scanner::OctalToDecimal(char16_t ch, std::uint32_t &result) {
    bool octal = (ch != '0');
    result = ch - '0';

    while (!IsEnd() && utils::IsOctalDigit((*source_)[index_])) {
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

bool Scanner::ScanIdentifier(Token &tok) {
    auto start = index_;

    UString id;
    if ((*source_)[start] == 0x5C) {
        DO(GetComplexIdentifier(id))
    } else {
        DO(GetIdentifier(id))
    }

    if (id.size() == 1) {
        tok.type_ = JsTokenType::Identifier;
    } else if (IsKeyword(id)) {
        tok.type_ = JsTokenType::Keyword;
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
        error_handler_->CreateError("InvalidEscapedReservedWord", index_, line_number_, index_ - line_start_);
        index_ = restore;
    }

    tok.value_ = id;
    tok.range_ = make_pair(start, index_);
    tok.line_number_ = line_number_;
    tok.line_start_ = line_start_;

    return true;
}

bool Scanner::ScanPunctuator(Token &tok) {
    auto start = index_;

    char16_t ch = (*source_)[index_];
    UString str;
    str.push_back(ch);
    switch (ch) {
        case '(':
        case '{':
            if (ch == '{') {
                curly_stack_.push(u"{");
            }
            ++index_;
            break;

        case '.':
            ++index_;
            if ((*source_)[index_] == '.' && (*source_)[index_ + 1] == '.') {
                // Spread operator: ...
                index_ += 2;
                str = u"...";
            }
            break;

        case '}':
            ++index_;
            curly_stack_.pop();
            break;

        case ')':
        case ';':
        case ',':
        case '[':
        case ']':
        case ':':
        case '?':
        case '~':
            ++index_;
            break;

        default:
            // 4-character punctuator.
            str = source_->substr(index_, 4);
            if (str == u">>>=") {
                index_ += 4;
            } else {

                // 3-character punctuators.
                str = str.substr(0, 3);
                if (str == u"===" || str == u"!==" || str == u">>>" ||
                                                              str == u"<<=" || str == u">>=" || str == u"**=") {
                    index_ += 3;
                } else {

                    // 2-character punctuators.
                    str = str.substr(0, 2);
                    if (str == u"&&" || str == u"||" || str == u"==" || str == u"!=" ||
                        str == u"+=" || str == u"-=" || str == u"*=" || str == u"/=" ||
                        str == u"++" || str == u"--" ||
                        str == u"<<" || str == u">>" ||
                        str == u"&=" ||
                        str == u"|=" ||
                        str == u"^=" ||
                        str == u"%=" ||
                        str == u"<=" ||
                        str == u">=" ||
                        str == u"=>" ||
                        str == u"**") {
                        index_ += 2;
                    } else {

                        // 1-character punctuators.
                        ch = (*source_)[index_];
                        if (UString(u"<>=!+-*%&|^/").find(ch) >= 0) {
                            ++index_;
                        }
                        str.push_back(ch);
                    }
                }
            }
    }

    if (index_ == start) {
        this->UnexpectedToken();
        return false;
    }

    tok.type_ = JsTokenType::Punctuator;
    tok.value_ = str;
    tok.line_start_ = line_start_;
    tok.line_number_ = line_number_;
    tok.range_ = make_pair(start, index_);

    return true;
}

bool Scanner::ScanHexLiteral(std::uint32_t start, Token &tok) {
    UString num;

    while (!IsEnd()) {
        if (!utils::IsHexDigit(source_->at(index_))) {
            break;
        }
        num += source_->at(index_++);
    }

    if (num.size() == 0) {
        UnexpectedToken();
        return false;
    }

    if (utils::IsIdentifierStart(source_->at(index_))) {
        UnexpectedToken();
        return false;
    }

    tok.type_ = JsTokenType::Punctuator;
    tok.value_ = UString(u"0x") + num;
    tok.line_start_ = line_start_;
    tok.line_number_ = line_number_;
    tok.range_ = make_pair(start, index_);

    return true;
}

bool Scanner::ScanBinaryLiteral(std::uint32_t start, Token &tok) {
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
        this->UnexpectedToken();
        return false;
    }

    if (!IsEnd()) {
        ch = (*source_)[index_++];
        /* istanbul ignore else */
        if (utils::IsIdentifierStart(ch) || utils::IsDecimalDigit(ch)) {
            this->UnexpectedToken();
            return false;
        }
    }

    tok.type_ = JsTokenType::NumericLiteral;
    tok.value_ = num;
    tok.line_number_ = line_number_;
    tok.line_start_ = line_start_;
    tok.range_ = make_pair(start, index_);

    return true;
}

bool Scanner::ScanOctalLiteral(char16_t prefix, std::uint32_t start, Token &tok) {
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
        UnexpectedToken();
        return false;
    }

    if (utils::IsIdentifierStart((*source_)[index_]) || utils::IsDecimalDigit((*source_)[index_])) {
        UnexpectedToken();
        return false;
    }

    tok.type_ = JsTokenType::NumericLiteral;
    tok.value_ = num;
    tok.octal_ = octal;
    tok.line_number_ = line_number_;
    tok.line_start_ = line_start_;
    tok.range_ = make_pair(start, index_);

    return true;
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

bool Scanner::ScanNumericLiteral(Token &tok) {
    auto start = index_;
    char16_t ch = (*source_)[start];
    if (!(utils::IsDecimalDigit(ch) || (ch == '.'))) {
        error_handler_->CreateError("Numeric literal must start with a decimal digit or a decimal point", index_, line_number_, index_ - line_start_);
        return false;
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
                return ScanHexLiteral(start, tok);
            }
            if (ch == 'b' || ch == 'B') {
                ++index_;
                return ScanBinaryLiteral(start, tok);
            }
            if (ch == 'o' || ch == 'O') {
                return ScanOctalLiteral(ch, start, tok);
            }

            if (ch && utils::IsOctalDigit(ch)) {
                if (IsImplicitOctalLiteral()) {
                    return ScanOctalLiteral(ch, start, tok);
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
            UnexpectedToken();
            return false;
        }
    }

    if (utils::IsIdentifierStart(source_->at(index_))) {
        UnexpectedToken();
        return false;
    }

    tok.type_ = JsTokenType::NumericLiteral;
    tok.value_ = num;
    tok.line_number_ = line_number_;
    tok.line_start_ = line_start_;
    tok.range_ = make_pair(start, index_);

    return true;
}

bool Scanner::ScanStringLiteral(Token &tok) {
    auto start = index_;
    char16_t quote = source_->at(start);

    if (!(quote == '\'' || quote == '"')) {
        error_handler_->CreateError("String literal must starts with a quote", index_, line_number_, index_ - line_start_);
        return false;
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
                            char32_t tmp;
                            DO(ScanUnicodeCodePointEscape(tmp))

                            utils::AddU32ToUtf16(str, tmp);
                        } else {
                            char32_t unescapedChar;
                            if (!ScanHexEscape(ch, unescapedChar)) {
                                UnexpectedToken();
                                return false;
                            }

                            utils::AddU32ToUtf16(str, unescapedChar);
                        }
                        break;

                    case 'x':
                        if (!ScanHexEscape(ch, unescaped)) {
                            UnexpectedToken();
                            return false;
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
                        UnexpectedToken();
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
        UnexpectedToken();
        return false;
    }

    tok.type_ = JsTokenType::StringLiteral;
    tok.value_ = move(str);
    tok.octal_ = octal;
    tok.line_number_ = line_number_;
    tok.line_start_ = line_start_;
    tok.range_ = make_pair(start, index_);

    return true;
}

bool Scanner::ScanTemplate(Token &tok) {
    char16_t cooked;
    bool terminated = false;
    std::uint32_t start = index_;

    bool head = ((*source_)[start] == '`');
    bool tail = false;
    std::uint32_t rawOffset = 2;

    ++index_;

    while (!IsEnd()) {
        char16_t ch = (*source_)[index_];
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
                            char32_t tmp;
                            DO(ScanUnicodeCodePointEscape(tmp))
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
                            UnexpectedToken();
                            return false;
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
                                UnexpectedToken();
                                return false;
                            }
                            cooked += '\0';
                        } else if (utils::IsOctalDigit(ch)) {
                            // Illegal: \1 \2
                            UnexpectedToken();
                            return false;
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
        UnexpectedToken();
        return false;
    }

    if (!head) {
        curly_stack_.pop();
    }

    tok.type_ = JsTokenType::Template;
    tok.value_ = source_->substr(start + 1, index_ - rawOffset);
    tok.line_number_ = line_number_;
    tok.line_start_ = line_start_;
    tok.range_ = make_pair(start, index_);
    tok.cooked_ = cooked;
    tok.head_ = head;
    tok.tail_ = tail;

    return true;
}

bool Scanner::Lex(Token &tok) {
    if (IsEnd()) {
        tok.type_ = JsTokenType::EOF_;
        tok.line_number_ = line_number_;
        tok.line_start_ = line_start_;
        tok.range_ = make_pair(index_, index_);
        return true;
    }

    char16_t cp = (*source_)[index_];

    if (utils::IsIdentifierStart(cp)) {
        return ScanIdentifier(tok);
    }
    // Very common: ( and ) and ;
    if (cp == 0x28 || cp == 0x29 || cp == 0x3B) {
        return ScanPunctuator(tok);
    }

    // String literal starts with single quote (U+0027) or double quote (U+0022).
    if (cp == 0x27 || cp == 0x22) {
        return ScanStringLiteral(tok);
    }

    // Dot (.) U+002E can also start a floating-point number, hence the need
    // to check the next character.
    if (cp == 0x2E) {
        if (utils::IsDecimalDigit((*source_)[index_ + 1])) {
            return ScanNumericLiteral(tok);
        }
        return ScanPunctuator(tok);
    }

    if (utils::IsDecimalDigit(cp)) {
        return ScanNumericLiteral(tok);
    }

    // Template literals start with ` (U+0060) for template head
    // or } (U+007D) for template middle or template tail.
    if (cp == 0x60 || (cp == 0x7D && curly_stack_.top() == u"${")) {
        return ScanTemplate(tok);
    }

    // Possible identifier start in a surrogate pair.
    if (cp >= 0xD800 && cp < 0xDFFF) {
        if (utils::IsIdentifierStart(CodePointAt(index_))) {
            return ScanIdentifier(tok);
        }
    }


    return ScanPunctuator(tok);
}
