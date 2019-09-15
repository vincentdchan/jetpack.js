//
// Created by Duzhong Chen on 2019/9/6.
//

#include "parser_common.h"

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

    void ParserCommon::LogError(const std::string &message) {
        auto index = last_marker_.index;
        auto line = last_marker_.line;
        auto column = last_marker_.column + 1;
        error_handler_->CreateError(message, index, line, column);
    }

    void ParserCommon::UnexpectedToken(const Token *token, const std::string *message) {
        string msg = "UnexpectedToken";
        if (message) {
            msg = *message;
        }

        UString value;
        if (token) {
            if (!message) {
                msg = (token->type_ == JsTokenType::EOF_) ? "UnexpectedEOS" :
                      (token->type_ == JsTokenType::Identifier) ? "UnexpectedIdentifier" :
                      (token->type_ == JsTokenType::NumericLiteral) ? "UnexpectedNumber" :
                      (token->type_ == JsTokenType::StringLiteral) ? "UnexpectedString" :
                      (token->type_ == JsTokenType::Template) ? "UnexpectedTemplate" :
                      "UnexpectedToken";

                if (token->type_ == JsTokenType::Keyword) {
                    if (Scanner::IsFutureReservedWord(token->value_)) {
                        msg = "UnexpectedReserved";
                    } else if (context_.strict_ && Scanner::IsStrictModeReservedWord(token->value_)) {
                        msg = "StrictReservedWord";
                    }
                }
            }

            value = token->value_;
        } else {
            value = utils::To_UTF16("ILLEGAL");
        }

        LogError(msg);
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

    bool ParserCommon::NextToken(Token *result) {
        Token token = lookahead_;

        last_marker_.index = scanner_->Index();
        last_marker_.line = scanner_->LineNumber();
        last_marker_.column = scanner_->Column();

        DO(CollectComments())

        if (scanner_->Index() != start_marker_.index) {
            start_marker_.index = scanner_->Index();
            start_marker_.line = scanner_->LineNumber();
            start_marker_.column = scanner_->Index() - scanner_->LineStart();
        }

        Token next;
        DO(scanner_->Lex(next))

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

        return true;
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

    bool ParserCommon::Expect(char16_t t) {
        Token token;
        DO(NextToken(&token))
        if (token.type_ != JsTokenType::Punctuator || token.value_.size() != 1 || token.value_[0] != t) {
            UnexpectedToken(&token);
            return false;
        }
        return true;
    }

    bool ParserCommon::Expect(const UString &keyword) {
        Token token;
        DO(NextToken(&token))
        if (token.type_ != JsTokenType::Punctuator || token.value_.size() != 1 || token.value_ != keyword) {
            UnexpectedToken(&token);
            return false;
        }
        return true;
    }

    bool ParserCommon::ExpectCommaSeparator() {
        return Expect(',');
    }

    bool ParserCommon::ExpectKeyword(const UString &keyword) {
        Token token;
        DO(NextToken(&token))

        if (token.type_ != JsTokenType::Keyword || token.value_ != keyword) {
            UnexpectedToken(&token);
            return false;
        }

        return true;
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
        return op == U("=") ||
               op == U("*=") ||
               op == U("**=") ||
               op == U("/=") ||
               op == U("%=") ||
               op == U("+=") ||
               op == U("-=") ||
               op == U("<<=") ||
               op == U(">>=") ||
               op == U(">>>=") ||
               op == U("&=") ||
               op == U("^=") ||
               op == U("|=");
    }

    bool ParserCommon::IsolateCoverGrammar(std::function<bool()> cb) {
        auto previousIsBindingElement = context_.is_binding_element;
        auto previousIsAssignmentTarget = context_.is_assignment_target;
        auto previousFirstCoverInitializedNameError = context_.first_cover_initialized_name_error;

        context_.is_binding_element = true;
        context_.is_assignment_target = true;
        context_.first_cover_initialized_name_error.reset();

        bool result = cb();
        if (context_.first_cover_initialized_name_error) {
            LogError("firstCoverInitializedNameError");
            return false;
        }

        context_.is_binding_element = previousIsBindingElement;
        context_.is_assignment_target = previousIsAssignmentTarget;
        context_.first_cover_initialized_name_error = previousFirstCoverInitializedNameError;

        return result;
    }

    bool ParserCommon::InheritCoverGrammar(std::function<bool()> cb) {
        auto previousIsBindingElement = context_.is_binding_element;
        auto previousIsAssignmentTarget = context_.is_assignment_target;
        auto previousFirstCoverInitializedNameError = context_.first_cover_initialized_name_error;

        context_.is_binding_element = true;
        context_.is_assignment_target = true;
        context_.first_cover_initialized_name_error.reset();

        bool result = cb();

        context_.is_binding_element &= previousIsBindingElement;
        context_.is_assignment_target &= previousIsAssignmentTarget;
        context_.first_cover_initialized_name_error =
            previousFirstCoverInitializedNameError ?
            previousFirstCoverInitializedNameError : context_.first_cover_initialized_name_error;

        return result;
    }

    bool ParserCommon::ConsumeSemicolon() {
        if (Match(';')) {
            return NextToken();
        } else if (!has_line_terminator_) {
            if (lookahead_.type_ != JsTokenType::EOF_ && !Match('}')) {
                UnexpectedToken(&lookahead_);
                return false;
            }
            last_marker_.index = start_marker_.index;
            last_marker_.line = start_marker_.line;
            last_marker_.column = start_marker_.column;
        }
        return true;
    }

    bool ParserCommon::MatchContextualKeyword(const UString& keyword) {
        return lookahead_.type_ == JsTokenType::Identifier && lookahead_.value_ == keyword;
    }

    bool ParserCommon::CollectComments() {
        DO(scanner_->ScanComments(comments_))

        return true;
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

}
