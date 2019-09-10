#include <iostream>
#include "parser/parser.hpp"
#include "parser/node_traverser.h"
#include "parser/ast_to_json.h"
#include "tokenizer/scanner.h"

using namespace parser;

static const char* source = "console.log('hello world')";

int main() {
    ParserCommon::Config config;
    Sp<Module> module_;
    auto src = std::make_shared<UString>(utils::To_UTF16(source));
    Parser parser(src, config);
    parser.ParseModule(module_);
    std::cout << "Hello, World!" << std::endl;
    return 0;
}