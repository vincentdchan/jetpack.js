//
// Created by Duzhong Chen on 2019/9/6.
//

#pragma once

#include <optional>
#include <memory>
#include <stack>
#include <functional>
#include <unordered_set>
#include "../tokenizer/token.h"
#include "parse_error_handler.h"
#include "../utils.h"
#include "../macros.h"
#include "../tokenizer/scanner.h"
#include "syntax_nodes.h"

namespace parser {
    using namespace std;

    class ParserCommon {
    public:
        struct Config {
        public:
            static Config Default();

            optional<UString> source;
            bool tokens;
            bool comment;
            bool tolerant;

        private:
            Config() = default;
        };

        struct Context {
            bool is_module = false;
            bool allow_in = false;
            bool allow_strict_directive = false;
            bool allow_yield = false;
            bool await = false;
            optional<Token> first_cover_initialized_name_error;
            bool is_assignment_target = false;
            bool is_binding_element = false;
            bool in_function_body = false;
            bool in_iteration = false;
            bool in_switch = false;
            unique_ptr<unordered_set<UString>> label_set;
            bool strict_ = false;

            Context() {
                label_set = make_unique<unordered_set<UString>>();
            }
        };

        struct Marker {
            uint32_t index = 0;
            uint32_t line = 0;
            uint32_t column = 0;
        };

        struct FormalParameterOptions {
            bool simple = true;
            vector<Sp<SyntaxNode>> params;
            std::unordered_set<UString> param_set;
            optional<Token> stricted;
            optional<Token> first_restricted;
            string message;
        };


        ParserCommon(
            shared_ptr<u16string> source,
            const Config& config
        );
        ParserCommon(const ParserCommon&) = delete;
        ParserCommon(ParserCommon&&) = delete;

        ParserCommon& operator=(const ParserCommon&) = delete;
        ParserCommon& operator=(ParserCommon&&) = delete;

        void DecorateToken(Token& );

        Token NextToken();
        void TolerateError(const string& message);

        void ThrowUnexpectedToken(const Token& tok);
        void ThrowUnexpectedToken(const Token& tok, const string& message);
        void TolerateUnexpectedToken(const Token& tok);
        void TolerateUnexpectedToken(const Token& tok, const string& message);

        void ThrowError(const std::string& message);
        void ThrowError(const std::string& message, const std::string& arg);

        Marker CreateStartMarker();
        Marker StartNode(Token& tok, uint32_t last_line_start = 0);

        void Expect(char16_t t);
        void Expect(const UString& str);
        void ExpectCommaSeparator();
        void ExpectKeyword(JsTokenType t);

        bool Match(char16_t t);
        bool Match(const UString& str);
        bool MatchKeyword(JsTokenType t);
        bool MatchContextualKeyword(const UString& keyword);
        bool MatchAssign();

        void ConsumeSemicolon();

        void CollectComments();

        inline Sp<ParseErrorHandler> ErrorHandler() {
            return error_handler_;
        }

        [[nodiscard]] int BinaryPrecedence(const Token& token) const;

        static bool IsIdentifierName(Token&);

        [[nodiscard]] inline Marker LastMarker() const {
            return last_marker_;
        }

        [[nodiscard]] inline Marker StartMarker() const {
            return start_marker_;
        }

        inline void Assert(bool value, std::string message) {
            if (value) {
                throw ParseAssertFailed(std::move(message), last_marker_.line, last_marker_.column);
            }
        }

    protected:
        Config config_;
        Context context_;
        Token lookahead_;
        unique_ptr<Scanner> scanner_;

        Sp<ParseErrorHandler> error_handler_;
        Sp<UString> source_;
        bool has_line_terminator_;

        stack<Token> tokens_;

        vector<Comment> comments_;

    private:

        ParseError UnexpectedToken(const Token& tok);
        ParseError UnexpectedToken(const Token& tok, const string& message);

        Marker start_marker_;
        Marker last_marker_;

    };

}
