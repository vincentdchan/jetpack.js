//
// Created by Vincent Chan on 2021/11/12.
//

#pragma once

#include <string>
#include <cstring>

namespace jetpack {

    template <typename T>
    class Slice {
    public:
        constexpr Slice(T* data, size_t len): data_(data), size_(len) {}

        constexpr T* begin() { return data_; }
        constexpr T* end() { return data_ + size_; }

        constexpr T* data() { return data_; }
        constexpr const T* data() const { return data_; }
        constexpr size_t size() const { return size_; }

        constexpr bool empty() const { return size_ == 0; }

    private:
        T* data_;
        size_t size_;

    };

    inline Slice<const char> make_slice(const std::string& str) {
        return Slice(str.c_str(), str.size());
    }

    template <typename T>
    inline Slice<const T> make_slice(const std::vector<T>& d) {
        return Slice(d.data(), d.size());
    }

#define STR(CONTENT) Slice<const char>(CONTENT, strlen(CONTENT))

}
