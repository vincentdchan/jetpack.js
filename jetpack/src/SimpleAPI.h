//
// Created by Duzhong Chen on 2021/3/30.
//

#pragma once

#include <string>

namespace jetpack::simple_api {

    int AnalyzeModule(const std::string& path,
                      bool jsx,
                      bool trace_file);

    int BundleModule(bool jsx,
                     bool minify,
                     bool library,
                     bool sourcemap,
                     const std::string& path,
                     const std::string& out_path);

}
