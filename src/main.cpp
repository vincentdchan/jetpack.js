#include <iostream>
#include <fstream>
#include <string>
#include "parser/parser.hpp"
#include "parser/node_traverser.h"
#include "dumper/ast_to_json.h"
#include "tokenizer/scanner.h"

using namespace parser;

static const char* source = "console.log('hello world')";

std::u16string ReadFileStream(const string& filename) {
    std::ifstream t(filename);
    std::string str((std::istreambuf_iterator<char>(t)),
                    std::istreambuf_iterator<char>());
    return utils::To_UTF16(str);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        return 1;
    }

    auto src = make_shared<UString>();

    (*src) = ReadFileStream(argv[1]);

    std::cout << utils::To_UTF8(*src) << std::endl;

    try {
        Parser parser(src);
        Sp<Module> module_ = parser.ParseModule();
        auto json_result = dumper::AstToJson::Dump(module_);
        std::cout << json_result.dump(2) << std::endl;
    } catch (ParseError& err) {
        std::cerr << err.ErrorMessage() << std::endl;
    } catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
    }
    return 0;
}
