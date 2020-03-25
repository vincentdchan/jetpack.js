//
// Created by Duzhong Chen on 2020/3/24.
//

#include <gtest/gtest.h>
#include <parser/Parser.hpp>
#include <parser/ParserContext.h>
#include <iostream>
#include <sstream>

#include "../src/ModuleResolver.h"
#include "../src/codegen/CodeGen.h"

using namespace rocket_bundle;
using namespace rocket_bundle::parser;

inline std::string ParseAndCodeGen(UString content) {
    auto src = std::make_shared<UString>();
    ParserContext::Config config = ParserContext::Config::Default();
    auto ctx = std::make_shared<ParserContext>(src, config);
    *src = std::move(content);
    Parser parser(ctx);

    auto mod = parser.ParseModule();

    std::stringstream ss;
    CodeGen::Config code_gen_config;
    CodeGen codegen(code_gen_config, ss);
    codegen.Traverse(mod);
    return ss.str();
}

TEST(CodeGen, Export) {
    EXPECT_EQ(ParseAndCodeGen(u"export const a = 3"), "export const a = 3;\n");
}

TEST(CodeGen, StringLiteral) {
    EXPECT_EQ(ParseAndCodeGen(u"export const a = '3'"), "export const a = '3';\n");
}

TEST(CodeGen, Import) {
    UString src = u"import { a } from 'three'";
    EXPECT_EQ(ParseAndCodeGen(src), "import { a } from 'three';\n");
}

TEST(CodeGen, Function) {
    std::string src = "function main() {\n"
                      "  console.log('hello world');\n"
                      "}\n";
    EXPECT_EQ(ParseAndCodeGen(utils::To_UTF16(src)), src);
}

TEST(CodeGen, Object) {
    std::string src = "const obj = {\n"
                      "  a: 3\n"
                      "};\n";

    EXPECT_EQ(ParseAndCodeGen(utils::To_UTF16(src)), src);
}

TEST(CodeGen, ObjectStringProps) {
    std::string src = "const obj = {\n"
                      "  '1': 3\n"
                      "};\n";

    EXPECT_EQ(ParseAndCodeGen(utils::To_UTF16(src)), src);
}

TEST(CodeGen, ClassExtends) {
    std::string src = "class A extends B {\n"
                      "  constructor() {  }\n"
                      "}\n";

    EXPECT_EQ(ParseAndCodeGen(utils::To_UTF16(src)), src);
}

TEST(CodeGen, Regex) {
    std::string src = "const a = /abc/;\n";

    EXPECT_EQ(ParseAndCodeGen(utils::To_UTF16(src)), src);
}

TEST(CodeGen, Pattern) {
    std::string src = "var { _lodPlanes, _sizeLods, _sigmas } = _createPlanes();\n";

    EXPECT_EQ(ParseAndCodeGen(utils::To_UTF16(src)), src);
}

TEST(CodeGen, TemplateLiteral) {
    std::string src = "const a = `abc${dd}k2`;\n";

    EXPECT_EQ(ParseAndCodeGen(utils::To_UTF16(src)), src);
}
