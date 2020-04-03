cmake_minimum_required(VERSION 3.12)
project(js-parser)

set(CMAKE_CXX_STANDARD 17)

# Typically you don't care so much for a third party library's tests to be
# run from your own project's code.
set(JSON_BuildTests OFF CACHE INTERNAL "")

# Don't use include(nlohmann_json/CMakeLists.txt) since that carries with it
# unintended consequences that will break the build.  It's generally
# discouraged (although not necessarily well documented as such) to use
# include(...) for pulling in other CMake projects anyways.

include_directories(../third_party/fmt/include)
include_directories(../third_party/robin-hood-hashing/src/include)
include_directories(../bundle-utils/src)

file(GLOB source_files
        "src/*.h"
        "src/*.hpp"
        "src/*.cpp"
        "src/parser/*.h"
        "src/parser/*.cpp"
        "src/scope/*.h"
        "src/scope/*.cpp"
        "src/dumper/*.h"
        "src/dumper/*.cpp"
        "src/tokenizer/*.h"
        "src/tokenizer/*.cpp"
        )

include_directories(${JEMALLOC_INCLUDE})

add_library(js-parser STATIC ${source_files})
target_link_libraries(js-parser
        PUBLIC nlohmann_json::nlohmann_json tsl::ordered_map)

set(CMAKE_CXX_FLAGS_RELEASE "-g3 -O3")

IF (CMAKE_BUILD_TYPE STREQUAL Debug)
  ADD_DEFINITIONS(-DDEBUG)
ENDIF()
