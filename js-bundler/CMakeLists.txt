cmake_minimum_required(VERSION 3.14)
project(js-bundler)

set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME}
        "src/ModuleResolver.h"
        "src/ModuleResolver.cpp"
        "src/Error.h"
        "src/Error.cpp"
        "src/main.cpp")
target_include_directories(${PROJECT_NAME} PUBLIC
        "../bundle-utils/src"
        "../js-parser/src"
        "../third_party/cxxopts/include"
        "../third_party/ThreadPool/"
        "../third_party/js-parser/src"
        "../third_party/robin-hood-hashing/src/include")
target_link_libraries(${PROJECT_NAME} js-parser fmt)
