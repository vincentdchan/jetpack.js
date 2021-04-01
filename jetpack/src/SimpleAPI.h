//
// Created by Duzhong Chen on 2021/3/30.
//

#pragma once

#include <string>
#include <cstdlib>

namespace jetpack::simple_api {

    struct Flags {
    public:
        Flags() {
            ::memset(this, 0, sizeof(Flags));
            setTraceFile(true);
        }

        bool isTraceFile() const {
            return trace_file;
        }

        void setTraceFile(bool traceFile) {
            trace_file = traceFile;
        }

        bool isMinify() const {
            return minify;
        }

        void setMinify(bool minify) {
            Flags::minify = minify;
        }

        bool isJsx() const {
            return jsx;
        }

        void setJsx(bool jsx) {
            Flags::jsx = jsx;
        }

        bool isLibrary() const {
            return library;
        }

        void setLibrary(bool library) {
            Flags::library = library;
        }

        bool isSourcemap() const {
            return sourcemap;
        }

        void setSourcemap(bool sourcemap) {
            Flags::sourcemap = sourcemap;
        }

        bool isProfile() const {
            return profile;
        }

        void setProfile(bool profile) {
            Flags::profile = profile;
        }

    private:
        bool trace_file : 1;
        bool minify     : 1;
        bool jsx        : 1;
        bool library    : 1;
        bool sourcemap  : 1;
        bool profile    : 1;
    };

    int AnalyzeModule(const std::string& path, Flags flags);

    int BundleModule(const std::string& path,
                     const std::string& out_path, Flags flags);

}
