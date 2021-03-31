//
// Created by Duzhong Chen on 2020/3/20.
//

#pragma once

#include <string>

namespace jetpack {

    class Error : std::exception {
    public:
        Error(std::string msg) : msg_(std::move(msg)) {

        }

        const char *what() const noexcept override {
            return msg_.c_str();
        }

    private:
        std::string msg_;

    };

}
