//
// Created by Duzhong Chen on 2019/9/6.
//

#pragma once

#include <optional>
#include <memory>
#include <stack>
#include <functional>
#include "../tokenizer/token.h"
#include "../parse_error_handler.h"
#include "../utils.h"
#include "../macros.h"
#include "../tokenizer/scanner.h"
#include "syntax_nodes.h"

namespace parser {
    using namespace std;

    static const char* ArrowParameterPlaceHolder = "ArrowParameterPlaceHolder";

    class ParserCommon {
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
            vector<Sp<SyntaxNode>> params;
            bool stricted;
            bool first_restricted;
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

        bool NextToken(Token* token = nullptr);
        void LogError(const string& message);
        void UnexpectedToken(const Token* tok = nullptr, const string* message = nullptr);

        Marker CreateNode();
        Marker StartNode(Token& tok, uint32_t last_line_start = 0);

        bool Expect(char16_t t);
        bool Expect(const UString& str);
        bool ExpectCommaSeparator();
        bool ExpectKeyword(const UString& keyword);

        bool Match(char16_t t);
        bool Match(const UString& str);
        bool MatchKeyword(const UString& keyword);
        bool MatchContextualKeyword(const UString& keyword);
        bool MatchAssign();

        bool IsolateCoverGrammar(std::function<bool()> cb);
        bool InheritCoverGrammar(std::function<bool()> cb);

        bool ConsumeSemicolon();

        bool CollectComments();

    protected:
        Config config_;
        Context context_;
        Token lookahead_;
        unique_ptr<Scanner> scanner_;

        Marker start_marker_;
        Marker last_marker_;

        Sp<ParseErrorHandler> error_handler_;
        Sp<std::u16string> source_;
        bool has_line_terminator_;

        stack<Token> tokens_;

        vector<Comment> comments_;


    };

}
