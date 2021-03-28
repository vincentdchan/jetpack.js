//
// Created by Duzhong Chen on 2020/7/14.
//

#include <gtest/gtest.h>
#include <parser/Parser.hpp>
#include <parser/ParserContext.h>
#include "SourceMapGenerator.h"
#include "ModuleResolver.h"
#include "codegen/CodeGen.h"

using namespace jetpack;
using namespace jetpack::parser;

inline std::string ParseAndGenSourceMap(const UString& content) {
    auto resolver = std::make_shared<ModuleResolver>();
    ParserContext::Config config = ParserContext::Config::Default();
    resolver->BeginFromEntryString(config, content);

    auto sourceMapGenerator = std::make_shared<SourceMapGenerator>(resolver, "");

    auto mod =  resolver->GetEntryModule();
    MemoryOutputStream ss;
    CodeGen::Config code_gen_config;
    CodeGen codegen(code_gen_config, sourceMapGenerator, ss);
    codegen.Traverse(mod->ast);

    return sourceMapGenerator->ToPrettyString();
}

TEST(SourceMap, VLQEncoding) {
    std::string str;
    SourceMapGenerator::IntToVLQ(str, 16);
    EXPECT_STREQ(str.c_str(), "gB");
}

TEST(SourceMap, Simple) {
    UString src(u""
                "function main() {\n"
                "    console.log('hello world');\n"
                "}\n"
    );
    auto result = ParseAndGenSourceMap(src);
    std::cout << result;
}
