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

TEST(Tokenizer, Token) {
    std::string src = "const a = 'aaa\\nbbb';\n";

    auto tokens = tokenize(src);

    for (const auto& tok : tokens) {
        std::cout << tok.ToString() << std::endl;
    }
}
