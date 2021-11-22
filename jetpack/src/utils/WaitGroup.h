//
// Created by Vincent Chan on 2021/11/22.
//

#pragma once

#include <mutex>
#include <atomic>

class WaitGroup {
public:
    inline void Add(int incr = 1) { counter += incr; }
    inline void Done() {
        if (--counter <= 0) {
            cond.notify_all();
        }
    }
    inline void Wait() {
        std::unique_lock<std::mutex> lock(mutex);
        cond.wait(lock, [&] { return counter <= 0; });
    }

private:
    std::mutex mutex;
    std::atomic<int> counter{0};
    std::condition_variable cond;
};
