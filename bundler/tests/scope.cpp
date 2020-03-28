//
// Created by Duzhong Chen on 2020/3/26.
//

#include <string>
#include <memory>
#include <gtest/gtest.h>
#include <parser/Parser.hpp>

#include "../src/codegen/CodeGen.h"

using namespace rocket_bundle;
using namespace rocket_bundle::parser;

inline Sp<Module> ParseString(const std::string& src) {
    auto u16src = std::make_shared<UString>();
    ParserContext::Config config = ParserContext::Config::Default();
    auto ctx = std::make_shared<ParserContext>(u16src, config);
    *u16src = utils::To_UTF16(src);
    Parser parser(ctx);
    return parser.ParseModule();
}

inline std::string GenCode(Sp<Module> mod) {
    std::stringstream ss;
    CodeGen::Config code_gen_config;
    CodeGen codegen(code_gen_config, ss);
    codegen.Traverse(mod);
    return ss.str();
}

TEST(Scope, Collect) {
    std::string src = "var name = 3;";

    auto u16src = std::make_shared<UString>();
    ParserContext::Config config = ParserContext::Config::Default();
    auto ctx = std::make_shared<ParserContext>(u16src, config);
    *u16src = utils::To_UTF16(src);
    Parser parser(ctx);

    auto mod = parser.ParseModule();
    mod->scope->ResolveAllSymbols();

    EXPECT_EQ(mod->scope->own_variables.size(), 1);
    EXPECT_TRUE(mod->scope->own_variables.find(u"name") != mod->scope->own_variables.end());
}

TEST(Scope, Rename) {
    std::string src = "var name = 3;\n";

    auto mod = ParseString(src);
    mod->scope->ResolveAllSymbols();
    mod->scope->RenameSymbol(u"name", u"new_name");

    EXPECT_EQ(mod->scope->own_variables.size(), 1);
    EXPECT_TRUE(mod->scope->own_variables.find(u"name") == mod->scope->own_variables.end());

    std::stringstream ss;
    CodeGen::Config code_gen_config;
    CodeGen codegen(code_gen_config, ss);
    codegen.Traverse(mod);
    EXPECT_EQ(GenCode(mod), "var new_name = 3;\n");
}

TEST(Scope, RenameImportNamespace) {
    std::string src = "import * as name from 'main';\n";

    auto mod = ParseString(src);
    mod->scope->ResolveAllSymbols();
    mod->scope->RenameSymbol(u"name", u"new_name");

    EXPECT_EQ(mod->scope->own_variables.size(), 1);
    EXPECT_TRUE(mod->scope->own_variables.find(u"name") == mod->scope->own_variables.end());

    std::stringstream ss;
    CodeGen::Config code_gen_config;
    CodeGen codegen(code_gen_config, ss);
    codegen.Traverse(mod);
    EXPECT_EQ(GenCode(mod), "import * as new_name from 'main';\n");
}

TEST(Scope, RenameFunction1) {
    std::string src = "var name = 3;\n"
                      "function ok() {\n"
                      "  console.log(name);\n"
                      "}\n";

    std::string expected = "var new_name = 3;\n"
                           "function ok() {\n"
                           "  console.log(new_name);\n"
                           "}\n";

    auto mod = ParseString(src);
    mod->scope->ResolveAllSymbols();
    EXPECT_TRUE(mod->scope->RenameSymbol(u"name", u"new_name"));

    EXPECT_TRUE(mod->scope->own_variables.find(u"name") == mod->scope->own_variables.end());

    std::stringstream ss;
    CodeGen::Config code_gen_config;
    CodeGen codegen(code_gen_config, ss);
    codegen.Traverse(mod);
    EXPECT_EQ(GenCode(mod), expected);
}

TEST(Scope, RenameFunction2) {
    std::string src = "var name = 3;\n"
                      "function ok() {\n"
                      "  console.log(name);\n"
                      "}\n";

    std::string expected = "var name = 3;\n"
                           "function ok1() {\n"
                           "  console.log(name);\n"
                           "}\n";

    auto mod = ParseString(src);
    mod->scope->ResolveAllSymbols();
    EXPECT_TRUE(mod->scope->RenameSymbol(u"ok", u"ok1"));

    std::stringstream ss;
    CodeGen::Config code_gen_config;
    CodeGen codegen(code_gen_config, ss);
    codegen.Traverse(mod);
    EXPECT_EQ(GenCode(mod), expected);
}

TEST(Scope, RenameFunction3) {
    std::string src = "var name = 3;\n"
                      "function ok(name) {\n"
                      "  console.log(name);\n"
                      "}\n";

    std::string expected = "var rename = 3;\n"
                           "function ok(name) {\n"
                           "  console.log(name);\n"
                           "}\n";

    auto mod = ParseString(src);
    mod->scope->ResolveAllSymbols();
    EXPECT_TRUE(mod->scope->RenameSymbol(u"name", u"rename"));

    std::stringstream ss;
    CodeGen::Config code_gen_config;
    CodeGen codegen(code_gen_config, ss);
    codegen.Traverse(mod);
    EXPECT_EQ(GenCode(mod), expected);
}

