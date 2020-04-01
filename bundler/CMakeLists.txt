cmake_minimum_required(VERSION 3.14)
project(js-bundler)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES
        "src/ModuleResolver.h"
        "src/ModuleResolver.cpp"
        "src/UniqueNameGenerator.h"
        "src/UniqueNameGenerator.cpp"
        "src/Error.h"
        "src/Error.cpp"
        "src/codegen/NodeTraverser.h"
        "src/codegen/NodeTraverser.cpp"
        "src/codegen/CodeGen.h"
        "src/codegen/CodeGen.cpp")

if (UNIX)

    include_directories("../third_party/jemalloc/include")

    if (APPLE)

        link_libraries(${PROJECT_SOURCE_DIR}/../third_party/jemalloc/lib/mac/libjemalloc.a)

    else()

        SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -pthread)
        link_libraries(${PROJECT_SOURCE_DIR}/../third_party/jemalloc/lib/linux/libjemalloc.a)

    endif()

endif()

include_directories(PUBLIC
        "../bundle-utils/src"
        "../js-parser/src"
        "../third_party/cxxopts/include"
        "../third_party/ThreadPool/"
        "../third_party/js-parser/src"
        "../third_party/robin-hood-hashing/src/include")
link_libraries(bundle-utils js-parser fmt)

add_executable(rocket-bundle ${SOURCE_FILES} src/main.cpp)

add_executable(rocket-bundle-test ${SOURCE_FILES}
        tests/scope.cpp
        tests/resolve.cpp
        tests/codegen.cpp
        tests/jsx.cpp)

target_include_directories(rocket-bundle-test PUBLIC "../third_party/googletest/googletest/include")
target_link_libraries(rocket-bundle-test gtest_main)

set(CMAKE_CXX_FLAGS_RELEASE "-g3 -O3")
