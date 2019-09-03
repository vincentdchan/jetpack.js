//
// Created by Duzhong Chen on 2019/9/3.
//
#pragma once

#include <string>
#include <list>

class ParseErrorHandler {
public:

    struct ParseError {
        std::string name_;
        std::string msg_;
        int index_;
        int line_;
        int col_;
    };

    void CreateError(std::string msg, int index, int line, int col) {
        ParseError error_ { "<Error>", std::move(msg), index, line, col };
        error_list_.push_back(std::move(error_));
    }

    void CreateError(std::string name, std::string msg, int index, int line, int col) {
        ParseError error_ { std::move(name), std::move(msg), index, line, col };
        error_list_.push_back(std::move(error_));
    }

private:
    std::list<ParseError> error_list_;

};
