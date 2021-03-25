cmake_minimum_required(VERSION 3.12)
project(bundle-utils)

set(CMAKE_CXX_STANDARD 17)

add_library(${PROJECT_NAME} STATIC
        "src/string/UChar.h"
        "src/string/UStringData.h"
        "src/string/UStringData.cpp"
        "src/string/UStringDataPointer.h"
        "src/string/UStringDataPointer.cpp"
        "src/string/UString.h"
        "src/string/UString.cpp"
        "src/Path.h"
        "src/Utils.h"
        "src/Utils.cpp")

IF (CMAKE_BUILD_TYPE STREQUAL Debug)
    ADD_DEFINITIONS(-DDEBUG)
ENDIF()
