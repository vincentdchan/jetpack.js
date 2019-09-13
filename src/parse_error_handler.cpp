//
// Created by Duzhong Chen on 2019/9/13.
//
#include "parse_error_handler.h"
#include <iostream>


using namespace std;

void ParseErrorHandler::PrintAllErrors() {
    for (auto& error : error_list_) {
        cout << error.line_ << ":" << error.col_ << " "
         << error.name_ << ": " << error.msg_ << endl;
    }
}
