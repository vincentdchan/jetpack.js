//
// Created by Duzhong Chen on 2020/4/3.
//

#include <memory>
#include <gtest/gtest.h>
#include <parser/Parser.hpp>
#include "utils/string/UString.h"

#include "codegen/CodeGen.h"

using namespace jetpack;
using namespace jetpack::parser;

inline std::string CF_ParseAndCodeGen(std::string_view content) {
    Config config = Config::Default();
    config.constant_folding = true;
    AstContext ctx;
    Parser parser(ctx, content, config);

    auto mod = parser.ParseModule();

    CodeGenConfig code_gen_config;
    CodeGen codegen(code_gen_config, nullptr);
    codegen.Traverse(*mod);
    return codegen.GetResult().content;
}

TEST(ConstantFolding, AddString1) {
    std::string src = "const a = 'aaa' + 'bbb';\n";
    std::string expected = "const a = \"aaabbb\";\n";

    EXPECT_EQ(CF_ParseAndCodeGen(src), expected);
}

TEST(ConstantFolding, AddString2) {
    std::string src = "const a = 'aaa' + 'bbb' + 'ccc' + 'ddd' + 2;\n";
    std::string expected = "const a = \"aaabbbcccddd\" + 2;\n";

    EXPECT_EQ(CF_ParseAndCodeGen(src), expected);
}

TEST(ConstantFolding, AddInt) {
    std::string src = "const a = 1 + 2 + 3;\n";
    std::string expected = "const a = 6;\n";

    EXPECT_EQ(CF_ParseAndCodeGen(src), expected);
}

TEST(ConstantFolding, Combite) {
    std::string src = "const a = 2 + 2 - 2 + 3;\n";
    std::string expected = "const a = 5;\n";

    EXPECT_EQ(CF_ParseAndCodeGen(src), expected);
}
