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

    inline void CreateError(std::string msg, int index, int line, int col) {
        ParseError error_ { "<Error>", std::move(msg), index, line, col };
        error_list_.push_back(std::move(error_));
    }

    inline void CreateError(std::string name, std::string msg, int index, int line, int col) {
        ParseError error_ { std::move(name), std::move(msg), index, line, col };
        error_list_.push_back(std::move(error_));
    }

    inline bool TolerateError(std::string msg, int index, int line, int col) {
        CreateError(move(msg), index, line, col);
        return tolerant_;
    }

    inline std::size_t Count() {
        return error_list_.size();
    }

    inline void SetTolerate(bool tol) {
        tolerant_ = tol;
    }

    inline bool GetTolerate() {
        return tolerant_;
    }

    void PrintAllErrors();

private:
    std::list<ParseError> error_list_;
    bool tolerant_ = false;

};
