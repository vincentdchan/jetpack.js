//
// Created by Duzhong Chen on 2019/9/6.
//

#include "parser_common.h"
#include "error_message.h"

namespace parser {

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

        if (token.type_ == JsTokenType::Keyword) {
            if (Scanner::IsFutureReservedWord(token.value_)) {
                msg = ParseMessages::UnexpectedReserved;
            } else if (context_.strict_ && Scanner::IsStrictModeReservedWord(token.value_)) {
                msg = ParseMessages::StrictReservedWord;
            }
        }
        value = token.value_;

        return UnexpectedToken(token, msg);
    }

    ParseError ParserCommon::UnexpectedToken(const Token &token, const std::string& message) {
        if (token.line_number_ > 0) {
            uint32_t index = token.line_start_;
            uint32_t line = token.line_number_;
            uint32_t lastMarkerLineStart = last_marker_.index - last_marker_.column;
            uint32_t column = token.line_start_ - lastMarkerLineStart + 1;
            return error_handler_->CreateError(message, index, line, column);
        } else {
            uint32_t index = token.line_start_;
            uint32_t line = token.line_number_;
            uint32_t column = last_marker_.column + 1;
            return error_handler_->CreateError(message, index, line, column);
        }
    }

    void ParserCommon::ThrowError(const std::string &message) {
        auto index = last_marker_.index;
        auto line = last_marker_.line;
        auto column = last_marker_.column + 1;
        ParseError err = error_handler_->CreateError(message, index, line, column);
        throw err;
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

    void ParserCommon::NextToken(Token *result) {
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

        Token next;
        scanner_->Lex(next);

        has_line_terminator_ = token.line_number_ != next.line_number_;

        if (context_.strict_ && next.type_ == JsTokenType::Identifier) {
            if (Scanner::IsStrictModeReservedWord(next.value_)) {
                next.type_ = JsTokenType::Keyword;
            }
        }
        lookahead_ = next;

        if (config_.tokens && next.type_ != JsTokenType::EOF_) {
            DecorateToken(next);
            tokens_.push(next);
        }

        if (result) {
            *result = token;
        }
    }

    ParserCommon::Marker ParserCommon::CreateNode() {
        return Marker {
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
        return Marker {
            static_cast<uint32_t>(tok.range_.first),
            line,
            column
        };
    }

    void ParserCommon::Expect(char16_t t) {
        Token token;
        NextToken(&token);
        if (token.type_ != JsTokenType::Punctuator || token.value_.size() != 1 || token.value_[0] != t) {
            ThrowUnexpectedToken(token);
        }
    }

     void ParserCommon::Expect(const UString &keyword) {
        Token token;
        NextToken(&token);
        if (token.type_ != JsTokenType::Punctuator || token.value_.size() != 1 || token.value_ != keyword) {
            ThrowUnexpectedToken(token);
        }
    }

    void ParserCommon::ExpectCommaSeparator() {
        Expect(',');
    }

    void ParserCommon::ExpectKeyword(const UString &keyword) {
        Token token;
        NextToken(&token);

        if (token.type_ != JsTokenType::Keyword || token.value_ != keyword) {
            ThrowUnexpectedToken(token);
        }
    }

    bool ParserCommon::Match(char16_t t) {
        return lookahead_.type_ == JsTokenType::Punctuator && lookahead_.value_.size() == 1 && lookahead_.value_[0] == t;
    }

    bool ParserCommon::Match(const UString& t) {
        return lookahead_.type_ == JsTokenType::Punctuator && lookahead_.value_.size() == 1 && lookahead_.value_ == t;
    }

    bool ParserCommon::MatchKeyword(const UString &keyword) {
        return lookahead_.type_ == JsTokenType::Keyword && lookahead_.value_ == keyword;
    }

    bool ParserCommon::MatchAssign() {
        if (lookahead_.type_ != JsTokenType::Punctuator) {
            return false;
        }
        auto& op = lookahead_.value_;
        return op == u"=" ||
               op == u"*=" ||
               op == u"**=" ||
               op == u"/=" ||
               op == u"%=" ||
               op == u"+=" ||
               op == u"-=" ||
               op == u"<<=" ||
               op == u">>=" ||
               op == u">>>=" ||
               op == u"&=" ||
               op == u"^=" ||
               op == u"|=";
    }

    void ParserCommon::ConsumeSemicolon() {
        if (Match(';')) {
            NextToken();
        } else if (!has_line_terminator_) {
            if (lookahead_.type_ != JsTokenType::EOF_ && !Match('}')) {
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
        int precedence = 0;
        if (token.type_ == JsTokenType::Punctuator) {
            if (
                op == u")" || op == u";" || op == u"," ||
                op == u"=" || op == u"]"
                ) {
                precedence = 0;
            } else if (op == u"||") {
                precedence = 1;
            } else if (op == u"&&") {
                precedence = 2;
            } else if (op == u"|") {
                precedence = 3;
            } else if (op == u"^") {
                precedence = 4;
            } else if (op == u"&") {
                precedence = 5;
            } else if (
                op == u"==" || op == u"!=" || op == u"===" ||
                op == u"!=="
                ) {
                precedence = 6;
            } else if (
                op == u"<" || op == u">" || op == u"<=" ||
                op == u">=="
                ) {
                precedence = 7;
            } else if (op == u"<<" || op == u">>" || op == u">>>") {
                precedence = 8;
            } else if (op == u"+" || op == u"-") {
                precedence = 9;
            } else if (
                op == u"*" || op == u"/" || op == u"%"
                ) {
                precedence = 11;
            }
        } else if (token.type_ == JsTokenType::Keyword) {
            if (op == u"instanceof" || (context_.allow_in && op == u"in")) {
                precedence = 7;
            } else {
                precedence = 0;
            }
        }
        return precedence;
    }

    bool ParserCommon::IsIdentifierName(Token &token) {
        return token.type_ == JsTokenType::Identifier ||
               token.type_ == JsTokenType::Keyword ||
               token.type_ == JsTokenType::BooleanLiteral ||
               token.type_ == JsTokenType::NullLiteral;
    }

}
