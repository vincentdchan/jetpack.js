//
// Created by Duzhong Chen on 2021/3/30.
//

#include <cxxopts.hpp>
#include <jemalloc/jemalloc.h>
#include "SimpleAPI.h"
#include "JetTime.h"
#include "ModuleResolver.h"
#include "Benchmark.h"

#define OPT_HELP "help"
#define OPT_ENTRY "entry"
#define OPT_TOLERANT "tolerant"
#define OPT_JSX "jsx"
#define OPT_LIBRARY "library"
#define OPT_ANALYZE_MODULE "analyze-module"
#define OPT_NO_TRACE "no-trace"
#define OPT_MINIFY "minify"
#define OPT_OUT "out"
#define OPT_SOURCEMAP "sourcemap"
#define OPT_PROFILE "profile"

namespace jetpack::simple_api {

    int AnalyzeModule(const std::string& path, Flags flags) {
        parser::ParserContext::Config parser_config = parser::ParserContext::Config::Default();
        if (flags.isJsx()) {
            parser_config.jsx = true;
        }

        // do not release memory
        // it will save your time
        auto resolver = std::shared_ptr<ModuleResolver>(new ModuleResolver, [](void*) {});

        try {
            resolver->SetTraceFile(flags.isTraceFile());
            resolver->BeginFromEntry(parser_config, path);
            resolver->PrintStatistic();
            return 0;
        } catch (ModuleResolveException& err) {
            err.PrintToStdErr();
            return 3;
        }
    }

    int BundleModule(const std::string& path, const std::string& out_path, Flags flags) {

        auto start = time::GetCurrentMs();

        try {
            auto resolver = std::shared_ptr<ModuleResolver>(new ModuleResolver, [](void*) {});
            CodeGen::Config codegen_config;
            parser::ParserContext::Config parser_config = parser::ParserContext::Config::Default();

            if (flags.isJsx()) {
                parser_config.jsx = true;
                parser_config.transpile_jsx = true;
            }

            if (flags.isMinify()) {
                parser_config.constant_folding = true;
                codegen_config.minify = true;
                codegen_config.comments = false;
                resolver->SetNameGenerator(MinifyNameGenerator::Make());
            }

            codegen_config.sourcemap = flags.isSourcemap();

            resolver->SetTraceFile(true);
            resolver->BeginFromEntry(parser_config, path);
            resolver->CodeGenAllModules(codegen_config, out_path);

            std::cout << "Finished." << std::endl;
            std::cout << "Totally " << resolver->ModCount() << " file(s) in " << jetpack::time::GetCurrentMs() - start << " ms." << std::endl;

            if (flags.isProfile()) {
                benchmark::PrintReport();
                malloc_stats_print(NULL, NULL, NULL);
            }
            return 0;
        } catch (ModuleResolveException& err) {
            err.PrintToStdErr();
            return 3;
        }
    }

    int HandleCommandLine(int argc, char** argv) {
        try {
            cxxopts::Options options("Jetpack++", "Jetpack++ command line");
            options.add_options()
                    (OPT_ENTRY, "entry file to parse", cxxopts::value<std::string>())
                    (OPT_TOLERANT, "tolerant parsing error")
                    (OPT_JSX, "support jsx syntax")
                    (OPT_LIBRARY, "bundle as library, do not bundle node_modules")
                    (OPT_HELP, "produce help message")
                    (OPT_ANALYZE_MODULE, "analyze a module and print result", cxxopts::value<std::string>())
                    (OPT_NO_TRACE, "do not trace ref file when analyze module")
                    (OPT_MINIFY, "minify the code")
                    (OPT_OUT, "output filename of bundle", cxxopts::value<std::string>())
                    (OPT_SOURCEMAP, "generate sourcemaps")
                    (OPT_PROFILE, "print profile information")
                    ;

            options.parse_positional(OPT_ENTRY);

            simple_api::Flags flags;
            auto result = options.parse(argc, argv);
            flags.setTraceFile(true);

            // print help message
            if (result[OPT_HELP].count()) {
                std::cout << options.help() << std::endl;
                return !result[OPT_HELP].count();
            }

            if (result[OPT_NO_TRACE].count()) {
                flags.setTraceFile(false);
            }

            if (result[OPT_MINIFY].count()) {
                flags.setMinify(true);
            }

            if (result[OPT_JSX].count()) {
                flags.setJsx(true);
            }

            if (result[OPT_LIBRARY].count()) {
                flags.setLibrary(true);
            }

            if (result[OPT_SOURCEMAP].count()) {
                flags.setSourcemap(true);
            }

            if (result[OPT_PROFILE].count()) {
                flags.setProfile(true);
            }

            if (result[OPT_ANALYZE_MODULE].count()) {
                std::string path = result[OPT_ANALYZE_MODULE].as<std::string>();
                return simple_api::AnalyzeModule(path, flags);
            }

            if (result[OPT_OUT].count()) {
                std::string entry_path = result[OPT_ENTRY].as<std::string>();
                std::string out_path = result[OPT_OUT].as<std::string>();

                return simple_api::BundleModule(entry_path, out_path, flags);
            }

            std::cout << options.help() << std::endl;
            return 0;
        } catch (std::exception& ex) {
            std::cerr << ex.what() << std::endl;
            return 3;
        }
    }

}
