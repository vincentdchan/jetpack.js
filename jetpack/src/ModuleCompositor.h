//
// Created by Duzhong Chen on 2021/3/30.
//

#pragma once

#include <cinttypes>
#include <ThreadPool.h>
#include "utils/string/UString.h"
#include "codegen/CodeGenConfig.h"
#include "CodeGenFragment.h"
#include "sourcemap/MappingCollector.h"
#include "sourcemap/SourceMapGenerator.h"

namespace jetpack {

    /**
     * concat modules and re-mapping symbols
     */
    class ModuleCompositor {
    public:
        explicit ModuleCompositor(const CodeGenConfig& config):
        config_(config) {}

        ModuleCompositor& Append(const CodeGenFragment& fragment);

        void AddSnippet(const std::string& content);

        void Write(const std::string& content);

        void WriteLineEnd();

        inline void Take(std::string& out) {
            result_.swap(out);
        }

        constexpr Slice<const MappingItem> MappingItems() const {
            return Slice(mapping_items_.data(), mapping_items_.size());
        }

    private:
        std::vector<MappingItem> mapping_items_;
        const CodeGenConfig& config_;
        std::string result_;
        uint32_t line_ = 0;  // start from 0
        uint32_t column_ = 0;

    };

}
