//
// Created by Duzhong Chen on 2020/7/14.
//

#include "./OutputStream.h"

namespace jetpack {

    FileOutputStream::FileOutputStream(const std::string &path): ofs(path, std::ios::out) {

    }

    OutputStream& FileOutputStream::operator<<(const char16_t* str) {
        ofs << utils::To_UTF8(str);
        return *this;
    }

    OutputStream& FileOutputStream::operator<<(const UString& str) {
        ofs << utils::To_UTF8(str);
        return *this;
    }

    OutputStream& FileOutputStream::operator<<(char ch) {
        ofs << ch;
        return *this;
    }

    OutputStream& MemoryOutputStream::operator<<(const char16_t *str) {
        ss << utils::To_UTF8(str);
        return *this;
    }

    OutputStream& MemoryOutputStream::operator<<(const UString &str) {
        ss << utils::To_UTF8(str);
        return *this;
    }

    OutputStream& MemoryOutputStream::operator<<(char ch) {
        ss << ch;
        return *this;
    }

    MemoryOutputStream& MemoryOutputStream::operator<<(const char *str) {
        ss << str;
        return *this;
    }

    MemoryOutputStream& MemoryOutputStream::operator<<(const std::string &str) {
        ss << str;
        return *this;
    }

    std::string MemoryOutputStream::ToString() const {
        return ss.str();
    }

}
