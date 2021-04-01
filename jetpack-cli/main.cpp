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

#include "SimpleAPI.h"

#ifndef __EMSCRIPTEN__

int main(int argc, char** argv) {
    return jetpack::simple_api::HandleCommandLine(argc, argv);
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
