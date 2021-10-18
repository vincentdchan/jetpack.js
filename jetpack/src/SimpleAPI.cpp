//
// Created by Duzhong Chen on 2021/3/30.
//

#include <cxxopts.hpp>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

#include "SimpleAPI.h"
#include "utils/JetTime.h"
#include "ModuleResolver.h"
#include "Benchmark.h"
#include "dumper/AstToJson.h"

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

using namespace jetpack;

#define ERROR_BUFFER_SIZE 4096

static char error_buffer[ERROR_BUFFER_SIZE];

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

EMSCRIPTEN_KEEPALIVE
int jetpack_analyze_module(const char *path, JetpackFlags flags, const char *basePath) {
    parser::Config parser_config = parser::Config::Default();
    parser_config.jsx = !!(flags & JETPACK_JSX);

    // do not release memory
    // it will save your time
    auto resolver = std::shared_ptr<ModuleResolver>(new ModuleResolver, [](void *) {});

    try {
        bool trace_file = !!(flags & JETPACK_TRACE_FILE);
        resolver->SetTraceFile(trace_file);
        resolver->BeginFromEntry(parser_config, path, basePath);
        resolver->PrintStatistic();
        return 0;
    } catch (ModuleResolveException &err) {
        err.PrintToStdErr();
        return 3;
    }
}

EMSCRIPTEN_KEEPALIVE
int jetpack_bundle_module(const char *path, const char *out_path, int flags, const char *base_path_c) {
    auto start = time::GetCurrentMs();
    std::string base_path;
    if (base_path_c) {
        base_path = base_path_c;
    }

    try {
        auto resolver = std::shared_ptr<ModuleResolver>(new ModuleResolver, [](void *) {});
        CodeGenConfig codegen_config;
        parser::Config parser_config = parser::Config::Default();

        if (flags & JETPACK_JSX) {
            parser_config.jsx = true;
            parser_config.transpile_jsx = true;
        }

        if (flags & JETPACK_MINIFY) {
            parser_config.constant_folding = true;
            codegen_config.minify = true;
            codegen_config.comments = false;
            resolver->SetNameGenerator(MinifyNameGenerator::Make());
        }

        codegen_config.sourcemap = !!(flags & JETPACK_SOURCEMAP);

        resolver->SetTraceFile(!!(flags & JETPACK_TRACE_FILE));
        resolver->BeginFromEntry(parser_config, path, base_path);
        resolver->CodeGenAllModules(codegen_config, out_path);

        std::cout << "Finished." << std::endl;
        std::cout << "Totally " << resolver->ModCount() << " file(s) in " << jetpack::time::GetCurrentMs() - start
                  << " ms." << std::endl;

        if (flags & JETPACK_PROFILE) {
            benchmark::PrintReport();
        }

        return 0;
    } catch (ModuleResolveException &err) {
        err.PrintToStdErr();
        return 3;
    }
}

