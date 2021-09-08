//
// Created by Duzhong Chen on 2020/3/24.
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
    auto ctx = std::make_shared<ParserContext>(-1, content, config);
    Parser parser(ctx);

    auto mod = parser.ParseModule();

    CodeGen::Config code_gen_config;
    CodeGen codegen(code_gen_config, nullptr);
    codegen.Traverse(mod);
    return UStringToUtf8(codegen.GetResult().content);
}

TEST(CodeGen, UString) {
    EXPECT_EQ(UStringToUtf8(UStringFromUtf32(U"你好世界", 4)), UStringToUtf8(UString(u"你好世界")));
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
    EXPECT_EQ(ParseAndCodeGen(UStringFromStdString(src)), src);
}

TEST(CodeGen, Object) {
    std::string src = "const obj = {\n"
                      "  a: 3\n"
                      "};\n";

    EXPECT_EQ(ParseAndCodeGen(UStringFromStdString(src)), src);
}

TEST(CodeGen, ObjectStringProps) {
    std::string src = "const obj = {\n"
                      "  '1': 3\n"
                      "};\n";

    EXPECT_EQ(ParseAndCodeGen(UStringFromStdString(src)), src);
}

TEST(CodeGen, ClassExtends) {
    std::string src = "class A extends B {\n"
                      "  constructor() {  }\n"
                      "}\n";

    EXPECT_EQ(ParseAndCodeGen(UStringFromStdString(src)), src);
}

TEST(CodeGen, Regex) {
    std::string src = "const a = /abc/;\n";

    EXPECT_EQ(ParseAndCodeGen(UStringFromStdString(src)), src);
}

TEST(CodeGen, Pattern) {
    std::string src = "var { _lodPlanes, _sizeLods, _sigmas } = _createPlanes();\n";

    EXPECT_EQ(ParseAndCodeGen(UStringFromStdString(src)), src);
}

TEST(CodeGen, TemplateLiteral) {
    std::string src = "const a = `abc${dd}k2`;\n";

    EXPECT_EQ(ParseAndCodeGen(UStringFromStdString(src)), src);
}

TEST(CodeGen, ObjectExpression) {
    std::string src = "const _ = {\n"
                      "  a,\n"
                      "  b,\n"
                      "  c\n"
                      "};\n";

    EXPECT_EQ(ParseAndCodeGen(UStringFromStdString(src)), src);
}

TEST(CodeGen, BinaryExpression) {
    std::string src =
        "if (currentBackground !== background || currentBackgroundVersion !== texture.version || currentTonemapping !== renderer.toneMapping) {\n"
        "  boxMesh.material.needsUpdate = true;\n"
        "  currentBackground = background;\n"
        "  currentBackgroundVersion = texture.version;\n"
        "  currentTonemapping = renderer.toneMapping;\n"
        "}\n";

    EXPECT_EQ(ParseAndCodeGen(UStringFromStdString(src)), src);
}

TEST(CodeGen, ForIn) {
    std::string src =
        "for (var nextKey in source) {\n"
        "  if (Object.prototype.hasOwnProperty.call(source, nextKey)) {\n"
        "    output[nextKey] = source[nextKey];\n"
        "  }\n"
        "}\n";

    EXPECT_EQ(ParseAndCodeGen(UStringFromStdString(src)), src);
}

TEST(CodeGen, BinaryExpression2) {
    std::string src =
        "1 + 2 + 3 + 4 + 5;\n";

    EXPECT_EQ(ParseAndCodeGen(UStringFromStdString(src)), src);
}

TEST(CodeGen, BinaryExpression3) {
    std::string src =
        "1 * 2 + 3 + 4 + 5;\n";

    EXPECT_EQ(ParseAndCodeGen(UStringFromStdString(src)), src);
}

TEST(CodeGen, BinaryExpression4) {
    std::string src =
        "1 + 2 * (3 + 4) + 5;\n";

    EXPECT_EQ(ParseAndCodeGen(UStringFromStdString(src)), src);
}

TEST(CodeGen, BinaryExpression5) {
    std::string src =
        "1 + 2 * 3 + 4 + 5;\n";

    EXPECT_EQ(ParseAndCodeGen(UStringFromStdString(src)), src);
}

TEST(CodeGen, LogicalExpression1) {
    std::string src =
        "a || b && c || d;\n";

    EXPECT_EQ(ParseAndCodeGen(UStringFromStdString(src)), src);
}

TEST(CodeGen, LogicalExpression2) {
    std::string src =
        "(a || b) && (c || d);\n";

    EXPECT_EQ(ParseAndCodeGen(UStringFromStdString(src)), src);
}

TEST(CodeGen, Getter) {
    std::string src =
            "const a = {\n"
            "  get hello() {\n"
            "    return 'world';\n"
            "  }\n"
            "};\n";

    EXPECT_EQ(ParseAndCodeGen(UStringFromStdString(src)), src);
}

TEST(CodeGen, Setter) {
    std::string src =
            "const a = {\n"
            "  set hello(value) {\n"
            "    this.name = value;\n"
            "  }\n"
            "};\n";

    EXPECT_EQ(ParseAndCodeGen(UStringFromStdString(src)), src);
}
