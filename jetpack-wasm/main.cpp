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
#include "dumper/AstToJson.h"

#ifdef __EMSCRIPTEN__

using namespace jetpack;
using namespace jetpack::parser;

inline std::string ParseAndCodeGen(std::string&& content, const ParserContext::Config& config, const CodeGen::Config& code_gen_config) {
    auto ctx = std::make_shared<ParserContext>(-1, std::move(content), config);
    Parser parser(ctx);

    auto mod = parser.ParseModule();

    CodeGen codegen(code_gen_config, nullptr);
    codegen.Traverse(mod);
    return codegen.GetResult().content;
}

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
    std::string content(str);
    ParserContext::Config parser_config = ParserContext::Config::Default();
    CodeGen::Config code_gen_config;
    parser_config.jsx = !!(flags & JSX_FLAG);
    parser_config.constant_folding = !!(flags & CONSTANT_FOLDING_FLAG);
    code_gen_config.minify = !!(flags & MINIFY_FLAG);
    auto result = ParseAndCodeGen(std::move(content), parser_config, code_gen_config);
    // muse be freed in JS
    auto jp_result = reinterpret_cast<JpResult *>(::malloc(sizeof(JpResult) + result.size() + 1));
    jp_result->flags = 0;
    jp_result->content[result.size()] = 0;
    std::memcpy(jp_result->content, result.c_str(), result.size());
    return jp_result;
}

EMSCRIPTEN_KEEPALIVE
JpResult *parse_to_ast(const char *str) {
    std::string content(str);
    ParserContext::Config config = ParserContext::Config::Default();
    auto ctx = std::make_shared<ParserContext>(-1, std::move(content), config);
    Parser parser(ctx);

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
