//
// Created by Duzhong Chen on 2019/10/30.
//
#include "ParserContext.h"

namespace jetpack::parser {
    using namespace std;

    ParserContext::ParserContext(int32_t fileId, std::string&& src, const Config& config):
        ParserContext(fileId, StringWithMapping::Make(std::move(src)), config) {
    }

    ParserContext::ParserContext(int32_t fileId, Sp<StringWithMapping> src, const Config &config):
        fileIndex(fileId), config_(config){
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
