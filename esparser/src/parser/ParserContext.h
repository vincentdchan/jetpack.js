//
// Created by Duzhong Chen on 2019/10/30.
//
#pragma once
#include <optional>
#include <unordered_set>
#include <vector>
#include "Utils.h"
#include "tokenizer/Scanner.h"
#include "SyntaxNodes.h"

namespace jetpack::parser {

    class ParserContext final {
    public:
        struct Config {
        public:
            static Config Default();

            std::optional<UString> source;
            bool tokens;
            bool comment;
            bool tolerant;
            bool jsx;
            bool typescript;
            bool constant_folding;

            /**
             * transpile jsx when parsing
             */
            bool transpile_jsx;

        private:
            Config() = delete;

        };

        struct Marker {
            uint32_t index = 0;
            uint32_t line = 0;
            uint32_t column = 0;
        };

        ParserContext(int32_t fileId, const UString& src, const Config& config);
        ParserContext(const ParserContext& ps) = delete;
        ParserContext(ParserContext&& ps) = delete;

        ParserContext& operator=(const ParserContext& ps) = delete;
        ParserContext& operator=(ParserContext&& ps) = delete;

        Config                   config_;
        Token                    lookahead_;
        std::unique_ptr<Scanner> scanner_;

        Sp<ParseErrorHandler>    error_handler_;
        UString                  source_;
        bool                     has_line_terminator_;

        std::stack<Token>        tokens_;
        std::vector<Sp<Comment>> comments_;

        Marker start_marker_;
        Marker last_marker_;

        // context in esprima
        bool    is_module_              = false;
        bool    allow_in_               = false;
        bool    allow_strict_directive_ = false;
        bool    allow_yield_            = false;
        bool    await_                  = false;
        bool    is_assignment_target_   = false;
        bool    is_binding_element_     = false;
        bool    in_function_body_       = false;
        bool    in_iteration_           = false;
        bool    in_switch_              = false;
        bool    strict_                 = false;
        int32_t fileIndex;

        std::optional<Token> first_cover_initialized_name_error_;
        std::unique_ptr<std::unordered_set<UString>> label_set_;

    };

}
