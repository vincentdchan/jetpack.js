//
// Created by Vincent Chan on 2021/11/19.
//

#include <fmt/format.h>
#include <iostream>
#include "Dir.h"

namespace jetpack {

    bool Dir::EnsurePath(ghc::filesystem::path& path) {
        if (ghc::filesystem::exists(path)) {
            return true;
        }
        std::error_code ec;
        ghc::filesystem::create_directories(path, ec);
        if (ec) {
            std::cerr << fmt::format("create dir {} failed: {}", path.string(), ec.message()) << std::endl;
            return false;
        }
        return true;
    }

    bool Dir::EnsureParent(const std::string &path) {
        ghc::filesystem::path p(path);
        auto parent = p.parent_path();
        return EnsurePath(parent);
    }

}
