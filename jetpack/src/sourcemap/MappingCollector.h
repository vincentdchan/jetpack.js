//
// Created by Duzhong Chen on 2021/3/29.
//

#pragma once

#include <cinttypes>
#include <nlohmann/json.hpp>
#include <tsl/ordered_map.h>
#include "CodeGenFragment.h"
#include "utils/Common.h"

namespace jetpack {
    template<class Key, class T, class Ignore, class Allocator,
            class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>,
            class AllocatorPair = typename std::allocator_traits<Allocator>::template rebind_alloc<std::pair<Key, T>>,
            class ValueTypeContainer = std::vector<std::pair<Key, T>, AllocatorPair>>
    using ordered_map = tsl::ordered_map<Key, T, Hash, KeyEqual, AllocatorPair, ValueTypeContainer>;

    using json = nlohmann::basic_json<ordered_map>;

    class SourceMapGenerator;
    class ModuleCompositor;

    class MappingCollector {
    public:
        MappingCollector(CodeGenFragment& codegen_fragment):
        codegen_fragment_(codegen_fragment) {}

        inline void push_back(const MappingItem& item) {
            codegen_fragment_.mapping_items.push_back(item);
        }

        inline void push_back(MappingItem&& item) {
            codegen_fragment_.mapping_items.push_back(std::move(item));
        }

        inline void EndLine() {
            dist_line_++;
        }

        void AddMapping(const std::string& name, const SourceLocation& origin, int32_t column);

        friend class SourceMapGenerator;
        friend class ModuleCompositor;

    private:
        int32_t          dist_line_ = 1;
        CodeGenFragment& codegen_fragment_;

    };

}
