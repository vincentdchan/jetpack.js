cmake_minimum_required(VERSION 3.12)
project(jsrt)

set(CMAKE_CXX_STANDARD 14)

add_library(${PROJECT_NAME} STATIC
        "cutils.h"
        "cutils.c"
        "libunicode.h"
        "libunicode.c"
        "libunicode-table.h"
        "libregexp-opcode.h"
        "libregexp.h"
        "libregexp.c"
        "jsheap.h"
        "jsheap.c"
        "static-str.h"
        "jsrt.h"
        "jsrt.c")
