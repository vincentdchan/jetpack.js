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

TEST(CommonJS, AddModuleVariable) {
    UString content = UString::fromStdString("exports.name = function() { console.log('name'); }\n");

    ParserContext::Config config = ParserContext::Config::Default();
    config.jsx = true;
    config.transpile_jsx = true;
    auto ctx = std::make_shared<ParserContext>(-1, content, config);
    ctx->is_common_js_ = true;

    Parser parser(ctx);
    auto mod = parser.ParseModule();
    ModuleScope* module_scope = mod->scope->CastToModule();

    std::vector<Sp<Identifier>> unresolved_ids;
    module_scope->ResolveAllSymbols(&unresolved_ids);
    EXPECT_EQ(unresolved_ids.size(), 1);  // has a 'console'

    const auto& var = module_scope->own_variables[u"exports"];
    EXPECT_EQ(var->identifiers.size(), 2);
}
