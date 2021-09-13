//
// Created by Duzhong Chen on 2021/3/31.
//

#ifndef ROCKET_BUNDLE_PROFILE_H
#define ROCKET_BUNDLE_PROFILE_H

#include "utils/JetTime.h"

namespace jetpack::benchmark {

    enum BenchType {
        BENCH_PARSING = 0,
        BENCH_PARSING_STAGE,
        BENCH_WRITING_IO,
        BENCH_MINIFY,
        BENCH_CODEGEN,
        BENCH_DUMP_SOURCEMAP,
        BENCH_MODULE_COMPOSITION,
        BENCH_FINALIZE_SOURCEMAP,
        BENCH_END,
    };

    const char* BenchTypeToCStr(BenchType t);

    void PrintReport();

    struct BenchMarker {
    public:
        inline explicit  BenchMarker(BenchType t) noexcept: type_(t) {
            start_time_ = jetpack::time::GetCurrentMs();
        }

        void Submit();

    private:
        BenchType type_;
        int64_t start_time_;

    };

}

#endif //ROCKET_BUNDLE_PROFILE_H
