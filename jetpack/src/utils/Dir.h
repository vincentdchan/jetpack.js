//
// Created by Vincent Chan  on 2021/11/19.
//

#pragma once

#include <filesystem.hpp>
#include "Path.h"

namespace jetpack {

    class Dir {
    public:

        static bool EnsureParent(const std::string& path);

        static bool EnsurePath(ghc::filesystem::path& path);

    };

}
