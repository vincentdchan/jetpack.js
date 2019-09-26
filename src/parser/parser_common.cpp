//
// Created by Duzhong Chen on 2019/9/6.
//

#include "parser_common.h"
#include "error_message.h"

namespace parser {

    ParserCommon::Config ParserCommon::Config::Default() {
        return {
            nullopt,
            false,
            true,
            false,
        };
    }

    ParserCommon::ParserCommon(shared_ptr <std::u16string> source, const ParserCommon::Config& config):
        source_(source), config_(config) {

        error_handler_ = std::make_shared<ParseErrorHandler>();

        scanner_ = make_unique<Scanner>(source, error_handler_);
        scanner_->SetTrackComment(config_.comment);
        has_line_terminator_ = false;

        lookahead_.type_ = JsTokenType::EOF_;
        lookahead_.line_number_ = scanner_->LineNumber();
        lookahead_.line_start_ = 0;
        lookahead_.range_ = make_pair(0, 0);

        context_.is_module = false;
        context_.await = false;
        context_.allow_in = true;
        context_.allow_strict_directive = true;
        context_.allow_yield = true;
        context_.is_assignment_target = false;
        context_.is_binding_element = false;
        context_.in_function_body = false;
        context_.in_iteration = false;
        context_.in_switch = false;
        context_.strict_ = false;

        start_marker_ = Marker {
            0,
            scanner_->LineNumber(),
            0,
        };

        last_marker_ = Marker {
            0,
            scanner_->LineNumber(),
            0,
        };

        NextToken();

        last_marker_ = Marker {
            scanner_->Index(),
            scanner_->LineNumber(),
            scanner_->Column(),
        };
    }

    void ParserCommon::TolerateError(const std::string &message) {
        ParseError error;
        error.msg_ = message;
        error.index_ = last_marker_.index;
        error.line_ = last_marker_.line ;
        error.col_ = last_marker_.column + 1;
        error_handler_->TolerateError(error);
    }

    void ParserCommon::TolerateUnexpectedToken(const Token& tok) {
        UnexpectedToken(tok);
    }

    void ParserCommon::TolerateUnexpectedToken(const Token& tok, const std::string& message) {
        UnexpectedToken(tok, message);
    }

    ParseError ParserCommon::UnexpectedToken(const Token &token) {
        string msg = ParseMessages::UnexpectedToken;
        UString value;

        msg = (token.type_ == JsTokenType::EOF_) ? ParseMessages::UnexpectedEOS :
              (token.type_ == JsTokenType::Identifier) ? ParseMessages::UnexpectedIdentifier :
              (token.type_ == JsTokenType::NumericLiteral) ? ParseMessages::UnexpectedNumber :
              (token.type_ == JsTokenType::StringLiteral) ? ParseMessages::UnexpectedString :
              (token.type_ == JsTokenType::Template) ? ParseMessages::UnexpectedTemplate :
              ParseMessages::UnexpectedToken;

        if (IsKeywordToken(token.type_)) {
            if (Scanner::IsFutureReservedWord(token.type_)) {
                msg = ParseMessages::UnexpectedReserved;
            } else {
                msg = ParseMessages::UnexpectedToken;
            }
        }
        value = token.value_;

        return UnexpectedToken(token, msg);
    }

    ParseError ParserCommon::UnexpectedToken(const Token &token, const std::string& message) {
        if (token.line_number_ > 0) {
            uint32_t index = token.range_.first;
            uint32_t line = token.line_number_;
            uint32_t lastMarkerLineStart = last_marker_.index - last_marker_.column;
            uint32_t column = token.range_.first - lastMarkerLineStart + 1;
            return error_handler_->CreateError(message, index, line, column);
        } else {
            uint32_t index = token.range_.first;
            uint32_t line = token.line_number_;
            uint32_t column = last_marker_.column + 1;
            return error_handler_->CreateError(message, index, line, column);
        }
    }

    void ParserCommon::ThrowError(const std::string &message) {
        auto index = last_marker_.index;
        auto line = last_marker_.line;
        auto column = last_marker_.column + 1;
        throw error_handler_->CreateError(message, index, line, column);
    }

    void ParserCommon::ThrowError(const std::string &message, const std::string &arg) {
        ThrowError(message + " " + arg);
    }

    void ParserCommon::ThrowUnexpectedToken(const Token& tok) {
        throw UnexpectedToken(tok);
    }

    void ParserCommon::ThrowUnexpectedToken(const Token& tok, const std::string &message) {
        throw UnexpectedToken(tok, message);
    }

    void ParserCommon::DecorateToken(Token& token) {
        token.loc_.start_ = Position {
            start_marker_.line,
            start_marker_.column,
        };

        token.loc_.end_ = Position {
            scanner_->LineNumber(),
            scanner_->Column(),
        };
    }

    Token ParserCommon::NextToken() {
        Token token = lookahead_;

        last_marker_.index = scanner_->Index();
        last_marker_.line = scanner_->LineNumber();
        last_marker_.column = scanner_->Column();

        CollectComments();

        if (scanner_->Index() != start_marker_.index) {
            start_marker_.index = scanner_->Index();
            start_marker_.line = scanner_->LineNumber();
            start_marker_.column = scanner_->Index() - scanner_->LineStart();
        }

        Token next = scanner_->Lex();

        has_line_terminator_ = token.line_number_ != next.line_number_;

        if (context_.strict_ && next.type_ == JsTokenType::Identifier) {
            if (JsTokenType t = Scanner::IsStrictModeReservedWord(next.value_); t != JsTokenType::Invalid) {
                next.type_ = t;
            }
        }
        lookahead_ = next;

        if (config_.tokens && next.type_ != JsTokenType::EOF_) {
            DecorateToken(next);
            tokens_.push(next);
        }

        return token;
    }

