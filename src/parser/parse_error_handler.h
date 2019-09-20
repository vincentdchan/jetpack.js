//
// Created by Duzhong Chen on 2019/9/3.
//
#pragma once

#include <string>
#include <list>

namespace parser {

    class ParseError {
    public:

        std::string name_;
        std::string msg_;
        int index_ = 0;
        int line_ = 0;
        int col_ = 0;

    };

    class ParseErrorHandler {
    public:

        void CreateError(std::string msg, int index, int line, int col);

        void CreateError(std::string name, std::string msg, int index, int line, int col);

        bool TolerateError(std::string msg, int index, int line, int col);

        inline std::size_t Count() {
            return error_list_.size();
        }

        inline void SetTolerate(bool tol) {
            tolerant_ = tol;
        }

        inline bool GetTolerate() {
            return tolerant_;
        }

        inline std::list<ParseError>& ErrorList() {
            return error_list_;
        }

        void PrintAllErrors();

    private:
        std::list<ParseError> error_list_;
        bool tolerant_ = false;

    };

}
