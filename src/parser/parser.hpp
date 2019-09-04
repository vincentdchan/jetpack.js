//
// Created by Duzhong Chen on 2019/9/3.
//
#pragma once

#include <optional>
#include <memory>
#include <stack>
#include <functional>
#include "../gc.hpp"
#include "../tokenizer/token.h"
#include "../tokenizer/scanner.h"
#include "../parse_error_handler.h"
#include "../utils.h"
#include "../macros.h"
#include "../js_string.h"
#include "ast.h"

namespace parser {

    using namespace std;

    static const char* ArrowParameterPlaceHolder = "ArrowParameterPlaceHolder";

    class Parser final {
    public:
        struct Config {
            bool range;
            bool loc;
            optional<UString> source;
            bool tokens;
            bool comment;
            bool tolerant;
        };

        struct Context {
            bool is_module;
            bool allow_in;
            bool allow_strict_directive;
            bool allow_yield;
            bool await;
            optional<Token> first_cover_initialized_name_error;
            bool is_assignment_target;
            bool is_binding_element;
            bool in_function_body;
            bool in_iteration;
            bool in_switch;
            bool label_set;
            bool strict_;
        };

        struct Marker {
            uint32_t index = 0;
            uint32_t line = 0;
            uint32_t column = 0;
        };

        struct FormalParameterOptions {
            bool simple = true;
            vector<AstNode*> params;
            bool stricted;
            bool first_restricted;
            string message;
        };

    private:
        Config config_;
        Context context_;
        Token lookahead_;
        unique_ptr<Scanner> scanner_;

        Marker start_marker_;
        Marker last_marker_;

        shared_ptr<ParseErrorHandler> error_handler_;
        shared_ptr<std::u16string> source_;
        shared_ptr<GarbageCollector> gc_;
        bool has_line_terminator_;

        stack<Token> tokens_;

        vector<Comment> comments_;

    public:
        Parser(
            shared_ptr<u16string> source,
            const Config& config,
            shared_ptr<GarbageCollector> gc
        );
        Parser(const Parser&) = delete;
        Parser(Parser&&) = delete;

        Parser& operator=(const Parser&) = delete;
        Parser& operator=(Parser&&) = delete;

        void DecorateToken(Token& );

        bool NextToken(Token* token = nullptr);
        void LogError(const string& message);
        void UnexpectedToken(const Token* tok = nullptr, const string* message = nullptr);

        Marker CreateNode();
        Marker StartNode(Token& tok, uint32_t last_line_start = 0);

        template<typename FromT, typename ToT>
        bool Finalize(const Marker& marker, FromT from, ToT& to);

        bool Expect(char16_t t);
        bool Expect(const UString& str);
        bool ExpectCommaSeparator();
        bool ExpectKeyword(const UString& keyword);

        bool Match(char16_t t);
        bool Match(const UString& str);
        bool MatchKeyword(const UString& keyword);
        bool MatchAssign();

        bool IsolateCoverGrammar(std::function<bool()> cb);
        bool InheritCoverGrammar(std::function<bool()> cb);

        bool ConsumeSemicolon();

        template <typename NodePtr>
        bool ParsePrimaryExpression(NodePtr& expr);

