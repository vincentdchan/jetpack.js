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
        explicit ModuleCompositor(CodeGenFragment& d, const CodeGenConfig& config):
        thread_pool_(1), d_(d), config_(config) {}

        ModuleCompositor& Append(const CodeGenFragment& fragment);

        void AddSnippet(const std::string& content);

        void Write(const std::string& content);

        void WriteLineEnd();

        std::future<void> DumpSourcemap(Sp<SourceMapGenerator> sg, std::string path);

        inline const CodeGenConfig& Config() const {
            return config_;
        }

    private:
        ThreadPool thread_pool_;
        CodeGenFragment& d_;
        const CodeGenConfig& config_;

    };

}
