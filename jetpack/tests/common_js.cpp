//
// Created by Duzhong Chen on 2021/4/8.
//

#include <gtest/gtest.h>
#include <parser/Parser.hpp>
#include <parser/ParserContext.h>
#include <iostream>

#include "ModuleResolver.h"
#include "codegen/CodeGen.h"

using namespace jetpack;
using namespace jetpack::parser;

inline std::string ParseAndCodeGen(UString content) {
    ParserContext::Config config = ParserContext::Config::Default();
    config.jsx = true;
    config.transpile_jsx = true;
    auto ctx = std::make_shared<ParserContext>(-1, content, config);
    Parser parser(ctx);

    auto mod = parser.ParseModule();

    CodeGen::Config code_gen_config;
    CodeGen codegen(code_gen_config, nullptr);
    codegen.Traverse(mod);
    return codegen.GetResult().content.toStdString();
}

TEST(CommonJS, HookParser) {
    UString content = UString::fromStdString("const result = require('react');\n");

    ParserContext::Config config = ParserContext::Config::Default();
    config.jsx = true;
    config.transpile_jsx = true;
    auto ctx = std::make_shared<ParserContext>(-1, content, config);
    Parser parser(ctx);
    bool is_called = false;
    parser.require_call_created_listener.On([&is_called](const Sp<CallExpression>&) {
        is_called = true;
    });

    auto _mod = parser.ParseModule();

    EXPECT_TRUE(is_called);
}