    Token ParserCommon::NextRegexToken() {
        CollectComments();

        Token token = scanner_->ScanRegExp();
        if (config_.tokens) {
            tokens_.pop();
            tokens_.push(token);
        }

        lookahead_ = token;
        NextToken();

        return token;
    }

    UString ParserCommon::GetTokenRaw(const Token& token) {
        return scanner_->Source()->substr(token.range_.first, token.range_.second);
    }

    ParserCommon::Marker ParserCommon::CreateStartMarker() {
        return {
            start_marker_.index,
            start_marker_.line,
            start_marker_.column
        };
    }

    ParserCommon::Marker ParserCommon::StartNode(Token &tok, std::uint32_t last_line_start) {
        auto column = tok.range_.first - tok.line_start_;
        auto line = tok.line_number_;
        if (column < 0) {
            column += last_line_start;
            line--;
        }
        return {
            static_cast<uint32_t>(tok.range_.first),
            line,
            column
        };
    }

    void ParserCommon::ExpectCommaSeparator() {
        if (config_.tolerant) {
            Token token = lookahead_;
            if (token.type_ == JsTokenType::Comma) {
                NextToken();
            } else if (token.type_ == JsTokenType::Semicolon) {
                NextToken();
                ThrowUnexpectedToken(token);
            } else {
                ThrowUnexpectedToken(token);
            }
        } else {
            Expect(JsTokenType::Comma);
        }
    }

    void ParserCommon::Expect(JsTokenType t) {
        Token token = NextToken();

        if (token.type_ != t) {
            ThrowUnexpectedToken(token);
        }
    }

    bool ParserCommon::Match(JsTokenType t) {
        return lookahead_.type_ == t;
    }

    bool ParserCommon::MatchAssign() {
        switch (lookahead_.type_) {
            case JsTokenType::Assign:
            case JsTokenType::MulAssign:
            case JsTokenType::PowAssign:
            case JsTokenType::DivAssign:
            case JsTokenType::ModAssign:
            case JsTokenType::PlusAssign:
            case JsTokenType::MinusAssign:
            case JsTokenType::LeftShiftAssign:
            case JsTokenType::RightShiftAssign:
            case JsTokenType::ZeroFillRightShiftAssign:
            case JsTokenType::BitAndAssign:
            case JsTokenType::BitOrAssign:
            case JsTokenType::BitXorAssign:
                return true;

            default:
                return false;
        }
    }

    void ParserCommon::ConsumeSemicolon() {
        if (Match(JsTokenType::Semicolon)) {
            NextToken();
        } else if (!has_line_terminator_) {
            if (lookahead_.type_ != JsTokenType::EOF_ && !Match(JsTokenType::RightBracket)) {
                ThrowUnexpectedToken(lookahead_);
            }
            last_marker_.index = start_marker_.index;
            last_marker_.line = start_marker_.line;
            last_marker_.column = start_marker_.column;
        }
    }

    bool ParserCommon::MatchContextualKeyword(const UString& keyword) {
        return lookahead_.type_ == JsTokenType::Identifier && lookahead_.value_ == keyword;
    }

    void ParserCommon::CollectComments() {
        scanner_->ScanComments(comments_);
    }

    int ParserCommon::BinaryPrecedence(const Token& token) const {
        auto op = token.value_;
        switch (token.type_) {
            case JsTokenType::RightParen:
            case JsTokenType::Semicolon:
            case JsTokenType::Comma:
            case JsTokenType::Assign:
            case JsTokenType::RightBrace:
                return 0;

            case JsTokenType::Or:
                return 1;

            case JsTokenType::And:
                return 2;

            case JsTokenType::BitOr:
                return 3;

            case JsTokenType::Xor:
                return 4;

            case JsTokenType::BitAnd:
                return 5;

            case JsTokenType::Equal:
            case JsTokenType::NotEqual:
            case JsTokenType::StrictEqual:
            case JsTokenType::StrictNotEqual:
                return 6;

            case JsTokenType::LessThan:
            case JsTokenType::GreaterThan:
            case JsTokenType::LessEqual:
            case JsTokenType::GreaterEqual:
                return 7;

            case JsTokenType::RightShift:
            case JsTokenType::LeftShift:
            case JsTokenType::ZeroFillRightShift:
                return 8;

            case JsTokenType::Plus:
            case JsTokenType::Minus:
                return 9;

            case JsTokenType::Mul:
            case JsTokenType::Div:
            case JsTokenType::Mod:
                return 11;

            default:
                if (IsKeywordToken(token.type_)) {
                    if (token.type_ == JsTokenType::K_Instanceof|| (context_.allow_in && token.type_ == JsTokenType::K_In)) {
                        return 7;
                    } else {
                        return 0;
                    }
                }

        }
        return 0;
    }

    bool ParserCommon::IsIdentifierName(Token &token) {
        return token.type_ == JsTokenType::Identifier ||
               IsKeywordToken(token.type_) ||
               token.type_ == JsTokenType::BooleanLiteral ||
               token.type_ == JsTokenType::NullLiteral;
    }

}