        template <typename NodePtr>
        bool ParseSpreadElement(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseAssignmentExpression(NodePtr& ptr);

        template <typename NodePtr>
        bool ParsePropertyMethodFunction(NodePtr& ptr);

        template <typename NodePtr>
        bool ParsePropertyMethodAsyncFunction(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseObjectPropertyKey(NodePtr& ptr);

        template <typename NodePtr>
        bool ParseObjectProperty(bool& has_proto, NodePtr& ptr);

        template <typename NodePtr>
        bool ParseArrayInitializer(NodePtr& expr);

        bool ParseFormalParameters(bool first_restricted, FormalParameterOptions& option);
        bool ParseFormalParameter(FormalParameterOptions& option);

        template <typename NodePtr>
        bool ParseRestElement(std::vector<AstNode*>& params, NodePtr& ptr);

        template <typename NodePtr>
        bool ParsePattern(std::vector<AstNode*>& params, VarKind  kind, NodePtr& ptr);

        bool ParseFunctionExpression(Expression*& expr);
        bool ParseTemplateLiteral(Expression*& expr);
        bool ParseGroupExpression(Expression*& expr);


        bool ParseObjectInitializer(Expression*& expr);
        bool ParseIdentifierName(Expression*& expr);
        bool ParseClassExpression(Expression*& expr);
        bool MatchImportCall();
        bool ParseImportCall(Expression*& expr);

        bool MatchAsyncFunction();

        bool CollectComments();

        ~Parser() = default;

    };

    Parser::Parser(shared_ptr <std::u16string> source, const Parser::Config& config, std::shared_ptr<GarbageCollector> gc):
        source_(source), config_(config), gc_(gc) {

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

    void Parser::LogError(const std::string &message) {
        auto index = last_marker_.index;
        auto line = last_marker_.line;
        auto column = last_marker_.column + 1;
        error_handler_->CreateError(message, index, line, column);
    }

    void Parser::UnexpectedToken(const Token *token, const std::string *message) {
        string msg = "UnexpetedToken";
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
    }



//    template<typename FromT, typename ToT>
//    bool Finalize(const Marker& marker, FromT from, ToT& to);
    template<typename FromT, typename ToT>
    bool Parser::Finalize(const Parser::Marker &marker, FromT from, ToT& to) {
        static_assert(std::is_convertible<FromT, ToT>::value, "FromT can not convert to ToT");

        if (config_.range) {
            from->range_ = std::make_pair(marker.index, last_marker_.index);
        }

        if (config_.loc) {
            from->loc_.start_ = Position {
                marker.line,
                marker.column,
            };
            from->loc_.end_ = Position {
                last_marker_.line,
                last_marker_.column,
            };
        }

        to = from;
        return true;
    }

    bool Parser::CollectComments() {
        DO(scanner_->ScanComments(comments_))

        return true;
    }

    void Parser::DecorateToken(Token& token) {
        token.loc_.start_ = Position {
            start_marker_.line,
            start_marker_.column,
        };

        token.loc_.end_ = Position {
            scanner_->LineNumber(),
            scanner_->Column(),
        };
    }

    bool Parser::NextToken(Token *result) {
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

    Parser::Marker Parser::CreateNode() {
        return Marker {
            start_marker_.index,
            start_marker_.line,
            start_marker_.column
        };
    }

    Parser::Marker Parser::StartNode(Token &tok, std::uint32_t last_line_start) {
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

    bool Parser::Expect(char16_t t) {
        Token token;
        DO(NextToken(&token))
        if (token.type_ != JsTokenType::Punctuator || token.value_.size() != 1 || token.value_[0] != t) {
            UnexpectedToken(&token);
            return false;
        }
        return true;
    }

    bool Parser::Expect(const UString &keyword) {
        Token token;
        DO(NextToken(&token))
        if (token.type_ != JsTokenType::Punctuator || token.value_.size() != 1 || token.value_ != keyword) {
            UnexpectedToken(&token);
            return false;
        }
        return true;
    }

    bool Parser::ExpectCommaSeparator() {
        return Expect(',');
    }

    bool Parser::ExpectKeyword(const UString &keyword) {
        Token token;
        DO(NextToken(&token))

        if (token.type_ != JsTokenType::Keyword || token.value_ != keyword) {
            UnexpectedToken(&token);
            return false;
        }

        return true;
    }

    bool Parser::Match(char16_t t) {
        Token token;
        DO(NextToken(&token))
        return token.type_ == JsTokenType::Punctuator && token.value_.size() == 1 && token.value_[0] == t;
    }

    bool Parser::Match(const UString& t) {
        Token token;
        DO(NextToken(&token))
        return token.type_ == JsTokenType::Punctuator && token.value_.size() == 1 && token.value_ == t;
    }

    bool Parser::MatchKeyword(const UString &keyword) {
        Token token;
        DO(NextToken(&token))

        return token.type_ == JsTokenType::Keyword && token.value_ == keyword;
    }

    bool Parser::MatchAssign() {
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

    bool Parser::IsolateCoverGrammar(std::function<bool()> cb) {
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

    bool Parser::InheritCoverGrammar(std::function<bool()> cb) {
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

    bool Parser::ConsumeSemicolon() {
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

    template <typename NodePtr>
    bool Parser::ParsePrimaryExpression(NodePtr& expr) {
        static_assert(std::is_convertible<Expression*, NodePtr>::value, "NodePtr can not convert to AstNode*");

        expr = nullptr;
        auto marker = CreateNode();
        Token token;

        switch (lookahead_.type_) {
            case JsTokenType::Identifier: {
                if ((context_.is_module || context_.await) && lookahead_.value_ == U("await")) {
                    UnexpectedToken(&lookahead_);
                }
                if (MatchAsyncFunction()) {
                    return ParseFunctionExpression(expr);
                } else {
                    auto node = gc_->Alloc<Identifier>();
                    Token next;
                    DO(NextToken(&next))
                    node->name_ = StringContext::MakeString(next.value_);
                    return Finalize(marker, node, expr);
                }
            }

            case JsTokenType::NumericLiteral:
            case JsTokenType::StringLiteral: {
                if (context_.strict_ && lookahead_.octal_) {
                    UnexpectedToken(&lookahead_);
//                this.tolerateUnexpectedToken(this.lookahead, Messages.StrictOctalLiteral);
                }
                context_.is_assignment_target = false;
                context_.is_binding_element = false;
                DO(NextToken(&token))
//            raw = this.getTokenRaw(token);
                auto node = gc_->Alloc<Literal>();
                node->value_ = StringContext::MakeString(token.value_);
                Finalize(marker, node, expr);
                break;
            }

            case JsTokenType::BooleanLiteral: {
                context_.is_assignment_target = false;
                context_.is_binding_element = false;
                DO(NextToken(&token))
//            raw = this.getTokenRaw(token);
                auto node = gc_->Alloc<Literal>();
                node->value_ = StringContext::MakeString(token.value_);
                Finalize(marker, node, expr);
                break;
            }

            case JsTokenType::NullLiteral: {
                context_.is_assignment_target = false;
                context_.is_binding_element = false;
                DO(NextToken(&token))
//            raw = this.getTokenRaw(token);
                auto node = gc_->Alloc<Literal>();
                node->value_ = StringContext::MakeString(token.value_);
                Finalize(marker, node, expr);
                break;
            }

            case JsTokenType::Template:
                return ParseTemplateLiteral(expr);

            case JsTokenType::Punctuator: {
                switch (lookahead_.value_[0]) {
                    case '(':
                        context_.is_binding_element = false;
                        return InheritCoverGrammar([this, &expr]() {
                            return ParseGroupExpression(expr);
                        });

                    case '[':
                        return InheritCoverGrammar([this, &expr]() {
                            return ParseArrayInitializer(expr);
                        });

                    case '{':
                        return InheritCoverGrammar([this, &expr]() {
                            return ParseObjectInitializer(expr);
                        });

                    case '/':
//                case '/=': TODO:
//                    context_.is_assignment_target = false;
//                    context_.is_binding_element = false;
//                    scanner_->SetIndex(start_marker_.index);
//                    token = this.nextRegexToken();
//                    raw = this.getTokenRaw(token);
//                    expr = this.finalize(node, new Node.RegexLiteral(token.regex
//                    as
//                    RegExp, raw, token.pattern, token.flags));
                        return false;

                    default: {
                        DO(NextToken(&token))
                        UnexpectedToken(&token);
                        return false;
                    }

                }
                break;
            }

            case JsTokenType::Keyword:
                if (!context_.strict_ && context_.allow_yield && MatchKeyword(U("yield"))) {
                    return ParseIdentifierName(expr);
                } else if (!context_.strict_ && MatchKeyword(U("let"))) {
                    DO(NextToken(&token));
                    auto id = gc_->Alloc<Identifier>();
                    id->name_ = StringContext::MakeString(token.value_);
                    return Finalize(marker, id, expr);
                } else {
                    context_.is_assignment_target = false;
                    context_.is_binding_element = false;
                    if (MatchKeyword(U("function"))) {
                        ParseFunctionExpression(expr);
                    } else if (MatchKeyword(U("this"))) {
                        DO(NextToken(&token))
                        auto th = gc_->Alloc<ThisExpression>();
                        return Finalize(marker, th, expr);
                    } else if (MatchKeyword(U("class"))) {
                        return ParseClassExpression(expr);
                    } else if (MatchImportCall()) {
                        return ParseImportCall(expr);
                    } else {
                        DO(NextToken(&token));
                        UnexpectedToken(&token);
                        return false;
                    }
                }
                break;

            default:
                DO(NextToken(&token))
                UnexpectedToken(&token);
                return false;

        }

        return true;
    }


    template <typename NodePtr>
    bool Parser::ParseSpreadElement(NodePtr& expr) {
        static_assert(std::is_convertible<SpreadElement*, NodePtr>::value, "NodePtr can not accept SpreadElement*");
        auto marker = CreateNode();

        DO(Expect(U("...")))

        auto node = gc_->Alloc<SpreadElement>();

        DO(InheritCoverGrammar([this, &node]() {
            return ParseAssignmentExpression(node->argument_);
        }));

        return Finalize(marker, node, expr);
    }

    template <typename NodePtr>
    bool Parser::ParseArrayInitializer(NodePtr &expr) {
        static_assert(std::is_convertible<ArrayExpression*, NodePtr>::value, "NodePtr can not accept ArrayExpression*");
        auto marker = CreateNode();
        auto node = gc_->Alloc<ArrayExpression>();
        Expression* element = nullptr;

        DO(Expect('['))
        while (!Match(']')) {
            if (Match(',')) {
                NextToken();
                node->elements_.push_back(nullptr);
            } else if (Match(U("..."))) {
                DO(ParseSpreadElement(element))
                if (!Match(']')) {
                    context_.is_assignment_target = false;
                    context_.is_binding_element = false;
                    Expect(',');
                }
                node->elements_.push_back(element);
            } else {
                DO(InheritCoverGrammar([this, &element]() {
                    return ParseAssignmentExpression(element);
                }))
                node->elements_.push_back(element);
                if (!Match(']')) {
                    DO(Expect(','))
                }
            }
        }
        DO(Expect(']'))

        return Finalize(marker, node, expr);
    }

    template <typename NodePtr>
    bool Parser::ParsePropertyMethodFunction(NodePtr &ptr) {
        static_assert(std::is_convertible<FunctionExpression*, NodePtr>::value, "NodePtr can not accept FunctionExpression*");
        auto marker = CreateNode();
        auto node = gc_->Alloc<FunctionExpression>();

        bool isGenerator = false;

        FormalParameterOptions options;
        bool previous_allow_yield = context_.allow_yield;
        context_.allow_yield = true;
        DO(ParseFormalParameters(false, options))
//        const params = this.parseFormalParameters();
//        const method = this.parsePropertyMethod(params);
        context_.allow_yield = previous_allow_yield;

        return Finalize(marker, node, ptr);
    }

    template <typename NodePtr>
    bool Parser::ParsePropertyMethodAsyncFunction(NodePtr& ptr) {
        static_assert(std::is_convertible<AsyncFunctionExpression*, NodePtr>::value, "NodePtr can not accept FunctionExpression*");
        auto marker = CreateNode();
        auto node = gc_->Alloc<AsyncFunctionExpression>();

        bool isGenerator = false;

        FormalParameterOptions options;
        bool previous_allow_yield = context_.allow_yield;
        bool previous_await = context_.await;
        context_.allow_yield = true;
        DO(ParseFormalParameters(false, options))
//        const params = this.parseFormalParameters();
//        const method = this.parsePropertyMethod(params);
        context_.allow_yield = previous_allow_yield;
        context_.await = previous_await;

        return Finalize(marker, node, ptr);
    }

    template <typename NodePtr>
    bool Parser::ParseObjectPropertyKey(NodePtr &ptr) {
        auto marker = CreateNode();
        Token token;
        DO(NextToken(&token))

        switch (token.type_) {
            case JsTokenType::StringLiteral:
            case JsTokenType::NumericLiteral: {
                if (context_.strict_ && token.octal_) {
                    LogError("StrictOctalLiteral");
                }
                auto node = gc_->Alloc<Literal>();
                node->value_ = StringContext::MakeString(token.value_);
                return Finalize(marker, node, ptr);
            }

            case JsTokenType::Identifier:
            case JsTokenType::BooleanLiteral:
            case JsTokenType::NullLiteral:
            case JsTokenType::Keyword: {
                auto node = gc_->Alloc<Identifier>();
                node->name_ = StringContext::MakeString(token.value_);
                return Finalize(marker, node, ptr);
            }

            case JsTokenType::Punctuator:
                if (token.value_ == U("[")) {
                    DO(IsolateCoverGrammar([this, &ptr]() {
                        return ParseAssignmentExpression(ptr);
                    }));
                    DO(Expect(']'))
                } else {
                    UnexpectedToken(&token);
                    return false;
                }
                break;

            default:
                UnexpectedToken(&token);
                return false;
        }

        return true;
    }

    template <typename NodePtr>
    bool Parser::ParseObjectProperty(bool &has_proto, NodePtr &ptr) {
        auto marker = CreateNode();
        Token token = lookahead_;
        VarKind kind;
        bool computed = false;
        bool method = false;
        bool shorthand = false;
        bool is_async = false;

        AstNode* key = nullptr;

        if (token.type_ == JsTokenType::Identifier) {
            auto id = token.value_;
            DO(NextToken())
            computed = Match('[');
            is_async = !has_line_terminator_ && (id == U("async")) &&
                      !Match(':') && !Match('(') && !Match('*') && !Match(',');
            if (is_async) {
                DO(ParseObjectPropertyKey(key))
            } else {
                auto node = gc_->Alloc<Identifier>();
                node->name_ = StringContext::MakeString(id);
                DO(Finalize(marker, node, key));
            }
        } else if (Match('*')) {
            DO(NextToken())
        } else {
            computed = Match('[');
            DO(ParseObjectPropertyKey(key))
        }


        return true;
    }

    bool Parser::ParseFormalParameters(bool first_restricted, parser::Parser::FormalParameterOptions &options) {

        options.simple = true;
        options.params.clear();
        options.first_restricted = first_restricted;

        DO(Expect('('))
        if (!Match(')')) {
            while (lookahead_.type_ != JsTokenType::EOF_) {
                DO(ParseFormalParameter(options))
                if (Match(')')) {
                    break;
                }
                DO(Expect(','))
                if (Match(')')) {
                    break;
                }
            }
        }
        DO(Expect(')'))

        return true;
    }

    bool Parser::ParseFormalParameter(parser::Parser::FormalParameterOptions &option) {
        if (Match(U("..."))) {

        }

        return true;
    }

    template <typename NodePtr>
    bool Parser::ParseRestElement(std::vector<AstNode*>& params, NodePtr &ptr) {
        auto marker = CreateNode();
        auto node = gc_->Alloc<RestElement>();

        DO(Expect(U("...")))
        DO(ParsePattern(params, VarKind::Invalid, node->argument_))
        if (Match('=')) {
            LogError("DefaultRestParameter");
            return false;
        }
        if (!Match(')')) {
            LogError("ParameterAfterRestParameter");
            return false;
        }

        return Finalize(marker, node, ptr);
    }

}
