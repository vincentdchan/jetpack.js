//
// Created by Duzhong Chen on 2020/3/24.
//

#include <gtest/gtest.h>
#include <iostream>
#include "parser/Parser.hpp"
#include "parser/ParserContext.h"

#include "ModuleResolver.h"
#include "codegen/CodeGen.h"

using namespace jetpack;
using namespace jetpack::parser;

inline std::string ParseAndCodeGen(std::string_view content) {
    Config config = Config::Default();
    AstContext ctx;
    Parser parser(ctx, content, config);

    auto mod = parser.ParseModule();

    CodeGenFragment fragment;
    CodeGenConfig code_gen_config;
    CodeGen codegen(code_gen_config, fragment);
    codegen.Traverse(*mod);
    return fragment.content;
}

TEST(CodeGen, Export) {
    EXPECT_EQ(ParseAndCodeGen("export const a = 3"), "export const a = 3;\n");
}

TEST(CodeGen, StringLiteral) {
    EXPECT_EQ(ParseAndCodeGen("export const a = '3'"), "export const a = '3';\n");
}

TEST(CodeGen, Import) {
    std::string src = "import { a } from 'three'";
    EXPECT_EQ(ParseAndCodeGen(std::string(src)), "import { a } from 'three';\n");
}

TEST(CodeGen, Function) {
    std::string src = "function main() {\n"
                      "  console.log('hello world');\n"
                      "}\n";
    EXPECT_EQ(ParseAndCodeGen(std::string(src)), src);
}

TEST(CodeGen, Object) {
    std::string src = "const obj = {\n"
                      "  a: 3\n"
                      "};\n";

    EXPECT_EQ(ParseAndCodeGen(std::string(src)), src);
}

TEST(CodeGen, ObjectStringProps) {
    std::string src = "const obj = {\n"
                      "  '1': 3\n"
                      "};\n";

    EXPECT_EQ(ParseAndCodeGen(std::string(src)), src);
}

TEST(CodeGen, ClassExtends) {
    std::string src = "class A extends B {\n"
                      "  constructor() {  }\n"
                      "}\n";

    EXPECT_EQ(ParseAndCodeGen(std::string(src)), src);
}

TEST(CodeGen, Regex) {
    std::string src = "const a = /abc/;\n";

    EXPECT_EQ(ParseAndCodeGen(std::string(src)), src);
}

TEST(CodeGen, Pattern) {
    std::string src = "var { _lodPlanes, _sizeLods, _sigmas } = _createPlanes();\n";

    EXPECT_EQ(ParseAndCodeGen(std::string(src)), src);
}

TEST(CodeGen, TemplateLiteral) {
    std::string src = "const a = `abc${dd}k2`;\n";

    EXPECT_EQ(ParseAndCodeGen(std::string(src)), src);
}

TEST(CodeGen, ObjectExpression) {
    std::string src = "const _ = {\n"
                      "  a,\n"
                      "  b,\n"
                      "  c\n"
                      "};\n";

    EXPECT_EQ(ParseAndCodeGen(std::string(src)), src);
}

TEST(CodeGen, BinaryExpression) {
    std::string src =
        "if (currentBackground !== background || currentBackgroundVersion !== texture.version || currentTonemapping !== renderer.toneMapping) {\n"
        "  boxMesh.material.needsUpdate = true;\n"
        "  currentBackground = background;\n"
        "  currentBackgroundVersion = texture.version;\n"
        "  currentTonemapping = renderer.toneMapping;\n"
        "}\n";

    EXPECT_EQ(ParseAndCodeGen(std::string(src)), src);
}

TEST(CodeGen, ForIn) {
    std::string src =
        "for (var nextKey in source) {\n"
        "  if (Object.prototype.hasOwnProperty.call(source, nextKey)) {\n"
        "    output[nextKey] = source[nextKey];\n"
        "  }\n"
        "}\n";

    EXPECT_EQ(ParseAndCodeGen(std::string(src)), src);
}

TEST(CodeGen, BinaryExpression2) {
    std::string src =
        "1 + 2 + 3 + 4 + 5;\n";

    EXPECT_EQ(ParseAndCodeGen(std::string(src)), src);
}

TEST(CodeGen, BinaryExpression3) {
    std::string src =
        "1 * 2 + 3 + 4 + 5;\n";

    EXPECT_EQ(ParseAndCodeGen(std::string(src)), src);
}

TEST(CodeGen, BinaryExpression4) {
    std::string src =
        "1 + 2 * (3 + 4) + 5;\n";

    EXPECT_EQ(ParseAndCodeGen(std::string(src)), src);
}

TEST(CodeGen, BinaryExpression5) {
    std::string src =
        "1 + 2 * 3 + 4 + 5;\n";

    EXPECT_EQ(ParseAndCodeGen(std::string(src)), src);
}

TEST(CodeGen, LogicalExpression1) {
    std::string src =
        "a || b && c || d;\n";

    EXPECT_EQ(ParseAndCodeGen(std::string(src)), src);
}

TEST(CodeGen, LogicalExpression2) {
    std::string src =
        "(a || b) && (c || d);\n";

    EXPECT_EQ(ParseAndCodeGen(std::string(src)), src);
}

TEST(CodeGen, Getter) {
    std::string src =
            "const a = {\n"
            "  get hello() {\n"
            "    return 'world';\n"
            "  }\n"
            "};\n";

    EXPECT_EQ(ParseAndCodeGen(std::string(src)), src);
}

TEST(CodeGen, Setter) {
    std::string src =
            "const a = {\n"
            "  set hello(value) {\n"
            "    this.name = value;\n"
            "  }\n"
            "};\n";

    EXPECT_EQ(ParseAndCodeGen(std::string(src)), src);
}
