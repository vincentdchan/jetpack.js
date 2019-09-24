//
// Created by Duzhong Chen on 2019/9/3.
//
#pragma once

#include <string>
#include <list>

namespace parser {

    class ParseError: public std::exception {
    public:
        ParseError() = default;

        std::string name_;
        std::string msg_;
        int index_ = 0;
        int line_ = 0;
        int col_ = 0;

        [[nodiscard]] const char *what() const noexcept override;

        [[nodiscard]] virtual std::string ErrorMessage() const;

    };

    class ParseAssertFailed: public ParseError {
    public:

        ParseAssertFailed(std::string message, int line, int col);

        [[nodiscard]] const char *what() const noexcept override;

    };

    class ParseErrorHandler {
    public:

        ParseError CreateError(std::string msg, int index, int line, int col);

        ParseError CreateError(std::string name, std::string msg, int index, int line, int col);

        void TolerateError(const ParseError& err);

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
