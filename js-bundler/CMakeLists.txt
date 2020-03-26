cmake_minimum_required(VERSION 3.14)
project(js-bundler)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES
        "src/ModuleResolver.h"
        "src/ModuleResolver.cpp"
        "src/ModuleConcatenation.h"
        "src/ModuleConcatenation.cpp"
        "src/UniqueNameGenerator.h"
        "src/UniqueNameGenerator.cpp"
        "src/Error.h"
        "src/Error.cpp"
        "src/codegen/NodeTraverser.h"
        "src/codegen/NodeTraverser.cpp"
        "src/codegen/CodeGen.h"
        "src/codegen/CodeGen.cpp")

include_directories(PUBLIC
        "../bundle-utils/src"
        "../js-parser/src"
        "../third_party/cxxopts/include"
        "../third_party/ThreadPool/"
        "../third_party/js-parser/src"
        "../third_party/robin-hood-hashing/src/include")
link_libraries(bundle-utils js-parser fmt)

add_executable(${PROJECT_NAME} ${SOURCE_FILES} src/main.cpp)

add_executable(js-bundler-test ${SOURCE_FILES}
        tests/scope.cpp
        tests/resolve.cpp
        tests/codegen.cpp)
target_include_directories(js-bundler-test PUBLIC "../third_party/googletest/googletest/include")
target_link_libraries(js-bundler-test gtest_main)
