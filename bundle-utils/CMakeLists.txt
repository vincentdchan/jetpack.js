cmake_minimum_required(VERSION 3.12)
project(bundle-utils)

set(CMAKE_CXX_STANDARD 17)

add_library(${PROJECT_NAME} STATIC
        src/string/UChar.h
        src/string/UStringData.h
        src/string/UStringData.cpp
        src/string/UStringDataPointer.h
        src/string/UStringDataPointer.cpp
        src/string/UString.h
        src/string/UString.cpp
        src/io/FileIO.h
        src/io/FileIO.cpp
        src/Path.h
        src/Utils.h
        src/Utils.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC src)

IF (CMAKE_BUILD_TYPE STREQUAL Debug)
    message("debug mode")
    add_definitions(-D_DEBUG)
ELSE()
    message("release mode")
ENDIF()
