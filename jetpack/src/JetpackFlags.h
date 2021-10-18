//
// Created by Duzhong Chen on 2021/10/18.
//

#pragma once

typedef enum {
    JETPACK_JSX = 0x1,
    JETPACK_CONSTANT_FOLDING = 0x2,
    JETPACK_MINIFY = 0x100,
    JETPACK_COMMENTS = 0x200,
    JETPACK_TRACE_FILE = 0x10000,
    JETPACK_SOURCEMAP = 0x20000,
    JETPACK_LIBRARY = 0x40000,
    JETPACK_PROFILE = 0x1000000,
} JetpackFlag;

#ifdef __cplusplus

#include "utils/JetFlags.h"

JET_DECLARE_FLAGS(JetpackFlags, JetpackFlag)

#endif
