//
// Created by Duzhong Chen on 2019/10/12.
//
#include "lazy_thread_pool.h"
#include <jemalloc/jemalloc.h>
#include <thread>

ThreadPool* lazy_thread_pool::get() {
    if (pool_ == nullptr) {
        unsigned int nthreads = std::thread::hardware_concurrency();
        void* space = malloc(sizeof(ThreadPool));
        pool_ = new(space) ThreadPool(nthreads);
    }

    return pool_;
}

ThreadPool* lazy_thread_pool::pool_ = nullptr;
