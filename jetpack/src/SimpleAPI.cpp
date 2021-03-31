//
// Created by Duzhong Chen on 2021/3/30.
//

#include "SimpleAPI.h"
#include "JetTime.h"
#include "ModuleResolver.h"
#include "Benchmark.h"

namespace jetpack::simple_api {

    int AnalyzeModule(const std::string& path,
                             bool jsx,
                             bool trace_file) {
        parser::ParserContext::Config parser_config = parser::ParserContext::Config::Default();
        if (jsx) {
            parser_config.jsx = true;
        }

        // do not release memory
        // it will save your time
        auto resolver = std::shared_ptr<ModuleResolver>(new ModuleResolver, [](void*) {});

        try {
            resolver->SetTraceFile(trace_file);
            resolver->BeginFromEntry(parser_config, path);
            resolver->PrintStatistic();
            return 0;
        } catch (ModuleResolveException& err) {
            err.PrintToStdErr();
            return 3;
        }
    }

    int BundleModule(bool jsx,
                     bool minify,
                     bool library,
                     bool sourcemap,
                     const std::string& path,
                     const std::string& out_path) {

        auto start = time::GetCurrentMs();

        try {
            auto resolver = std::shared_ptr<ModuleResolver>(new ModuleResolver, [](void*) {});
            CodeGen::Config codegen_config;
            parser::ParserContext::Config parser_config = parser::ParserContext::Config::Default();

            if (jsx) {
                parser_config.jsx = true;
                parser_config.transpile_jsx = true;
            }

            if (minify) {
                parser_config.constant_folding = true;
                codegen_config.minify = true;
                codegen_config.comments = false;
                resolver->SetNameGenerator(MinifyNameGenerator::Make());
            }

            codegen_config.sourcemap = sourcemap;

            resolver->SetTraceFile(true);
            resolver->BeginFromEntry(parser_config, path);
            resolver->CodeGenAllModules(codegen_config, out_path);

            std::cout << "Finished." << std::endl;
            std::cout << "Totally " << resolver->ModCount() << " file(s) in " << jetpack::time::GetCurrentMs() - start << " ms." << std::endl;

            benchmark::PrintReport();
            return 0;
        } catch (ModuleResolveException& err) {
            err.PrintToStdErr();
            return 3;
        }
    }

}
