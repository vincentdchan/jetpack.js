//
// Created by Duzhong Chen on 2020/4/1.
//

#include <gtest/gtest.h>
#include <parser/Parser.hpp>
#include <parser/ParserContext.h>
#include <iostream>

#include "ModuleResolver.h"
#include "codegen/CodeGen.h"

using namespace jetpack;
using namespace jetpack::parser;

inline std::string ParseJSXAndCodeGen(std::string_view content) {
    Config config = Config::Default();
    config.jsx = true;
    config.transpile_jsx = true;
    auto ctx = std::make_shared<ParserContext>(-1, content, config);
    Parser parser(ctx);

    auto mod = parser.ParseModule();

    CodeGenConfig code_gen_config;
    CodeGen codegen(code_gen_config, nullptr);
    codegen.Traverse(*mod);
    return codegen.GetResult().content;
}

TEST(JSX, TranspileSimple1) {
    std::string src =
        "const result = <a></a>;\n";

    std::string expected =
        "const result = React.createElement(\"a\");\n";

    EXPECT_EQ(ParseJSXAndCodeGen(src), expected);
}

TEST(JSX, TranspileSimple2) {
    std::string src =
        "const result = <a />;\n";

    std::string expected =
        "const result = React.createElement(\"a\");\n";

    EXPECT_EQ(ParseJSXAndCodeGen(src), expected);
}

TEST(JSX, TranspileSimple3) {
    std::string src =
        "const result = <a name=\"hello\" />;\n";

    std::string expected =
        "const result = React.createElement(\"a\", {\n"
        "  name: \"hello\"\n"
        "});\n";

    EXPECT_EQ(ParseJSXAndCodeGen(src), expected);
}

TEST(JSX, TranspileSimple4) {
    std::string src =
        "const result = <a name={1 + 1} />;\n";

    std::string expected =
        "const result = React.createElement(\"a\", {\n"
        "  name: 1 + 1\n"
        "});\n";

    EXPECT_EQ(ParseJSXAndCodeGen(src), expected);
}

TEST(JSX, TranspileSpread1) {
    std::string src =
        "const result = <a name={1 + 1} {...props} />;\n";

    std::string expected =
        "const result = React.createElement(\"a\", {\n"
        "  name: 1 + 1,\n"
        "  ...props\n"
        "});\n";

    EXPECT_EQ(ParseJSXAndCodeGen(src), expected);
}

TEST(JSX, TranspileChildren) {
    std::string src =
        "const result = <a>aaa</a>;\n";

    std::string expected =
        "const result = React.createElement(\"a\", null, \"aaa\");\n";

    EXPECT_EQ(ParseJSXAndCodeGen(src), expected);
}

TEST(JSX, TranspileRecursively) {
    std::string src =
        "const result = <a><b /></a>;\n";

    std::string expected =
        "const result = React.createElement(\"a\", null, React.createElement(\"b\"));\n";

    EXPECT_EQ(ParseJSXAndCodeGen(src), expected);
}
