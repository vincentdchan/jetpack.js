//
// Created by Duzhong Chen on 2019/10/12.
//
#pragma once
#include <ThreadPool.h>

class lazy_thread_pool {
public:
    static ThreadPool* get();

private:
    static ThreadPool* pool_;

};
