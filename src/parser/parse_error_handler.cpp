//
// Created by Duzhong Chen on 2019/9/13.
//
#include "parse_error_handler.h"
#include <iostream>

using namespace std;

namespace parser {

    ParseError ParseErrorHandler::CreateError(std::string msg, int index, int line, int col) {
        ParseError error_;
        error_.name_ = "<Error>";
        error_.msg_ = std::move(msg);
        error_.index_ = index;
        error_.line_ = line;
        error_.col_ = col;
        error_list_.push_back(error_);
        return error_;
    }

    ParseError ParseErrorHandler::CreateError(std::string name, std::string msg, int index, int line, int col) {
        ParseError error_;
        error_.name_ = std::move(name);
        error_.msg_ = std::move(msg);
        error_.index_ = index;
        error_.line_ = line;
        error_.col_ = col;
        error_list_.push_back(error_);
        return error_;
    }

    void ParseErrorHandler::TolerateError(std::exception& err) {
        if (!tolerant_) {
            throw err;
        }
    }

    void ParseErrorHandler::PrintAllErrors() {
        for (auto& error : error_list_) {
            cout << error.line_ << ":" << error.col_ << " "
                 << error.name_ << ": " << error.msg_ << endl;
        }
    }

}
