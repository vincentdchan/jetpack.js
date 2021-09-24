//
// Created by Duzhong Chen on 2019/10/30.
//
#pragma once
#include <optional>
#include <unordered_set>
#include <vector>
#include "utils/Common.h"
#include "tokenizer/Scanner.h"
#include "parser/Config.h"
#include "parser/AstContext.h"
#include "SyntaxNodes.h"

namespace jetpack::parser {

    class ParserContext {
    public:
        struct Marker {
            Scanner::Cursor cursor;
            uint32_t line = 0;
            uint32_t column = 0;
        };

        ParserContext(AstContext& ast_ctx, std::string_view src, const Config& config);
        ParserContext(AstContext& ast_ctx, Sp<StringWithMapping> src, const Config& config);
        ParserContext(const ParserContext& ps) = delete;
        ParserContext(ParserContext&& ps) = delete;

        ParserContext& operator=(const ParserContext& ps) = delete;
        ParserContext& operator=(ParserContext&& ps) = delete;

        AstContext&              ast_context_;

        inline void SetFileIndex(int32_t file_index) {
            fileIndex = file_index;
        }

        Config                   config_;
        Token                    lookahead_;
        std::unique_ptr<Scanner> scanner_;

        Sp<ParseErrorHandler>    error_handler_;
        Sp<StringWithMapping>    source_;
        bool                     has_line_terminator_;

        std::stack<Token>        tokens_;
        std::vector<Sp<Comment>> comments_;

        Marker start_marker_;
        Marker last_marker_;

        // context in esprima
        bool    is_module_              = false;
        bool    is_common_js_           = false;
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
        int32_t fileIndex = -1;

        std::optional<Token> first_cover_initialized_name_error_;
        std::unique_ptr<HashSet<std::string>> label_set_;

    };

}
