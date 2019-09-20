//
// Created by Duzhong Chen on 2019/9/13.
//
#include "parse_error_handler.h"
#include <iostream>

using namespace std;

void ParseErrorHandler::CreateError(std::string msg, int index, int line, int col) {
    ParseError error_ { "<Error>", std::move(msg), index, line, col };
    error_list_.push_back(std::move(error_));
}

void ParseErrorHandler::CreateError(std::string name, std::string msg, int index, int line, int col) {
    ParseError error_ { std::move(name), std::move(msg), index, line, col };
    error_list_.push_back(std::move(error_));
}

bool ParseErrorHandler::TolerateError(std::string msg, int index, int line, int col) {
    CreateError(move(msg), index, line, col);
    return tolerant_;
}

void ParseErrorHandler::PrintAllErrors() {
    for (auto& error : error_list_) {
        cout << error.line_ << ":" << error.col_ << " "
         << error.name_ << ": " << error.msg_ << endl;
    }
}
