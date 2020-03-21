//
// Created by Duzhong Chen on 2019/9/13.
//
#include "ParseErrorHandler.h"
#include <iostream>
#include <sstream>

using namespace std;

namespace rocket_bundle::parser {

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

    void ParseErrorHandler::TolerateError(const ParseError& err) {
        if (!tolerant_) {
            throw err;
        }
    }

    void ParseErrorHandler::PrintAllErrors() {
        for (auto& error : error_list_) {
            cout << error.line_ + 1 << ":" << error.col_ << " "
                 << error.name_ << ": " << error.msg_ << endl;
        }
    }

    ParseAssertFailed::ParseAssertFailed(std::string message, int line, int col) {
        name_ = "<AssertFailed>";
        msg_ = move(message);
        line_ = line;
        col_ = col;
    }

    const char *ParseAssertFailed::what() const noexcept {
        return "ParseAssertFailed";
    }


    const char *ParseError::what() const noexcept {
        return "ParseError";
    }

    std::string ParseError::ErrorMessage() const {
        stringstream ss;
        ss << line_ + 1 << ":" << col_ << ": " << name_ << ": " << msg_;
        return ss.str();
    }

}