char* jetpack_parse_and_codegen_will_throw(const char* content, int flags) {
    error_buffer[0] = 0;

    parser::Config config = parser::Config::Default();
    CodeGenConfig code_gen_config;

    config.jsx = !!(flags & JETPACK_JSX);
    config.constant_folding = !!(flags & JETPACK_CONSTANT_FOLDING);

    code_gen_config.minify = !!(flags & JETPACK_MINIFY);

    AstContext ast_context;
    parser::Parser parser(ast_context, content, config);

    auto mod = parser.ParseModule();
    mod->scope->ResolveAllSymbols(nullptr);

    if (code_gen_config.minify) {
        std::vector<Scope::PVar> variables;
        for (auto &tuple : mod->scope->own_variables) {
            variables.push_back(tuple.second);
        }

        std::sort(std::begin(variables), std::end(variables), [](const Scope::PVar &p1, const Scope::PVar &p2) {
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
        for (auto &var : variables) {
            auto new_name_opt = name_generator->Next(var->name);

            if (new_name_opt.has_value()) {
                rename_vec.emplace_back(var->name, *new_name_opt);
            }
        }

        mod->scope->BatchRenameSymbols(rename_vec);
    }

    CodeGen codegen(code_gen_config, nullptr);
    codegen.Traverse(*mod);
    auto result = codegen.GetResult();
    char* str_result = reinterpret_cast<char*>(::malloc(result.content.size() + 1));
    ::memcpy(str_result, result.content.c_str(), result.content.size());
    str_result[result.content.size()] = 0;
    return str_result;
}

EMSCRIPTEN_KEEPALIVE
char* jetpack_parse_and_codegen(const char* content, int flags) {
    try {
        return jetpack_parse_and_codegen_will_throw(content, flags);
    } catch (jetpack::parser::ParseError& err) {
        std::string err_msg = err.ErrorMessage();
        snprintf(error_buffer, ERROR_BUFFER_SIZE, "%s", err_msg.c_str());
    } catch (...) {
        snprintf(error_buffer, ERROR_BUFFER_SIZE, "unknown error");
    }
    return nullptr;
}

EMSCRIPTEN_KEEPALIVE
char* jetpack_parse_to_ast(const char* str, int flags) {
    error_buffer[0] = 0;
    parser::Config config = parser::Config::Default();
    config.jsx = !!(flags & JETPACK_JSX);
    config.constant_folding = !!(flags & JETPACK_CONSTANT_FOLDING);
    AstContext ctx;
    parser::Parser parser(ctx, str, config);

    std::string json_str;

    try {
        auto mod = parser.ParseModule();
        auto json = jetpack::dumper::AstToJson::Dump(mod);

        json_str = json.dump();
    } catch (jetpack::parser::ParseError& err) {
        std::string err_msg = err.ErrorMessage();
        snprintf(error_buffer, ERROR_BUFFER_SIZE, "%s", err_msg.c_str());
        return nullptr;
    }

    char* result = reinterpret_cast<char*>(::malloc(json_str.size() + 1));
    memcpy(result, json_str.c_str(), json_str.size());
    result[json_str.size()] = 0;

    return result;
}

EMSCRIPTEN_KEEPALIVE
void jetpack_free_string(char* data) {
    ::free(data);
}

EMSCRIPTEN_KEEPALIVE
char* jetpack_error_message() {
    return error_buffer;
}

EMSCRIPTEN_KEEPALIVE
int jetpack_handle_command_line(int argc, char **argv) {
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
                (OPT_PROFILE_MALLOC, "print profile of malloc");

        options.parse_positional(OPT_ENTRY);

        JetpackFlags flags;
        auto result = options.parse(argc, argv);
        flags |= JETPACK_TRACE_FILE;

        // print help message
        if (result[OPT_HELP].count()) {
            std::cout << options.help() << std::endl;
            return !result[OPT_HELP].count();
        }

        if (result[OPT_NO_TRACE].count()) {
            flags.setFlag(JETPACK_TRACE_FILE, false);
        }

        if (result[OPT_MINIFY].count()) {
            flags |= JETPACK_MINIFY;
        }

        if (result[OPT_JSX].count()) {
            flags |= JETPACK_JSX;
        }

        if (result[OPT_LIBRARY].count()) {
            flags |= JETPACK_LIBRARY;
        }

        if (result[OPT_SOURCEMAP].count()) {
            flags |= JETPACK_SOURCEMAP;
        }

        if (result[OPT_PROFILE].count()) {
            flags |= JETPACK_PROFILE;
        }

        if (result[OPT_ANALYZE_MODULE].count()) {
            std::string path = result[OPT_ANALYZE_MODULE].as<std::string>();
            return jetpack_analyze_module(path.c_str(), flags, nullptr);
        }

        if (result[OPT_OUT].count()) {
            std::string entry_path = result[OPT_ENTRY].as<std::string>();
            std::string out_path = result[OPT_OUT].as<std::string>();

            return jetpack_bundle_module(entry_path.c_str(), out_path.c_str(), flags, nullptr);
        }

        std::cout << options.help() << std::endl;
        return 0;
    } catch (std::exception &ex) {
        std::cerr << ex.what() << std::endl;
        return 3;
    }
}
