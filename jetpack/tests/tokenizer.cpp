//
// Created by Vincent Chan on 2021/11/23.
//

#include <gtest/gtest.h>
#include <vector>
#include <memory>
#include "tokenizer/Scanner.h"

using namespace jetpack;

static std::vector<Token> tokenize(const std::string& content) {
    std::vector<Token> result;

    RawMemoryViewOwner mem((std::string_view(content)));
    parser::ParseErrorHandler error_handler;

    std::vector<Sp<Comment>> comments;

    Scanner scanner(mem, error_handler);

    Token next = scanner.Lex();

    while (next.type != JsTokenType::EOF_) {
        result.push_back(next);
        scanner.ScanComments(comments);
        next = scanner.Lex();
    }

    return result;
}

TEST(Tokenizer, Simple) {
    std::string src = "const a = 'aaa\\nbbb';\n";

    auto tokens = tokenize(src);

    std::vector<std::string> got;
    got.reserve(tokens.size());
    for (const auto& tok : tokens) {
        got.push_back(tok.ToString());
    }

    std::vector<std::string> expect {
        "K_Const { const }",
        "Identifier { a }",
        "Assign",
        "StringLiteral { aaa\nbbb }",
        "Semicolon",
    };
    EXPECT_EQ(got, expect);
}

TEST(Tokenizer, Template) {
    std::string src = "`hello ${tmp} world`";

    auto tokens = tokenize(src);

    std::vector<std::string> got;
    got.reserve(tokens.size());
    for (const auto& tok : tokens) {
        got.push_back(tok.ToString());
    }

    std::vector<std::string> expect {
            "Template { hello  }",
            "Identifier { tmp }",
            "Template {  world }",
    };
    EXPECT_EQ(got, expect);
}

TEST(Tokenizer, Template2) {
    std::string src = "`hello world`";

    auto tokens = tokenize(src);

    std::vector<std::string> got;
    got.reserve(tokens.size());
    for (const auto& tok : tokens) {
        got.push_back(tok.ToString());
    }

    std::vector<std::string> expect {
            "Template { hello world }",
    };
    EXPECT_EQ(got, expect);
}
