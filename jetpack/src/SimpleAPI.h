//
// Created by Duzhong Chen on 2021/3/30.
//

#pragma once

#include <string>
#include <cstdlib>
#include <cstring>
#include "./JetpackFlags.h"

#ifdef __cplusplus

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
