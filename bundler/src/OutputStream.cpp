//
// Created by Duzhong Chen on 2020/7/14.
//

#include "./OutputStream.h"

namespace jetpack {
    using std::uint32_t;

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

    MemoryOutputStream::MemoryOutputStream() {
        data_ = reinterpret_cast<char16_t*>(malloc(capacity_ * sizeof(char16_t)));
    }

    OutputStream& MemoryOutputStream::operator<<(const char16_t *str) {
        uint32_t size = 0;
        while (str[size] != 0) {
            size++;
        }
        return Write(str, size);
    }

    OutputStream& MemoryOutputStream::Write(const char16_t *str, std::uint32_t size) {
        uint32_t expected_size = size_ + size;
        if (expected_size > capacity_) {
            uint32_t expected_cap = capacity_;
            while (expected_cap <= expected_size) {
                expected_cap *= 2;
            }
            data_ = reinterpret_cast<char16_t*>(realloc(data_, expected_cap * sizeof(char16_t )));
            capacity_ = expected_cap;
        }
        // copy buffer
        for (uint32_t i = 0; i < size; i++) {
            data_[i + size_] = str[i];
        }
        size_ = expected_size;
        return *this;
    }

    OutputStream& MemoryOutputStream::operator<<(const UString &str) {
        return Write(str.c_str(), str.size());
    }

    OutputStream& MemoryOutputStream::operator<<(char ch) {
        char16_t buffer[2] = { 0, 0 };
        buffer[0] = ch;
        return Write(buffer, 1);
    }

    std::u16string MemoryOutputStream::ToString() const {
        return std::u16string(data_, size_);
    }

    MemoryOutputStream::~MemoryOutputStream() {
        free(data_);
    }

}
