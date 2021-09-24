//
// Created by Duzhong Chen on 2019/10/30.
//
#include <memory>
#include "ParserContext.h"

namespace jetpack::parser {
    using namespace std;

    ParserContext::ParserContext(AstContext& ast_ctx, std::string_view src, const Config& config):
        ParserContext(ast_ctx, std::make_shared<StringWithMapping>(std::make_unique<RawMemoryViewOwner>(src)), config) {
    }

    ParserContext::ParserContext(AstContext& ast_ctx, Sp<StringWithMapping> src, const Config &config):
        ast_context_(ast_ctx), config_(config) {
        error_handler_ = std::make_shared<ParseErrorHandler>();

        source_ = std::move(src);
        scanner_ = make_unique<Scanner>(source_, error_handler_);
        has_line_terminator_ = false;

        lookahead_.type = JsTokenType::EOF_;
        lookahead_.lineNumber = scanner_->LineNumber();
        lookahead_.lineStart = 0;
        lookahead_.range = make_pair(0, 0);

        is_module_ = false;
        await_ = false;
        allow_in_ = true;
        allow_strict_directive_ = true;
        allow_yield_ = true;
        is_assignment_target_ = false;
        is_binding_element_ = false;
        in_function_body_ = false;
        in_iteration_ = false;
        in_switch_ = false;
        strict_ = false;

        label_set_ = std::make_unique<HashSet<std::string>>();
    }

}
