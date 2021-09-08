//
// Created by Duzhong Chen on 2021/3/28.
//

#ifndef ROCKET_BUNDLE_JETTIME_H
#define ROCKET_BUNDLE_JETTIME_H

#include <cinttypes>
#include <chrono>

namespace jetpack::time {

    inline int64_t GetCurrentMs() {
        using namespace std::chrono;
        milliseconds ms = duration_cast< milliseconds >(
                system_clock::now().time_since_epoch()
        );
        return ms.count();
    }

}

#endif //ROCKET_BUNDLE_JETTIME_H