TEST(Scope, RenameObjectPattern) {
    std::string src = "var { name: other } = obj;\n";

    std::string expected = "var { name: renamed } = obj;\n";

    auto mod = ParseString(src);
    mod->scope->ResolveAllSymbols();
    EXPECT_TRUE(mod->scope->RenameSymbol(u"other", u"renamed"));

    std::stringstream ss;
    CodeGen::Config code_gen_config;
    CodeGen codegen(code_gen_config, ss);
    codegen.Traverse(mod);
    EXPECT_EQ(GenCode(mod), expected);
}

TEST(Scope, RenameObjectPattern2) {
    std::string src = "var { name: other } = obj;\n";

    std::string expected = "var { name: other } = obj;\n";

    auto mod = ParseString(src);
    mod->scope->ResolveAllSymbols();
    EXPECT_FALSE(mod->scope->RenameSymbol(u"name", u"renamed"));

    std::stringstream ss;
    CodeGen::Config code_gen_config;
    CodeGen codegen(code_gen_config, ss);
    codegen.Traverse(mod);
    EXPECT_EQ(GenCode(mod), expected);
}

TEST(Scope, RenameObjectPattern3) {
    std::string src = "var { name } = obj;\n";

    std::string expected = "var { name: renamed } = obj;\n";

    auto mod = ParseString(src);
    mod->scope->ResolveAllSymbols();
    EXPECT_TRUE(mod->scope->RenameSymbol(u"name", u"renamed"));

    std::stringstream ss;
    CodeGen::Config code_gen_config;
    CodeGen codegen(code_gen_config, ss);
    codegen.Traverse(mod);
    EXPECT_EQ(GenCode(mod), expected);
}

TEST(Scope, Cls) {
    std::string src = "const print = 'hello world';\n"
                      "class A {"
                      "  print() {\n"
                      "    console.log(print);\n"
                      "  }\n"
                      "}\n";

    std::string expected = "const renamed = 'hello world';\n"
                           "class A {\n"
                           "  print() {\n"
                           "    console.log(renamed);\n"
                           "  }\n"
                           "}\n";

    auto mod = ParseString(src);
    mod->scope->ResolveAllSymbols();
    EXPECT_TRUE(mod->scope->RenameSymbol(u"print", u"renamed"));

    std::stringstream ss;
    CodeGen::Config code_gen_config;
    CodeGen codegen(code_gen_config, ss);
    codegen.Traverse(mod);
    EXPECT_EQ(GenCode(mod), expected);
}

TEST(Scope, RenameImport) {
    std::string src = "import { name } from 'main';\n";

    std::string expected = "import { name as renamed } from 'main';\n";

    auto mod = ParseString(src);
    mod->scope->ResolveAllSymbols();
    EXPECT_TRUE(mod->scope->RenameSymbol(u"name", u"renamed"));

    std::stringstream ss;
    CodeGen::Config code_gen_config;
    CodeGen codegen(code_gen_config, ss);
    codegen.Traverse(mod);
    EXPECT_EQ(GenCode(mod), expected);
}

TEST(Scope, RenameImportDefault) {
    std::string src = "import React from 'react';\n";

    std::string expected = "import Angular from 'react';\n";

    auto mod = ParseString(src);
    mod->scope->ResolveAllSymbols();
    EXPECT_TRUE(mod->scope->RenameSymbol(u"React", u"Angular"));

    std::stringstream ss;
    CodeGen::Config code_gen_config;
    CodeGen codegen(code_gen_config, ss);
    codegen.Traverse(mod);
    EXPECT_EQ(GenCode(mod), expected);
}

TEST(Scope, RenameImport2) {
    std::string src = "import { cc as name } from 'main';\n"
                      "console.log(name);\n";

    std::string expected = "import { cc as renamed } from 'main';\n"
                           "console.log(renamed);\n";

    auto mod = ParseString(src);
    mod->scope->ResolveAllSymbols();
    EXPECT_TRUE(mod->scope->RenameSymbol(u"name", u"renamed"));

    std::stringstream ss;
    CodeGen::Config code_gen_config;
    CodeGen codegen(code_gen_config, ss);
    codegen.Traverse(mod);
    EXPECT_EQ(GenCode(mod), expected);
}

TEST(Scope, RenameExport1) {
    std::string src = "const name = 3;\n"
                      "export { name as foo };\n";

    std::string expected = "const renamed = 3;\n"
                           "export { renamed as foo };\n";

    auto mod = ParseString(src);
    mod->scope->ResolveAllSymbols();
    EXPECT_TRUE(mod->scope->RenameSymbol(u"name", u"renamed"));

    std::stringstream ss;
    CodeGen::Config code_gen_config;
    CodeGen codegen(code_gen_config, ss);
    codegen.Traverse(mod);
    EXPECT_EQ(GenCode(mod), expected);
}

TEST(Scope, RenameExport2) {
    std::string src = "const name = 3;\n"
                      "export { name };\n";

    std::string expected = "const renamed = 3;\n"
                           "export { renamed as name };\n";

    auto mod = ParseString(src);
    mod->scope->ResolveAllSymbols();
    EXPECT_TRUE(mod->scope->RenameSymbol(u"name", u"renamed"));

    std::stringstream ss;
    CodeGen::Config code_gen_config;
    CodeGen codegen(code_gen_config, ss);
    codegen.Traverse(mod);
    EXPECT_EQ(GenCode(mod), expected);
}
