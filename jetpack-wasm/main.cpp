//
// Created by Duzhong Chen on 2021/9/10.
//

#include <emscripten.h>
#include <cstdlib>
#include <cstring>
#include <memory>
#include "ModuleResolver.h"
#include "parser/ParserCommon.h"
#include "parser/Parser.hpp"
#include "parser/ParserContext.h"
#include "dumper/AstToJson.h"
#include "SimpleAPI.h"
#include "UniqueNameGenerator.h"

#ifdef __EMSCRIPTEN__

using namespace jetpack;

struct JpResult {
    uint32_t flags;
    char     content[];
};

#define JSX_FLAG 0x1
#define CONSTANT_FOLDING_FLAG 0x2
#define MINIFY_FLAG 0x100

extern "C" {

EMSCRIPTEN_KEEPALIVE
JpResult *parse_and_codegen(const char *str, uint32_t flags) {
    std::string result;
    JpResult* jp_result = nullptr;
    try {
        parser::Config parser_config = parser::Config::Default();
        CodeGenConfig code_gen_config;
        parser_config.jsx = !!(flags & JSX_FLAG);
        parser_config.constant_folding = !!(flags & CONSTANT_FOLDING_FLAG);
        code_gen_config.minify = !!(flags & MINIFY_FLAG);
        result = jetpack::simple_api::ParseAndCodeGen(str, parser_config, code_gen_config);
    } catch (jetpack::parser::ParseError& err) {
        std::string errMsg = err.ErrorMessage();
        jp_result = reinterpret_cast<JpResult *>(::malloc(sizeof(JpResult) + errMsg.size() + 1));
        jp_result->flags = 1;
        std::memcpy(jp_result->content, errMsg.c_str(), errMsg.size());
        jp_result->content[errMsg.size()] = 0;
        return jp_result;
    } catch (...) {
        std::string errMsg = "unknown error";
        jp_result = reinterpret_cast<JpResult *>(::malloc(sizeof(JpResult) + errMsg.size() + 1));
        jp_result->flags = 1;
        std::memcpy(jp_result->content, errMsg.c_str(), errMsg.size());
        jp_result->content[errMsg.size()] = 0;
        return jp_result;
    }
    // muse be freed in JS
    jp_result = reinterpret_cast<JpResult *>(::malloc(sizeof(JpResult) + result.size() + 1));
    jp_result->flags = 0;
    jp_result->content[result.size()] = 0;
    std::memcpy(jp_result->content, result.c_str(), result.size());
    return jp_result;
}

EMSCRIPTEN_KEEPALIVE
JpResult *parse_to_ast(const char *str) {
    parser::Config config = parser::Config::Default();
    auto ctx = std::make_shared<parser::ParserContext>(-1, str, config);
    parser::Parser parser(ctx);

    auto mod = parser.ParseModule();
    auto json = jetpack::dumper::AstToJson::Dump(mod);

    auto json_str = json.dump();

    // muse be freed in JS
    auto jp_result = reinterpret_cast<JpResult*>(::malloc(sizeof(JpResult) + json_str.size() + 1));
    jp_result->flags = 0;
    jp_result->content[json_str.size()] = 0;
    std::memcpy(jp_result->content, json_str.c_str(), json_str.size());
    return jp_result;
}

}
#endif
