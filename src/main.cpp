#include <iostream>
#include <memory>
#include "utils.h"
#include "parser/parser.hpp"

static const char * SOURCE = "console.log('hello')";

using namespace std;
using namespace parser;

int main() {
    auto source = make_shared<UString>(utils::To_UTF16(SOURCE));
    auto parser_config = Parser::Config::Default();
    Parser parser(source, parser_config);
    return 0;
}
