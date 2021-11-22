//
// Created by Duzhong Chen on 2021/3/30.
//

#pragma once

#include <cinttypes>
#include <ThreadPool.h>
#include "utils/io/FileIO.h"
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
        explicit ModuleCompositor(io::Writer& writer, const CodeGenConfig& config):
        thread_pool_(1), writer_(writer), config_(config) {}

        ModuleCompositor& Append(const CodeGenFragment& fragment);

        void AddSnippet(const std::string& content);

        void Write(const std::string& content);

        void WriteLineEnd();

        void DumpSources(Sp<SourceMapGenerator> sg);

        std::future<void> DumpSourcemap(Sp<SourceMapGenerator> sg);

        inline const CodeGenConfig& Config() const {
            return config_;
        }

    private:
        ThreadPool thread_pool_;
        io::Writer& writer_;
        int32_t     line_ = 1;
        int32_t     column_ = 0;
        std::vector<MappingItem> mapping_items_;
        const CodeGenConfig& config_;

    };

}
