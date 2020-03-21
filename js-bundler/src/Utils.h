//
// Created by Duzhong Chen on 2020/3/20.
//

#pragma once

#ifndef _WIN32
#include <sys/stat.h>
#endif

namespace rocket_bundle {

    inline bool IsFileExist(const std::string& path) {
#ifndef _WIN32
        struct stat st;
        return stat(path.c_str(), &st) >= 0;
#else
        return false;
#endif
    }

}
