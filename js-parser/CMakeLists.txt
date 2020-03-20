cmake_minimum_required(VERSION 3.14)
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

file(GLOB source_files
        "src/*.h"
        "src/*.hpp"
        "src/*.cpp"
        "src/parser/*.h"
        "src/parser/*.cpp"
        "src/dumper/*.h"
        "src/dumper/*.cpp"
        "src/tokenizer/*.h"
        "src/tokenizer/*.cpp"
        "src/codegen/*.h"
        "src/codegen/*.cpp"
        )

execute_process(COMMAND jemalloc-config "--includedir" OUTPUT_VARIABLE JEMALLOC_INCLUDE)
execute_process(COMMAND jemalloc-config "--libdir" OUTPUT_VARIABLE JEMALLOC_LIBDIR)
execute_process(COMMAND jemalloc-config "--cxxflags" OUTPUT_VARIABLE JEMALLOC_CXXFLAGS)
string(STRIP ${JEMALLOC_LIBDIR} JEMALLOC_LIBDIR)

#set(Boost_NO_BOOST_CMAKE TRUE)
#find_package(Boost COMPONENTS program_options REQUIRED)
#include_directories(${Boost_INCLUDE_DIR})
#link_directories(${Boost_LIBRARY_DIR})

include_directories(${JEMALLOC_INCLUDE})

add_library(js-parser STATIC ${source_files})
target_link_libraries(js-parser
        PUBLIC -ljemalloc -L${JEMALLOC_LIBDIR}
        nlohmann_json::nlohmann_json tsl::ordered_map
)

IF (CMAKE_BUILD_TYPE STREQUAL Debug)
  ADD_DEFINITIONS(-DDEBUG)
ENDIF()

set(CMAKE_CXX_FLAGS_RELEASE "-g3 -O3")
