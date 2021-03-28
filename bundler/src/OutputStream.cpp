//
// Created by Duzhong Chen on 2020/7/14.
//

#include "OutputStream.h"
#include "io/FileIO.h"

namespace jetpack {
    using std::uint32_t;

    MemoryOutputStream::MemoryOutputStream() {
    }

    OutputStream& MemoryOutputStream::operator<<(const char16_t *str) {
        uint32_t size = 0;
        while (str[size] != 0) {
            size++;
        }
        return Write(str, size);
    }

    OutputStream& MemoryOutputStream::Write(const char16_t *str, std::uint32_t size) {
        data_.append(str, size);
        return *this;
    }

    OutputStream& MemoryOutputStream::operator<<(const UString &str) {
        data_.append(str);
        return *this;
    }

    OutputStream& MemoryOutputStream::operator<<(char ch) {
        char16_t buffer[2] = { 0, 0 };
        buffer[0] = ch;
        return Write(buffer, 1);
    }

    UString MemoryOutputStream::ToString() const {
        return data_;
    }

    std::string MemoryOutputStream::ToUTF8() const {
        return ToString().toStdString();
    }

    MemoryOutputStream::~MemoryOutputStream() {
    }

}
