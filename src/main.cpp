#include <iostream>
#include "parser/parser.hpp"
#include "parser/node_traverser.h"
#include "dumper/ast_to_json.h"
#include "tokenizer/scanner.h"

using namespace parser;

static const char* source = "console.log('hello world')";

int main() {
    ParserCommon::Config config;
    Sp<Module> module_;
    auto src = std::make_shared<UString>(utils::To_UTF16(source));
    Parser parser(src, config);
    if (!parser.ParseModule(module_)) {
        auto err_handler = parser.ErrorHandler();
        err_handler->PrintAllErrors();
        return 1;
    }
    auto json_result = dumper::AstToJson::Dump(module_);
    std::cout << json_result.dump(2) << std::endl;
    return 0;
}