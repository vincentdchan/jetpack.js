//
// Created by Duzhong Chen on 2019/9/6.
//

#include "ParserCommon.h"
#include "ErrorMessage.h"
#include <fmt/format.h>

namespace rocket_bundle::parser {
    using namespace std;

    ParserCommon::ParserCommon(std::shared_ptr<ParserContext> state):
        ctx(std::move(state)) {

        ctx->start_marker_ = ParserContext::Marker {
            0,
            ctx->scanner_->LineNumber(),
            0,
        };

        ctx->last_marker_ = ParserContext::Marker {
            0,
            ctx->scanner_->LineNumber(),
            0,
        };

        NextToken();

        ctx->last_marker_ = ParserContext::Marker {
            ctx->scanner_->Index(),
            ctx->scanner_->LineNumber(),
            ctx->scanner_->Column(),
        };
    }

    void ParserCommon::TolerateError(const std::string &message) {
        ParseError error;
        error.msg_ = message;
        error.index_ = ctx->last_marker_.index;
        error.line_ = ctx->last_marker_.line ;
        error.col_ = ctx->last_marker_.column + 1;
        ctx->error_handler_->TolerateError(error);
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

        string final_message = fmt::format(msg, utils::To_UTF8(value));
        return UnexpectedToken(token, final_message);
    }

    ParseError ParserCommon::UnexpectedToken(const Token &token, const std::string& message) {
        if (token.line_number_ > 0) {
            uint32_t index = token.range_.first;
            uint32_t line = token.line_number_;
            uint32_t lastMarkerLineStart = ctx->last_marker_.index - ctx->last_marker_.column;
            uint32_t column = token.range_.first - lastMarkerLineStart + 1;
            return ctx->error_handler_->CreateError(message, index, line, column);
        } else {
            uint32_t index = token.range_.first;
            uint32_t line = token.line_number_;
            uint32_t column = ctx->last_marker_.column + 1;
            return ctx->error_handler_->CreateError(message, index, line, column);
        }
    }

    void ParserCommon::ThrowError(const std::string &message) {
        auto index = ctx->last_marker_.index;
        auto line = ctx->last_marker_.line;
        auto column = ctx->last_marker_.column + 1;
        throw ctx->error_handler_->CreateError(message, index, line, column);
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
            ctx->start_marker_.line,
            ctx->start_marker_.column,
        };

        token.loc_.end_ = Position {
            ctx->scanner_->LineNumber(),
            ctx->scanner_->Column(),
        };
    }

    Token ParserCommon::NextToken() {
        Token token = ctx->lookahead_;
        Scanner& scanner = *ctx->scanner_.get();

        ctx->last_marker_ = ParserContext::Marker {
            scanner.Index(),
            scanner.LineNumber(),
            scanner.Column(),
        };

        CollectComments();

        if (scanner.Index() != ctx->start_marker_.index) {
            ctx->start_marker_ = ParserContext::Marker {
                scanner.Index(),
                scanner.LineNumber(),
                scanner.Index() - scanner.LineStart(),
            };
        }

        Token next = scanner.Lex();

        ctx->has_line_terminator_ = token.line_number_ != next.line_number_;

        if (ctx->strict_ && next.type_ == JsTokenType::Identifier) {
            if (JsTokenType t = Scanner::IsStrictModeReservedWord(next.value_); t != JsTokenType::Invalid) {
                next.type_ = t;
            }
        }
        ctx->lookahead_ = next;

        if (ctx->config_.tokens && next.type_ != JsTokenType::EOF_) {
            DecorateToken(next);
            ctx->tokens_.push(next);
        }

        return token;
    }

    Token ParserCommon::NextRegexToken() {
        CollectComments();

        Token token = ctx->scanner_->ScanRegExp();
        if (ctx->config_.tokens) {
            ctx->tokens_.pop();
            ctx->tokens_.push(token);
        }

        ctx->lookahead_ = token;
        NextToken();

        return token;
    }

    ParserContext::Marker ParserCommon::StartNode(Token &tok, std::uint32_t last_line_start) {
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
        if (ctx->config_.tolerant) {
            Token token = ctx->lookahead_;
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

    bool ParserCommon::MatchAssign() {
        switch (ctx->lookahead_.type_) {
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
        } else if (!ctx->has_line_terminator_) {
            if (ctx->lookahead_.type_ != JsTokenType::EOF_ && !Match(JsTokenType::RightBracket)) {
                ThrowUnexpectedToken(ctx->lookahead_);
            }
            ctx->last_marker_ = ParserContext::Marker {
                ctx->start_marker_.index,
                ctx->start_marker_.line,
                ctx->start_marker_.column,
            };
        }
    }

    bool ParserCommon::MatchContextualKeyword(const UString& keyword) {
        Token& lookahead = ctx->lookahead_;
        return lookahead.type_ == JsTokenType::Identifier && lookahead.value_ == keyword;
    }

    void ParserCommon::CollectComments() {
        ctx->scanner_->ScanComments(ctx->comments_);
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
                    if (token.type_ == JsTokenType::K_Instanceof|| (ctx->allow_in_ && token.type_ == JsTokenType::K_In)) {
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
