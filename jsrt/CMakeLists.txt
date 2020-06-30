cmake_minimum_required(VERSION 3.12)
project(jsrt)

set(CMAKE_CXX_STANDARD 14)

add_library(${PROJECT_NAME} STATIC
        "cutils.h"
        "cutils.c"
        "jsheap.h"
        "jsheap.c"
        "static-str.h"
        "jsrt.h"
        "jsrt.c")
