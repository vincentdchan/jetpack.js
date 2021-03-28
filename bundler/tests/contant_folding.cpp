//
// Created by Duzhong Chen on 2020/4/3.
//

#include <memory>
#include <gtest/gtest.h>
#include <parser/Parser.hpp>

#include "codegen/CodeGen.h"
#include "OutputStream.h"

using namespace jetpack;
using namespace jetpack::parser;

inline std::string CF_ParseAndCodeGen(UString content) {
    ParserContext::Config config = ParserContext::Config::Default();
    config.constant_folding = true;
    auto ctx = std::make_shared<ParserContext>(-1, content, config);
    Parser parser(ctx);

    auto mod = parser.ParseModule();

    MemoryOutputStream ss;
    CodeGen::Config code_gen_config;
    CodeGen codegen(code_gen_config, nullptr, ss);
    codegen.Traverse(mod);
    return ss.ToUTF8();
}

TEST(ConstantFolding, AddString1) {
    std::string src = "const a = 'aaa' + 'bbb';\n";
    std::string expected = "const a = \"aaabbb\";\n";

    EXPECT_EQ(CF_ParseAndCodeGen(UString::fromStdString(src)), expected);
}

TEST(ConstantFolding, AddString2) {
    std::string src = "const a = 'aaa' + 'bbb' + 'ccc' + 'ddd' + 2;\n";
    std::string expected = "const a = \"aaabbbcccddd\" + 2;\n";

    EXPECT_EQ(CF_ParseAndCodeGen(UString::fromStdString(src)), expected);
}

TEST(ConstantFolding, AddInt) {
    std::string src = "const a = 1 + 2 + 3;\n";
    std::string expected = "const a = 6;\n";

    EXPECT_EQ(CF_ParseAndCodeGen(UString::fromStdString(src)), expected);
}

TEST(ConstantFolding, Combite) {
    std::string src = "const a = 2 + 2 - 2 + 3;\n";
    std::string expected = "const a = 5;\n";

    EXPECT_EQ(CF_ParseAndCodeGen(UString::fromStdString(src)), expected);
}
