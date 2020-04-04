//
// Created by Duzhong Chen on 2020/3/20.
//

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <stdlib.h>
#include <string.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

//#ifndef _WIN32
//#include <jemalloc/jemalloc.h>
//#endif

#include <iostream>
#include <cxxopts.hpp>

#include "Path.h"
#include "ModuleResolver.h"
#include "codegen/CodeGen.h"
#include "parser/ParserContext.h"

#define OPT_HELP "help"
#define OPT_ENTRY "entry"
#define OPT_TOLERANT "tolerant"
#define OPT_JSX "jsx"
#define OPT_LIBRARY "library"
#define OPT_ANALYZE_MODULE "analyze-module"
#define OPT_NO_TRACE "no-trace"
#define OPT_MINIFY "minify"
#define OPT_OUT "out"

using namespace jetpack;

#ifndef __EMSCRIPTEN__

static int AnalyzeModule(const std::string& self_path,
                         const std::string& path,
                         bool jsx,
                         bool trace_file);

static int BundleModule(const std::string& self_path,
                        bool jsx,
                        bool minify,
                        bool library,
                        const std::string& path,
                        const std::string& out_path);

int main(int argc, char** argv) {
    try {
        cxxopts::Options options("rocket-bundle", "Rocket Bundle command line");
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
                ;

        options.parse_positional(OPT_ENTRY);

        auto result = options.parse(argc, argv);
        bool trace_file = true;
        bool minify = false;
        bool jsx = false;
        bool library = false;

        // print help message
        if (result[OPT_HELP].count()) {
            std::cout << options.help() << std::endl;
            return !result[OPT_HELP].count();
        }

        if (result[OPT_NO_TRACE].count()) {
            trace_file = false;
        }

        if (result[OPT_MINIFY].count()) {
            minify = true;
        }

        if (result[OPT_JSX].count()) {
            jsx = true;
        }

        if (result[OPT_LIBRARY].count()) {
            library = true;
        }

        if (result[OPT_ANALYZE_MODULE].count()) {
            std::string path = result[OPT_ANALYZE_MODULE].as<std::string>();
            return AnalyzeModule(argv[0], path, jsx, trace_file);
        }

        if (result[OPT_OUT].count()) {
            std::string entry_path = result[OPT_ENTRY].as<std::string>();
            std::string out_path = result[OPT_OUT].as<std::string>();

            return BundleModule(argv[0], jsx, minify, library, entry_path, out_path);
        }

        std::cout << options.help() << std::endl;
        return 0;
    } catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 3;
    }
}

static int AnalyzeModule(const std::string& self_path_str,
                         const std::string& path,
                         bool jsx,
                         bool trace_file) {
    Path self_path(self_path_str);
    self_path.Pop();

    parser::ParserContext::Config parser_config = parser::ParserContext::Config::Default();
    if (jsx) {
        parser_config.jsx = true;
    }

    // do not release memory
    // it will save your time
    auto resolver = std::shared_ptr<ModuleResolver>(new ModuleResolver, [](void*) {});

    try {
        resolver->SetTraceFile(trace_file);
        resolver->BeginFromEntry(parser_config, self_path.ToString(), path);
        resolver->PrintStatistic();
        return 0;
    } catch (ModuleResolveException& err) {
        err.PrintToStdErr();
        return 3;
    }
}

static int BundleModule(const std::string& self_path_str,
                        bool jsx,
                        bool minify,
                        bool library,
                        const std::string& path,
                        const std::string& out_path) {

    Path self_path(self_path_str);
    self_path.Pop();

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

        resolver->SetTraceFile(true);
        resolver->BeginFromEntry(parser_config, self_path.ToString(), path);
        resolver->CodeGenAllModules(codegen_config, out_path);

        std::cout << "Finished." << std::endl;
        std::cout << "Totally " << resolver->ModCount() << " file(s)." << std::endl;
        return 0;
    } catch (ModuleResolveException& err) {
        err.PrintToStdErr();
        return 3;
    }
}


/**
 * for WASM
 */
#else

static constexpr size_t SLOT_BUFFER_SIZE = 512;

struct JetPackSlotContent {
    int id;
    int status;
    bool minify;
    bool jsx;
    std::string final_buffer;
};

static JetPackSlotContent JETPACK_SLOT[SLOT_BUFFER_SIZE];
static int SlotCounter = 0;

extern "C" {

EMSCRIPTEN_KEEPALIVE
int SlotMake() {
    if (SlotCounter >= SLOT_BUFFER_SIZE) {
        return -1;
    }
    JetPackSlotContent* slot = &JETPACK_SLOT[SlotCounter++];
    slot->id = SlotCounter - 1;
    slot->status = -1;
    slot->minify = false;
    slot->jsx = false;
    slot->final_buffer.resize(0);
    slot->final_buffer.shrink_to_fit();

    return slot->id;
}

EMSCRIPTEN_KEEPALIVE
int SlotSetKv(int handle, const char* key, const char* value) {
    printf("%s: %s", key, value);
    if (strcmp(key, "minify") == 0) {
        JETPACK_SLOT[handle].minify = strcmp(value, "true") == 0;
        return 0;
    } else if (strcmp(key, "jsx") == 0) {
        JETPACK_SLOT[handle].jsx = strcmp(value, "true") == 0;
        return 0;
    }

    return -1;
}

EMSCRIPTEN_KEEPALIVE
int SlotParseAndCodegen(int handle, const char16_t* buffer) {
    bool jsx = JETPACK_SLOT[handle].jsx;
    bool minify = JETPACK_SLOT[handle].minify;
    try {
        auto resolver = std::make_shared<ModuleResolver>();
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

        resolver->SetTraceFile(false);
        resolver->BeginFromEntryString(parser_config, buffer);

        auto final_export_vars = resolver->GetAllExportVars();
        if (minify) {
            resolver->RenameAllInnerScopes();
        }
        resolver->RenameAllRootLevelVariable();

        auto entry_mod = resolver->GetEntryModule();
        entry_mod->CodeGenFromAst(codegen_config);

        JETPACK_SLOT[handle].final_buffer = std::move(entry_mod->codegen_result);

        JETPACK_SLOT[handle].status = 0;
        return 0;
    } catch (ModuleResolveException& err) {
        err.PrintToStdErr();
        JETPACK_SLOT[handle].status = -1;
        return -1;
    }
}

EMSCRIPTEN_KEEPALIVE
int SlotGetStatus(int handle) {
    return JETPACK_SLOT[handle].status;
}

EMSCRIPTEN_KEEPALIVE
int SlotGetBuffer(int handle, char* buffer) {
    if (buffer == NULL) {
        return static_cast<int>(JETPACK_SLOT[handle].final_buffer.size());
    }

    memcpy(buffer,
           JETPACK_SLOT[handle].final_buffer.c_str(),
           JETPACK_SLOT[handle].final_buffer.size());
    return static_cast<int>(JETPACK_SLOT[handle].final_buffer.size());
}

EMSCRIPTEN_KEEPALIVE
void SlotFree(int handle) {
    if (SlotCounter == handle + 1) {
        SlotCounter--;
    }

    JetPackSlotContent* slot = &JETPACK_SLOT[handle];
    slot->final_buffer.resize(0);
    slot->final_buffer.shrink_to_fit();
}


}

#endif
