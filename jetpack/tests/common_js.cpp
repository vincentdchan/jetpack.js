//
// Created by Duzhong Chen on 2021/4/8.
//

#include <iostream>
#include <gtest/gtest.h>
#include "parser/Parser.hpp"
#include "parser/ParserContext.h"
#include "parser/NodesMaker.h"
#include "codegen/CodeGen.h"
#include "SimpleAPI.h"
#include "utils/Path.h"

#include "ModuleResolver.h"

using namespace jetpack;
using namespace jetpack::parser;

inline std::string ParseAndCodeGen(std::string_view content) {
    Config config = Config::Default();
    config.jsx = true;
    config.transpile_jsx = true;
    auto ctx = std::make_shared<ParserContext>(-1, content, config);
    Parser parser(ctx);

    auto mod = parser.ParseModule();

    CodeGenConfig code_gen_config;
    code_gen_config.minify = false;
    CodeGen codegen(code_gen_config, nullptr);
    codegen.Traverse(mod);
    return codegen.GetResult().content;
}

TEST(CommonJS, HookParser) {
    std::string content = "const result = require('react');\n";

    Config config = Config::Default();
    config.jsx = true;
    config.transpile_jsx = true;
    auto ctx = std::make_shared<ParserContext>(-1, std::move(content), config);
    Parser parser(ctx);
    bool is_called = false;
    parser.require_call_created_listener.On([&is_called](const Sp<CallExpression>& expr) {
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
    auto ctx = std::make_shared<ParserContext>(-1, content, config);
    ctx->is_common_js_ = true;

    Parser parser(ctx);
    auto mod = parser.ParseModule();
    ModuleScope* module_scope = mod->scope->CastToModule();

    std::vector<Sp<Identifier>> unresolved_ids;
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
    auto ctx = std::make_shared<ParserContext>(-1, content, config);
    ctx->is_common_js_ = true;

    Parser parser(ctx);
    auto mod = parser.ParseModule();
    WrapModuleWithCommonJsTemplate(mod, "require_foo", "__commonJS");

    ModuleScope* module_scope = mod->scope->CastToModule();

    std::vector<Sp<Identifier>> unresolved_ids;
    module_scope->ResolveAllSymbols(&unresolved_ids);

    std::vector<std::tuple<std::string, std::string>> renames {
            { "exports", "a" },
    };
    module_scope->BatchRenameSymbols(renames);

    CodeGenConfig code_gen_config;
    CodeGen codegen(code_gen_config, nullptr);
    codegen.Traverse(mod);
    const auto& output = codegen.GetResult().content;
    EXPECT_EQ(output, "let require_foo = __commonJS(a => {\n"
                      "  a.name = function() {\n"
                      "    console.log('name');\n"
                      "  };\n"
                      "});\n");
}

TEST(CommonJS, Complex) {
    Path path(JETPACK_TEST_RUNNING_DIR);
    path.Join("tests/fixtures/cjs/index.js");

    auto entryPath = path.ToString();
    std::cout << "dir: " << entryPath << std::endl;

    Path outputPath(JETPACK_BUILD_DIR);
    outputPath.Join("cjs_bundle_test.js");

    std::cout << "output dir: " << outputPath.ToString() << std::endl;

    JetpackFlags flags;
    flags |= JetpackFlag::Jsx;
    flags |= JetpackFlag::Sourcemap;
    EXPECT_EQ(simple_api::BundleModule(entryPath, outputPath.ToString(), flags), 0);
}
