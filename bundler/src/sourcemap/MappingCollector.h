//
// Created by Duzhong Chen on 2021/3/29.
//

#pragma once

#include <cinttypes>
#include "Utils.h"
#include "tokenizer/Location.h"
#include <nlohmann/json.hpp>
#include <tsl/ordered_map.h>

namespace jetpack {
    template<class Key, class T, class Ignore, class Allocator,
            class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>,
            class AllocatorPair = typename std::allocator_traits<Allocator>::template rebind_alloc<std::pair<Key, T>>,
            class ValueTypeContainer = std::vector<std::pair<Key, T>, AllocatorPair>>
    using ordered_map = tsl::ordered_map<Key, T, Hash, KeyEqual, AllocatorPair, ValueTypeContainer>;

    using json = nlohmann::basic_json<ordered_map>;

    class SourceMapGenerator;

    struct MappingItem {
    public:
        inline MappingItem(const UString& n,
                           const SourceLocation& loc,
                           int32_t dL,
                           int32_t dC) noexcept:
                name(n), origin(loc),
                dist_line(dL),
                dist_column(dC) {}

        UString        name;
        SourceLocation origin;
        int32_t        dist_line   = -1;
        int32_t        dist_column = -1;

    };

    class MappingCollector {
    public:
        MappingCollector() noexcept = default;

        inline void push_back(const MappingItem& item) {
            items_.push_back(item);
        }

        inline void EndLine() {
            dist_line_++;
        }

        void AddMapping(const UString& name, const SourceLocation& origin, int32_t column);

        friend class SourceMapGenerator;

    private:
        int32_t          dist_line_ = 0;
        Vec<MappingItem> items_;

    };

}
