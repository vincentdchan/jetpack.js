//
// Created by Duzhong Chen on 2020/3/26.
//

#include <memory>
#include <string_view>
#include <gtest/gtest.h>
#include <parser/Parser.hpp>

#include "codegen/CodeGen.h"

using namespace jetpack;
using namespace jetpack::parser;

inline Module* ParseString(AstContext& ctx, std::string_view src) {
    Parser parser(ctx, src, Config::Default());
    return parser.ParseModule();
}

inline std::string GenCode(Module* mod) {
    CodeGenConfig code_gen_config;
    CodeGenFragment fragment;
    CodeGen codegen(code_gen_config, fragment);
    codegen.Traverse(*mod);
    return fragment.content;
}

TEST(Scope, Collect) {
    auto content = "var name = 3;";

    Config config = Config::Default();
    AstContext ctx;
    Parser parser(ctx, content, config);

    auto mod = parser.ParseModule();
    mod->scope->ResolveAllSymbols(nullptr);

    EXPECT_EQ(mod->scope->own_variables.size(), 1);
    EXPECT_TRUE(mod->scope->own_variables.find("name") != mod->scope->own_variables.end());
}

TEST(Scope, Rename) {
    AstContext ctx;
    auto mod = ParseString(ctx, "var name = 3;\n");
    mod->scope->ResolveAllSymbols(nullptr);

    ModuleScope::ChangeSet changeset;
    changeset.emplace_back("name", "new_name");
    EXPECT_TRUE(mod->scope->BatchRenameSymbols(changeset));

    EXPECT_EQ(mod->scope->own_variables.size(), 1);
    EXPECT_TRUE(mod->scope->own_variables.find("name") == mod->scope->own_variables.end());

    EXPECT_EQ(GenCode(mod), "var new_name = 3;\n");
}

TEST(Scope, RenameImportNamespace) {
    AstContext ctx;
    auto mod = ParseString(ctx, "import * as name from 'main';\n");
    mod->scope->ResolveAllSymbols(nullptr);

    ModuleScope::ChangeSet changeset;
    changeset.emplace_back("name", "new_name");
    EXPECT_TRUE(mod->scope->BatchRenameSymbols(changeset));

    EXPECT_EQ(mod->scope->own_variables.size(), 1);
    EXPECT_TRUE(mod->scope->own_variables.find("name") == mod->scope->own_variables.end());

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

    AstContext ctx;
    auto mod = ParseString(ctx, src);
    mod->scope->ResolveAllSymbols(nullptr);

    ModuleScope::ChangeSet changeset;
    changeset.emplace_back("name", "new_name");
    EXPECT_TRUE(mod->scope->BatchRenameSymbols(changeset));

    EXPECT_TRUE(mod->scope->own_variables.find("name") == mod->scope->own_variables.end());

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

    AstContext ctx;
    auto mod = ParseString(ctx, src);
    mod->scope->ResolveAllSymbols(nullptr);

    ModuleScope::ChangeSet changeset;
    changeset.emplace_back("ok", "ok1");
    EXPECT_TRUE(mod->scope->BatchRenameSymbols(changeset));

    EXPECT_EQ(GenCode(mod), expected);
}

TEST(Scope, RenameFunction3) {
    auto src = "var name = 3;\n"
               "function ok(name) {\n"
               "  console.log(name);\n"
               "}\n";

    std::string expected = "var rename = 3;\n"
                           "function ok(name) {\n"
                           "  console.log(name);\n"
                           "}\n";

    AstContext ctx;
    auto mod = ParseString(ctx, src);
    mod->scope->ResolveAllSymbols(nullptr);

    ModuleScope::ChangeSet changeset;
    changeset.emplace_back("name", "rename");
    EXPECT_TRUE(mod->scope->BatchRenameSymbols(changeset));

    EXPECT_EQ(GenCode(mod), expected);
}

TEST(Scope, RenameObjectPattern) {
    auto src = "var { name: other } = obj;\n";

    std::string expected = "var { name: renamed } = obj;\n";

    AstContext ctx;
    auto mod = ParseString(ctx, src);
    mod->scope->ResolveAllSymbols(nullptr);

    ModuleScope::ChangeSet changeset;
    changeset.emplace_back("other", "renamed");
    EXPECT_TRUE(mod->scope->BatchRenameSymbols(changeset));

    EXPECT_EQ(GenCode(mod), expected);
}

TEST(Scope, RenameObjectPattern2) {
    auto src = "var { name: other } = obj;\n";

    std::string expected = "var { name: other } = obj;\n";

    AstContext ctx;
    auto mod = ParseString(ctx, src);
    mod->scope->ResolveAllSymbols(nullptr);

    ModuleScope::ChangeSet changeset;
    changeset.emplace_back("name", "ok1");
    EXPECT_FALSE(mod->scope->BatchRenameSymbols(changeset));

    EXPECT_EQ(GenCode(mod), expected);
}

