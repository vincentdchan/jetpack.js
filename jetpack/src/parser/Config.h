//
// Created by Duzhong Chen on 2021/9/11.
//

#pragma once

#include <string>
#include <optional>

namespace jetpack { namespace parser {

    struct Config {
    public:
        static Config Default();

        std::optional<std::string> source;
        bool tokens;
        bool comment;
        bool tolerant;
        bool jsx;
        bool typescript;
        bool constant_folding;

        /**
         * transpile jsx when parsing
         */
        bool transpile_jsx;

        bool common_js;

    private:
        Config() = delete;

    };

}}
