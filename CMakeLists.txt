cmake_minimum_required(VERSION 3.14)
project(BestPack)

set(CMAKE_CXX_STANDARD 17)

file(GLOB source_files
        "src/*.h"
        "src/*.hpp"
        "src/*.cpp"
        "src/parser/*.h"
        "src/parser/*.cpp"
        )

add_executable(BestPack ${source_files})