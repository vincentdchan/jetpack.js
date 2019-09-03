//
// Created by Duzhong Chen on 2019/9/3.
//
#pragma once

#include <locale>
#include <codecvt>
#include <string>

namespace utils {

    inline std::u32string To_UTF32(const std::string &s) {
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
        return conv.from_bytes(s);
    }

    inline std::string To_UTF8(const std::u32string &s) {
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
        return conv.to_bytes(s);
    }

}
