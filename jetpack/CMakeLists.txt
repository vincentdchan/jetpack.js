cmake_minimum_required(VERSION 3.12)
project(jetpack)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES
        src/Benchmark.h
        src/Benchmark.cpp
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
        src/sourcemap/SourceMapDecoder.h
        src/sourcemap/SourceMapDecoder.cpp
        src/codegen/NodeTraverser.h
        src/codegen/NodeTraverser.cpp
        src/codegen/CodeGen.h
        src/codegen/CodeGen.cpp)

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
        "../third_party/cxxopts/include"
        "../third_party/robin-hood-hashing/src/include")
link_libraries(bundle-utils esparser fmt)

add_library(jetpack ${SOURCE_FILES})
add_library(jetpackd SHARED ${SOURCE_FILES})

if (APPLE)
    set(DYLIB_SUFFIX "dylib")
else()
    set(DYLIB_SUFFIX "so")
endif()

add_executable(jetpack-test
        tests/sourcemap.cpp
        tests/scope.cpp
        tests/resolve.cpp
        tests/codegen.cpp
        tests/jsx.cpp
        tests/simple_api.cpp
        tests/constant_folding.cpp)

target_include_directories(jetpack-test PUBLIC
        "../third_party/googletest/googletest/include"
        ../bundle-utils/src
        ./src
        )
target_link_libraries(jetpack-test gtest_main jetpack)

set(TEST_RUNNING_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(TEST_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})
message("test running dir: ${TEST_RUNNING_DIR}")

target_compile_definitions(jetpack-test PUBLIC
        JETPACK_SINGLE_THREAD
        -DJETPACK_TEST_RUNNING_DIR="${TEST_RUNNING_DIR}"
        -DJETPACK_BUILD_DIR="${TEST_BUILD_DIR}"
)

set(CMAKE_CXX_FLAGS_RELEASE "-g3 -O3")

if (UNIX AND NOT EMSCRIPTEN)
    add_definitions(-DJETPACK_HAS_JEMALLOC)
    include_directories("../third_party/jemalloc/include")

    if (APPLE)
        execute_process(COMMAND uname -m OUTPUT_VARIABLE MACHINE_ARCH OUTPUT_STRIP_TRAILING_WHITESPACE)
        message("ARCH: ${MACHINE_ARCH}")

        target_link_libraries(jetpack-test ${PROJECT_SOURCE_DIR}/../third_party/jemalloc/lib/mac/${MACHINE_ARCH}/libjemalloc.a)
        target_link_libraries(jetpackd ${PROJECT_SOURCE_DIR}/../third_party/jemalloc/lib/mac/${MACHINE_ARCH}/libjemalloc.2.${DYLIB_SUFFIX})

    else()

	message("linux jemalloc")
        SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -pthread)
        target_link_libraries(jetpack-test -ldl ${PROJECT_SOURCE_DIR}/../third_party/jemalloc/lib/linux/libjemalloc_pic.a)

    endif()

endif()
