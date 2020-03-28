//
// Created by Duzhong Chen on 2020/3/20.
//

#include <iostream>
#include <cxxopts.hpp>

#ifndef _WIN32
#include <jemalloc/jemalloc.h>
#endif

#include "Path.h"
#include "ModuleResolver.h"

#define OPT_HELP "help"
#define OPT_ENTRY "entry"
#define OPT_TOLERANT "tolerant"
#define OPT_ES_MODULE "es-module"
#define OPT_JSX "jsx"
#define OPT_ANALYZE_MODULE "analyze-module"
#define OPT_NO_TRACE "no-trace"
#define OPT_OUT "out"

using namespace rocket_bundle;

static int AnalyzeModule(const std::string& self_path, const std::string& path, bool trace_file);
static int BundleModule(const std::string& self_path, const std::string& path, const std::string& out_path);

int main(int argc, char** argv) {
    try {
        cxxopts::Options options("rocket-bundle", "Rocket Bundle command line");
        options.add_options()
                (OPT_ENTRY, "entry file to parse", cxxopts::value<std::string>())
                (OPT_TOLERANT, "tolerant parsing error")
                (OPT_ES_MODULE, "parsing as ES module")
                (OPT_JSX, "support jsx syntax")
                (OPT_HELP, "produce help message")
                (OPT_ANALYZE_MODULE, "analyze a module and print result", cxxopts::value<std::string>())
                (OPT_NO_TRACE, "do not trace ref file when analyze module")
                (OPT_OUT, "output filename of bundle", cxxopts::value<std::string>())
                ;

        options.parse_positional(OPT_ENTRY);

        auto result = options.parse(argc, argv);
        bool trace_file = true;

        // print help message
        if (result[OPT_HELP].count()) {
            std::cout << options.help() << std::endl;
            return !result[OPT_HELP].count();
        }

        if (result[OPT_NO_TRACE].count()) {
            trace_file = false;
        }

        if (result[OPT_ANALYZE_MODULE].count()) {
            std::string path = result[OPT_ANALYZE_MODULE].as<std::string>();
            return AnalyzeModule(argv[0], path, trace_file);
        }

        if (result[OPT_OUT].count()) {
            std::string entry_path = result[OPT_ENTRY].as<std::string>();
            std::string out_path = result[OPT_OUT].as<std::string>();

            return BundleModule(argv[0], entry_path, out_path);
        }

        std::cout << options.help() << std::endl;
        return 0;
    } catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 3;
    }
}

static int AnalyzeModule(const std::string& self_path_str, const std::string& path, bool trace_file) {
    Path self_path(self_path_str);
    self_path.Pop();

    // do not release memory
    // it will save your time
    auto resolver = std::shared_ptr<ModuleResolver>(new ModuleResolver, [](void*) {});

    try {
        resolver->SetTraceFile(trace_file);
        resolver->BeginFromEntry(self_path.ToString(), path);
        resolver->PrintStatistic();
        return 0;
    } catch (ModuleResolveException& err) {
        err.PrintToStdErr();
        return 3;
    }
}

static int BundleModule(const std::string& self_path_str, const std::string& path, const std::string& out_path) {
    Path self_path(self_path_str);
    self_path.Pop();

    try {
        auto resolver = std::shared_ptr<ModuleResolver>(new ModuleResolver, [](void*) {});
        resolver->SetTraceFile(true);
        resolver->BeginFromEntry(self_path.ToString(), path);
        resolver->CodeGenAllModules(out_path);
        return 0;
    } catch (ModuleResolveException& err) {
        err.PrintToStdErr();
        return 3;
    }
}