TEST(Scope, RenameObjectPattern3) {
    auto src = "var { name } = obj;\n";

    std::string expected = "var { name: renamed } = obj;\n";

    AstContext ctx;
    auto mod = ParseString(ctx, src);
    mod->scope->ResolveAllSymbols(nullptr);

    ModuleScope::ChangeSet changeset;
    changeset.emplace_back("name", "renamed");
    EXPECT_TRUE(mod->scope->BatchRenameSymbols(changeset));

    EXPECT_EQ(GenCode(mod), expected);
}

TEST(Scope, Cls) {
    auto src = "const print = 'hello world';\n"
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

    AstContext ctx;
    auto mod = ParseString(ctx, src);
    mod->scope->ResolveAllSymbols(nullptr);

    ModuleScope::ChangeSet changeset;
    changeset.emplace_back("print", "renamed");
    EXPECT_TRUE(mod->scope->BatchRenameSymbols(changeset));

    EXPECT_EQ(GenCode(mod), expected);
}

TEST(Scope, RenameImport) {
    auto src = "import { name } from 'main';\n";

    std::string expected = "import { name as renamed } from 'main';\n";

    AstContext ctx;
    auto mod = ParseString(ctx, src);
    mod->scope->ResolveAllSymbols(nullptr);

    ModuleScope::ChangeSet changeset;
    changeset.emplace_back("name", "renamed");
    EXPECT_TRUE(mod->scope->BatchRenameSymbols(changeset));

    EXPECT_EQ(GenCode(mod), expected);
}

TEST(Scope, RenameImport3) {
    auto src = "import { a, fun, ooo } from './a';\n"
               "\n"
               "var b = 44444;\n"
               "\n"
               "export default a + 3 + b + fun();";

    AstContext ctx;
    auto mod = ParseString(ctx, src);
    mod->scope->ResolveAllSymbols(nullptr);

    ModuleScope::ChangeSet changeset;
    changeset.emplace_back("a", "p");
    EXPECT_TRUE(mod->scope->BatchRenameSymbols(changeset));

    EXPECT_GT(mod->body.size(), 0);
    auto import_decl = dynamic_cast<ImportDeclaration*>(*mod->body.begin());
    EXPECT_NE(import_decl, nullptr);

    auto first_spec = dynamic_cast<ImportSpecifier*>(import_decl->specifiers[0]);
    EXPECT_NE(import_decl, nullptr);

    EXPECT_EQ(first_spec->local->name, "p");
}

TEST(Scope, RenameImportDefault) {
    auto src = "import React from 'react';\n";

    std::string expected = "import Angular from 'react';\n";

    AstContext ctx;
    auto mod = ParseString(ctx, src);
    mod->scope->ResolveAllSymbols(nullptr);

    ModuleScope::ChangeSet changeset;
    changeset.emplace_back("React", "Angular");
    EXPECT_TRUE(mod->scope->BatchRenameSymbols(changeset));

    EXPECT_EQ(GenCode(mod), expected);
}

TEST(Scope, RenameImport2) {
    auto src = "import { cc as name } from 'main';\n"
               "console.log(name);\n";

    std::string expected = "import { cc as renamed } from 'main';\n"
                           "console.log(renamed);\n";

    AstContext ctx;
    auto mod = ParseString(ctx, src);
    mod->scope->ResolveAllSymbols(nullptr);

    ModuleScope::ChangeSet changeset;
    changeset.emplace_back("name", "renamed");
    EXPECT_TRUE(mod->scope->BatchRenameSymbols(changeset));

    EXPECT_EQ(GenCode(mod), expected);
}

TEST(Scope, RenameExport1) {
    auto src = "const name = 3;\n"
               "export { name as foo };\n";

    std::string expected = "const renamed = 3;\n"
                           "export { renamed as foo };\n";

    AstContext ctx;
    auto mod = ParseString(ctx, src);
    mod->scope->ResolveAllSymbols(nullptr);

    ModuleScope::ChangeSet changeset;
    changeset.emplace_back("name", "renamed");
    EXPECT_TRUE(mod->scope->BatchRenameSymbols(changeset));

    EXPECT_EQ(GenCode(mod), expected);
}

TEST(Scope, RenameExport2) {
    auto src = "const name = 3;\n"
               "export { name };\n";

    std::string expected = "const renamed = 3;\n"
                           "export { renamed as name };\n";

    AstContext ctx;
    auto mod = ParseString(ctx, src);
    mod->scope->ResolveAllSymbols(nullptr);

    ModuleScope::ChangeSet changeset;
    changeset.emplace_back("name", "renamed");
    EXPECT_TRUE(mod->scope->BatchRenameSymbols(changeset));

    EXPECT_EQ(GenCode(mod), expected);
}
