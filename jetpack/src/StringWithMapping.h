//
// Created by Duzhong Chen on 2021/9/8.
//

#pragma once

#include <string>
#include <vector>
#include <cinttypes>
#include "utils/Common.h"

namespace jetpack {

    class Scanner;

    class StringWithMapping {
    public:
        static Sp<StringWithMapping> Make(std::string&& content);

        StringWithMapping(std::string&& content);

        StringWithMapping(const StringWithMapping&) = delete;
        StringWithMapping(StringWithMapping&&) = delete;

        StringWithMapping& operator=(const StringWithMapping&) = delete;
        StringWithMapping& operator=(StringWithMapping&&) = delete;

        [[nodiscard]]
        inline const std::string& ConstData() const {
            return data_;
        }

        inline std::string& Data() {
            return data_;
        }

        [[nodiscard]]
        inline std::string::size_type size() const {
            return data_.size();
        }

        friend class Scanner;

    private:
        std::string data_;
        // utf8 index -> u16 index
        std::vector<uint32_t> mapping_;

    };

}
