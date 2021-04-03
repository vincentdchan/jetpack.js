//
// Created by Duzhong Chen on 2021/3/31.
//

#include "Benchmark.h"
#include <mutex>
#include <iostream>

namespace jetpack::benchmark {

    static int64_t BENCH_STAT[BenchType::BENCH_END];
    static std::mutex mutex_;

    const char* BenchTypeToCStr(BenchType t) {
        switch (t) {
            case BENCH_PARSING:
                return "parsing";

            case BENCH_READING_IO:
                return "reading io";

            case BENCH_WRITING_IO:
                return "writing io";

            case BENCH_MINIFY:
                return "minify";

            case BENCH_CODEGEN:
                return "codegen";

            case BENCH_DUMP_SOURCEMAP:
                return "dump sourcemap";

            case BENCH_MODULE_COMPOSITION:
                return "module composition";

            case BENCH_FINALIZE_SOURCEMAP:
                return "finalize sourcemap";

            default:
                return "unknown";

        }
    }

    void PrintReport() {
        std::lock_guard<std::mutex> guard(mutex_);
        for (int i = 0; i < BENCH_END; i++) {
            std::cout << BenchTypeToCStr(static_cast<BenchType>(i)) << ":\t" << BENCH_STAT[i] << "ms" << std::endl;
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
