//
// Created by Duzhong Chen on 2021/4/8.
//

#include <iostream>
#include <gtest/gtest.h>
#include <filesystem.hpp>
#include "parser/Parser.hpp"
#include "parser/ParserContext.h"
#include "parser/NodesMaker.h"
#include "codegen/CodeGen.h"
#include "CodeGenFragment.h"
#include "SimpleAPI.h"

#include "ModuleResolver.h"

using namespace jetpack;
using namespace jetpack::parser;

inline std::string ParseAndCodeGen(std::string_view content) {
    Config config = Config::Default();
    config.jsx = true;
    config.transpile_jsx = true;
    AstContext ctx;
    Parser parser(ctx, content, config);

    auto mod = parser.ParseModule();

    CodeGenFragment fragment;
    CodeGenConfig code_gen_config;
    code_gen_config.minify = false;
    CodeGen codegen(code_gen_config, fragment);
    codegen.Traverse(*mod);
    return fragment.content;
}

TEST(CommonJS, HookParser) {
    std::string content = "const result = require('react');\n";

    Config config = Config::Default();
    config.jsx = true;
    config.transpile_jsx = true;
    AstContext ctx;
    Parser parser(ctx, content, config);
    bool is_called = false;
    parser.require_call_created_listener.On([&is_called](CallExpression* expr) {
        is_called = true;
        return expr;
    });

    auto _mod = parser.ParseModule();

    EXPECT_TRUE(is_called);
}

TEST(CommonJS, AddModuleVariable) {
    auto content = "exports.name = function() { console.log('name'); }\n";

    Config config = Config::Default();
    config.jsx = true;
    config.transpile_jsx = true;

    AstContext ctx;
    Parser parser(ctx, content, config);
    parser.Context()->is_common_js_ = true;
    auto mod = parser.ParseModule();
    ModuleScope* module_scope = mod->scope->CastToModule();

    std::vector<Identifier*> unresolved_ids;
    module_scope->ResolveAllSymbols(&unresolved_ids);
    EXPECT_EQ(unresolved_ids.size(), 1);  // has a 'console'

    const auto& var = module_scope->own_variables["exports"];
    EXPECT_EQ(var->identifiers.size(), 2);
}

TEST(CommonJS, CodeGen) {
    auto content = "exports.name = function() { console.log('name'); }\n";

    Config config = Config::Default();
    config.jsx = true;
    config.transpile_jsx = true;

    AstContext ctx;
    Parser parser(ctx, content, config);
    parser.Context()->is_common_js_ = true;
    auto mod = parser.ParseModule();
    WrapModuleWithCommonJsTemplate(ctx, *mod, "require_foo", "__commonJS");

    ModuleScope* module_scope = mod->scope->CastToModule();

    std::vector<Identifier*> unresolved_ids;
    module_scope->ResolveAllSymbols(&unresolved_ids);

    std::vector<std::tuple<std::string, std::string>> renames {
            { "exports", "a" },
    };
    module_scope->BatchRenameSymbols(renames);

    CodeGenFragment fragment;
    CodeGenConfig code_gen_config;
    CodeGen codegen(code_gen_config, fragment);
    codegen.Traverse(*mod);
    const auto& output = fragment.content;
    EXPECT_EQ(output, "let require_foo = __commonJS(a => {\n"
                      "  a.name = function() {\n"
                      "    console.log('name');\n"
                      "  };\n"
                      "});\n");
}

TEST(CommonJS, Complex) {
    ghc::filesystem::path path(JETPACK_TEST_RUNNING_DIR);
    path.append("tests/fixtures/cjs/index.js");

    auto entryPath = path.string();
    std::cout << "dir: " << entryPath << std::endl;

    ghc::filesystem::path outputPath(JETPACK_BUILD_DIR);
    outputPath.append("cjs_bundle_test.js");

    std::cout << "output dir: " << outputPath.string() << std::endl;

    JetpackFlags flags;
    flags |= JETPACK_JSX;
    flags |= JETPACK_SOURCEMAP;
    flags |= JETPACK_TRACE_FILE;
    EXPECT_EQ(jetpack_bundle_module(entryPath.c_str(), outputPath.string().c_str(), static_cast<int>(flags), nullptr), 0);
}
