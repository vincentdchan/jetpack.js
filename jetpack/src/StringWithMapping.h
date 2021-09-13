//
// Created by Duzhong Chen on 2021/9/8.
//

#pragma once

#include <string_view>
#include <vector>
#include <cinttypes>
#include "utils/MemoryViewOwner.h"
#include "utils/Common.h"

namespace jetpack {

    class Scanner;

    /**
     * Do NOT own the string it self
     * The string could be from a mapped file
     */
    class StringWithMapping {
    public:
        explicit StringWithMapping(Up<MemoryViewOwner> content);

        StringWithMapping(const StringWithMapping&) = delete;
        StringWithMapping(StringWithMapping&&) = delete;

        StringWithMapping& operator=(const StringWithMapping&) = delete;
        StringWithMapping& operator=(StringWithMapping&&) = delete;

        [[nodiscard]]
        inline std::string_view Data() const {
            return data_->View();
        }

        [[nodiscard]]
        inline std::string::size_type size() const {
            return data_->View().size();
        }

        friend class Scanner;

    private:
        Up<MemoryViewOwner> data_;
        // utf8 index -> u16 index
        std::vector<uint32_t> mapping_;

    };

}

