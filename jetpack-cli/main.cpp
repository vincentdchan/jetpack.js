//
// Created by Duzhong Chen on 2020/3/20.
//

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <cstdlib>
#include <cstring>
#include <memory>
#include "ModuleResolver.h"
#include "parser/ParserCommon.h"
#include "parser/Parser.hpp"
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

//#ifndef _WIN32
//#include <jemalloc/jemalloc.h>
//#endif

#include <iostream>

#include "SimpleAPI.h"

#ifndef __EMSCRIPTEN__

int main(int argc, char** argv) {
    return jetpack::simple_api::HandleCommandLine(argc, argv);
}

/**
 * for WASM
 */
#else

using namespace jetpack;
using namespace jetpack::parser;

inline std::string ParseAndCodeGen(std::string&& content) {
    ParserContext::Config config = ParserContext::Config::Default();
    auto ctx = std::make_shared<ParserContext>(-1, std::move(content), config);
    Parser parser(ctx);

    auto mod = parser.ParseModule();

    CodeGen::Config code_gen_config;
    CodeGen codegen(code_gen_config, nullptr);
    codegen.Traverse(mod);
    return codegen.GetResult().content;
}

extern "C" {

EMSCRIPTEN_KEEPALIVE
int ParseAndCodeGen(const char* str, char* buffer, uint32_t buffer_size) {
    std::string content(str);
    auto result = ParseAndCodeGen(std::move(content));
    if (buffer == nullptr) {
        return result.size();
    }
    std::memcpy(buffer, result.c_str(), std::min<size_t>(result.size(), buffer_size));
    return 0;
}

}

#endif
