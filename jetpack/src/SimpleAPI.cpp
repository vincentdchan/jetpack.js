//
// Created by Duzhong Chen on 2021/3/30.
//

#ifdef JETPACK_HAS_JEMALLOC
#include <jemalloc/jemalloc.h>
#endif

#include <cxxopts.hpp>
#include "SimpleAPI.h"
#include "utils/JetTime.h"
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
#define OPT_PROFILE_MALLOC "profile-malloc"

namespace jetpack {

    static Sp<MinifyNameGenerator> RenameInnerScopes(Scope &scope, UnresolvedNameCollector* idLogger) {
        std::vector<Sp<MinifyNameGenerator>> temp;
        temp.reserve(scope.children.size());

        for (auto child : scope.children) {
            temp.push_back(RenameInnerScopes(*child, idLogger));
        }

        std::vector<std::tuple<std::string, std::string>> renames;
        auto renamer = MinifyNameGenerator::Merge(temp);

        for (auto& variable : scope.own_variables) {
            auto new_opt = renamer->Next(variable.first);
            if (new_opt.has_value()) {
                renames.emplace_back(variable.first, *new_opt);
            }
        }

        scope.BatchRenameSymbols(renames);

        return renamer;
    }

}

namespace jetpack::simple_api {

    int AnalyzeModule(const std::string& path, JetpackFlags flags, const std::string& basePath) {
        parser::ParserContext::Config parser_config = parser::ParserContext::Config::Default();
        parser_config.jsx = !!(flags & JetpackFlag::Jsx);

        // do not release memory
        // it will save your time
        auto resolver = std::shared_ptr<ModuleResolver>(new ModuleResolver, [](void*) {});

        try {
            bool trace_file = !!(flags & JetpackFlag::TraceFile);
            resolver->SetTraceFile(trace_file);
            resolver->BeginFromEntry(parser_config, path, basePath);
            resolver->PrintStatistic();
            return 0;
        } catch (ModuleResolveException& err) {
            err.PrintToStdErr();
            return 3;
        }
    }

    int BundleModule(const std::string& path, const std::string& out_path, JetpackFlags flags, const std::string& basePath) {

        auto start = time::GetCurrentMs();

        try {
            auto resolver = std::shared_ptr<ModuleResolver>(new ModuleResolver, [](void*) {});
            CodeGen::Config codegen_config;
            parser::ParserContext::Config parser_config = parser::ParserContext::Config::Default();

            if (flags & JetpackFlag::Jsx) {
                parser_config.jsx = true;
                parser_config.transpile_jsx = true;
            }

            if (flags & JetpackFlag::Minify) {
                parser_config.constant_folding = true;
                codegen_config.minify = true;
                codegen_config.comments = false;
                resolver->SetNameGenerator(MinifyNameGenerator::Make());
            }

            codegen_config.sourcemap = !!(flags & JetpackFlag::Sourcemap);

            resolver->SetTraceFile(true);
            resolver->BeginFromEntry(parser_config, path, basePath);
            resolver->CodeGenAllModules(codegen_config, out_path);

            std::cout << "Finished." << std::endl;
            std::cout << "Totally " << resolver->ModCount() << " file(s) in " << jetpack::time::GetCurrentMs() - start << " ms." << std::endl;

            if (flags & JetpackFlag::Profile) {
                benchmark::PrintReport();
            }

#ifdef JETPACK_HAS_JEMALLOC
            if (flags & JetpackFlag::ProfileMalloc) {
                malloc_stats_print(NULL, NULL, NULL);

            }
#endif

            return 0;
        } catch (ModuleResolveException& err) {
            err.PrintToStdErr();
            return 3;
        }
    }

    std::string ParseAndCodeGen(std::string&& content, const jetpack::parser::ParserContext::Config& config, const CodeGen::Config& code_gen_config) {
        auto ctx = std::make_shared<jetpack::parser::ParserContext>(-1, std::move(content), config);
        jetpack::parser::Parser parser(ctx);

        auto mod = parser.ParseModule();
        mod->scope->ResolveAllSymbols(nullptr);

        if (code_gen_config.minify) {
            std::vector<Scope::PVar> variables;
            for (auto& tuple : mod->scope->own_variables) {
                variables.push_back(tuple.second);
            }

            std::sort(std::begin(variables), std::end(variables), [] (const Scope::PVar& p1, const Scope::PVar& p2) {
                return p1->identifiers.size() > p2->identifiers.size();
            });

            std::vector<Sp<MinifyNameGenerator>> result;
            for (auto child : mod->scope->children) {
                result.push_back(RenameInnerScopes(*child, nullptr));
            }

            auto name_generator = MinifyNameGenerator::Merge(result);

            // RenameSymbol() will change iterator, call it later
            std::vector<std::tuple<std::string, std::string>> rename_vec;

            // Distribute new name to root level variables
            for (auto& var : variables) {
                auto new_name_opt = name_generator->Next(var->name);

                if (new_name_opt.has_value()) {
                    rename_vec.emplace_back(var->name, *new_name_opt);
                }
            }

            mod->scope->BatchRenameSymbols(rename_vec);
        }

        CodeGen codegen(code_gen_config, nullptr);
        codegen.Traverse(mod);
        return codegen.GetResult().content;
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
                    (OPT_PROFILE_MALLOC, "print profile of malloc")
                    ;

            options.parse_positional(OPT_ENTRY);

            JetpackFlags flags;
            auto result = options.parse(argc, argv);
            flags |= JetpackFlag::TraceFile;

            // print help message
            if (result[OPT_HELP].count()) {
                std::cout << options.help() << std::endl;
                return !result[OPT_HELP].count();
            }

            if (result[OPT_NO_TRACE].count()) {
                flags.setFlag(JetpackFlag::TraceFile, false);
            }

            if (result[OPT_MINIFY].count()) {
                flags |= JetpackFlag::Minify;
            }

            if (result[OPT_JSX].count()) {
                flags |= JetpackFlag::Jsx;
            }

            if (result[OPT_LIBRARY].count()) {
                flags |= JetpackFlag::Library;
            }

            if (result[OPT_SOURCEMAP].count()) {
                flags |= JetpackFlag::Library;
            }

            if (result[OPT_PROFILE].count()) {
                flags |= JetpackFlag::Profile;
            }

            if (result[OPT_PROFILE_MALLOC].count()) {
                flags |= JetpackFlag::ProfileMalloc;
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
