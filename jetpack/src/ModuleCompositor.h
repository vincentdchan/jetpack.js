//
// Created by Duzhong Chen on 2021/3/30.
//

#pragma once

#include <cinttypes>
#include <ThreadPool.h>
#include "string/UString.h"
#include "sourcemap/MappingCollector.h"
#include "sourcemap/SourceMapGenerator.h"

namespace jetpack {

    /**
     * concat modules and re-mapping symbols
     */
    class ModuleCompositor {
    public:
        explicit ModuleCompositor(SourceMapGenerator& sg): sourceMapGenerator(sg) {}

        ModuleCompositor& append(
                const UString& content,
                Sp<MappingCollector> mappingCollector
                );

        [[nodiscard]]
        inline UString Finalize(ThreadPool& threadPool) const {
            sourceMapGenerator.Finalize(threadPool);
            return result;
        }

    private:
        SourceMapGenerator& sourceMapGenerator;
        UString result;
        uint32_t line = 0;  // start from 0
        uint32_t column = 0;

    };

}
