//
// Created by Duzhong Chen on 2021/3/31.
//

#include "Benchmark.h"
#include <mutex>
#include <iostream>
#include <fmt/format.h>

namespace jetpack::benchmark {

    static int64_t BENCH_STAT[BenchType::BENCH_END];
    static std::mutex mutex_;

    const char* BenchTypeToCStr(BenchType t) {
        switch (t) {
            case BENCH_PARSING:
                return "Parsing";

            case BENCH_PARSING_STAGE:
                return "Parsing Stage";

            case BENCH_WRITING_IO:
                return "Writing IO";

            case BENCH_MINIFY:
                return "Minify";

            case BENCH_CODEGEN:
                return "Codegen";

            case BENCH_DUMP_SOURCEMAP:
                return "Dump sourcemap";

            case BENCH_MODULE_COMPOSITION:
                return "Module composition";

            case BENCH_FINALIZE_SOURCEMAP:
                return "Finalize sourcemap";

            case BENCH_FINALIZE_SOURCEMAP_2:
                return "Finalize sourcemap 2";

            default:
                return "Unknown";

        }
    }

    void PrintReport() {
        std::lock_guard<std::mutex> guard(mutex_);
        for (int i = 0; i < BENCH_END; i++) {
            std::cerr << fmt::format("{:<24} {}ms\n", BenchTypeToCStr(static_cast<BenchType>(i)), BENCH_STAT[i]);
        }
    }

    void BenchMarker::Submit() {
        int64_t end_time_ = jetpack::time::GetCurrentMs();

        {
            std::lock_guard<std::mutex> guard(mutex_);
            BENCH_STAT[type_] += (end_time_ - start_time_);
        }
    }

}
