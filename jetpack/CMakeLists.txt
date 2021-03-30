cmake_minimum_required(VERSION 3.12)
project(jetpack)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES
        src/ResolveResult.h
        src/WorkerError.h
        src/ModuleProvider.h
        src/ModuleProvider.cpp
        src/ModuleFile.h
        src/ModuleFile.cpp
        src/ModulesTable.h
        src/ModulesTable.cpp
        src/ModuleResolver.h
        src/ModuleResolver.cpp
        src/ModuleCompositor.h
        src/ModuleCompositor.cpp
        src/UniqueNameGenerator.h
        src/UniqueNameGenerator.cpp
        src/GlobalImportHandler.h
        src/GlobalImportHandler.cpp
        src/Error.h
        src/SimpleAPI.h
        src/SimpleAPI.cpp
        src/sourcemap/MappingCollector.h
        src/sourcemap/MappingCollector.cpp
        src/sourcemap/SourceMapGenerator.h
        src/sourcemap/SourceMapGenerator.cpp
        src/codegen/NodeTraverser.h
        src/codegen/NodeTraverser.cpp
        src/codegen/CodeGen.h
        src/codegen/CodeGen.cpp)

if (UNIX AND NOT EMSCRIPTEN)

    include_directories("../third_party/jemalloc/include")

    if (APPLE)

        if(CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
            message("arch: ${CMAKE_OSX_ARCHITECTURES}")
            link_libraries(${PROJECT_SOURCE_DIR}/../third_party/jemalloc/lib/mac/arm64/libjemalloc.a)
        else()
            link_libraries(${PROJECT_SOURCE_DIR}/../third_party/jemalloc/lib/mac/x64/libjemalloc.a)
        endif()

    else()

        SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -pthread)
        link_libraries(${PROJECT_SOURCE_DIR}/../third_party/jemalloc/lib/linux/libjemalloc.a)

    endif()

endif()

if (EMSCRIPTEN)
add_definitions(-DJETPACK_SINGLE_THREAD)
set(CMAKE_CXX_FLAGS "-s \"EXTRA_EXPORTED_RUNTIME_METHODS=['UTF8ToString', 'lengthBytesUTF8', 'stringToUTF8', 'stringToUTF16']\"")
endif()

include_directories(PUBLIC
        "../bundle-utils/src"
        "../esparser/src"
        "./src"
        "../third_party/ThreadPool/"
        "../third_party/js-parser/src"
        "../third_party/robin-hood-hashing/src/include")
link_libraries(bundle-utils esparser fmt)

add_library(jetpack ${SOURCE_FILES})

add_executable(jetpack-test
        tests/sourcemap.cpp
        tests/scope.cpp
        tests/resolve.cpp
        tests/codegen.cpp
        tests/jsx.cpp
        tests/contant_folding.cpp)

target_include_directories(jetpack-test PUBLIC
        "../third_party/googletest/googletest/include"
        ./src
        )
target_link_libraries(jetpack-test gtest_main jetpack)

set(TEST_RUNNING_DIR ${CMAKE_CURRENT_SOURCE_DIR})
message("test running dir: ${TEST_RUNNING_DIR}")

target_compile_definitions(jetpack-test PUBLIC
        JETPACK_SINGLE_THREAD
        -DJETPACK_TEST_RUNNING_DIR="${TEST_RUNNING_DIR}"
)

set(CMAKE_CXX_FLAGS_RELEASE "-g3 -O3")
