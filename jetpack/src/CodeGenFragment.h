//
// Created by Vincent Chan on 2021/11/12.
//

#pragma once

#include <vector>
#include <string>
#include "tokenizer/Location.h"

namespace jetpack {

    struct MappingItem {
    public:
        inline MappingItem(const std::string& n,
                           const SourceLocation& loc,
                           int32_t dL,
                           int32_t dC) noexcept:
                name(n), origin(loc),
                dist_line(dL),
                dist_column(dC) {}

        std::string        name;
        SourceLocation origin;
        int32_t        dist_line   = -1;
        int32_t        dist_column = -1;

    };

    class CodeGenFragment {
    public:
        std::string content;
        int32_t     line = 1;
        int32_t     column = 0;
        std::vector<MappingItem> mapping_items;

    };

}
