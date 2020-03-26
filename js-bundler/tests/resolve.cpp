//
// Created by Duzhong Chen on 2020/3/24.
//

#include <gtest/gtest.h>
#include <unordered_set>
#include <sstream>
#include <parser/ParserContext.h>
#include <parser/Parser.hpp>
#include "../src/codegen/CodeGen.h"
#include "../src/UniqueNameGenerator.h"
#include "../src/ModuleResolver.h"

using namespace rocket_bundle;
using namespace rocket_bundle::parser;

/**
 * do not generate duplicate var name
 */
TEST(MinifyNameGenerator, Next) {
    MinifyNameGenerator gen;
    std::unordered_set<std::u16string> gen_set;

    for (int i = 0; i < 10000; i++) {
        auto next_str = gen.Next(u"");
        EXPECT_TRUE(gen_set.find(next_str) == gen_set.end());
        gen_set.insert(next_str);
    }
}

inline std::string ReplaceDefault(const std::string& src) {
    auto resolver = std::make_shared<ModuleResolver>();
    auto mod = std::make_shared<ModuleFile>();
    mod->module_resolver = resolver;

    auto u16src = std::make_shared<UString>();
    ParserContext::Config config = ParserContext::Config::Default();
    auto ctx = std::make_shared<ParserContext>(u16src, config);
    *u16src = utils::To_UTF16(src);
    Parser parser(ctx);

    mod->ast = parser.ParseModule();
    mod->ReplaceAllNamedExports();

    std::stringstream ss;
    CodeGen::Config code_gen_config;
    CodeGen codegen(code_gen_config, ss);
    codegen.Traverse(mod->ast);

    return ss.str();
}

TEST(ModuleResolver, HandleExportDefault) {
    std::string src = "export default a = 3;";

    auto result = ReplaceDefault(src);

    EXPECT_EQ(result,
            "const a = 3;\n"
            "const default_0 = a;\n");
}

TEST(ModuleResolver, HandleExportDefaultFunction1) {
    std::string src = "export default function() {\n"
                      "}";

    auto result = ReplaceDefault(src);

    EXPECT_EQ(result,
              "function default_0() {}\n");
}

TEST(ModuleResolver, HandleExportDefaultFunction2) {
    std::string src = "export default function name() {\n"
                      "}";

    auto result = ReplaceDefault(src);

    EXPECT_EQ(result,
              "function name() {}\n"
              "const default_0 = name;\n");
}

TEST(ModuleResolver, HandleExportDefaultLiteral) {
    std::string src = "export default 3;\n";

    auto result = ReplaceDefault(src);

    EXPECT_EQ(result,
              "const default_0 = 3;\n");
}

TEST(ModuleResolver, HandleExportDefaultLiteral2) {
    std::string src = "export default `3`;\n";

    auto result = ReplaceDefault(src);

    EXPECT_EQ(result,
              "const default_0 = `3`;\n");
}

TEST(ModuleResolver, HandleExportDefaultLiteral3) {
    std::string src = "export default `\n"
                      "aaaabb\n"
                      "ddd`;\n";

    auto result = ReplaceDefault(src);

    EXPECT_EQ(result,
              "const default_0 = `\n"
              "aaaabb\n"
              "ddd`;\n");
}

//TEST(ModuleResolver, HandleExportDefaultLiteral4) {
//    std::string src = "export default /* glsl */`\n"
//                      "#ifdef USE_ALPHAMAP\n"
//                      "\n"
//                      "\tdiffuseColor.a *= texture2D( alphaMap, vUv ).g;\n"
//                      "\n"
//                      "#endif\n"
//                      "`;";
//
//    auto result = ReplaceDefault(src);
//
//    std::cout << result << std::endl;
//}
