//
// Created by Duzhong Chen on 2021/3/30.
//

#pragma once

#include <string>
#include <cstdlib>
#include <cstring>
//#include "parser/Config.h"
//#include "codegen/CodeGenConfig.h"

typedef enum {
    JETPACK_JSX = 0x1,
    JETPACK_CONSTANT_FOLDING = 0x2,
    JETPACK_MINIFY = 0x100,
    JETPACK_TRACE_FILE = 0x10000,
    JETPACK_SOURCEMAP = 0x20000,
    JETPACK_LIBRARY = 0x40000,
    JETPACK_PROFILE = 0x1000000,
} JetpackFlag;

#ifdef __cplusplus

#include "utils/JetFlags.h"

JET_DECLARE_FLAGS(JetpackFlags, JetpackFlag)

extern "C" {

#endif

int jetpack_analyze_module(const char* path,
          int flags,
          const char* base_path);  // <-- optional

int jetpack_bundle_module(const char* path,
         const char* out_path,
         int flags,
         const char* base_path);  // <-- optional

int jetpack_handle_command_line(int argc, char** argv);

char* jetpack_parse_and_codegen(const char* content, int flags);

char* jetpack_parse_to_ast(const char* str, int flags);

void jetpack_free_string(char* data);

char* jetpack_error_message();

#ifdef __cplusplus

}  // extern "C"

#endif
