cmake_minimum_required(VERSION 3.14)
project(BestPack)

set(CMAKE_CXX_STANDARD 17)

# Typically you don't care so much for a third party library's tests to be
# run from your own project's code.
set(JSON_BuildTests OFF CACHE INTERNAL "")

# Don't use include(nlohmann_json/CMakeLists.txt) since that carries with it
# unintended consequences that will break the build.  It's generally
# discouraged (although not necessarily well documented as such) to use
# include(...) for pulling in other CMake projects anyways.
add_subdirectory(nlohmann_json)

add_subdirectory(ordered-map)

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
        )

add_executable(BestPack ${source_files})
target_link_libraries(BestPack PUBLIC -ljemalloc PRIVATE nlohmann_json::nlohmann_json tsl::ordered_map)

add_library(jsci STATIC ${source_files})
target_link_libraries(jsci PUBLIC -ljemalloc nlohmann_json::nlohmann_json tsl::ordered_map)
