cmake_minimum_required(VERSION 3.12)
project(jetpack)

option(JETPACK_BUILD_TESTS "Build tests" ON)

set(CMAKE_CXX_STANDARD 17)

file(GLOB parser_source_files
        "src/parser/*.h"
        "src/parser/*.cpp"
        )

set(SOURCE_FILES
        src/utils/string/UChar.h
        src/utils/string/PrivateStringUtils.h
        src/utils/string/PrivateStringUtils.cpp
        src/utils/string/UString.h
        src/utils/string/UString.cpp
        src/utils/io/FileIO.h
        src/utils/io/FileIO.cpp
        src/utils/JetTime.h
        src/utils/JetJSON.h
        src/utils/JetJSON.cpp
        src/utils/JetNumeric.h
        src/utils/Path.h
        src/utils/Common.h
        src/utils/Common.cpp
        src/utils/JetFlags.h
        src/utils/MemoryViewOwner.h
        src/utils/Alloc.h
        src/utils/Alloc.cpp
        src/tokenizer/Token.h
        src/tokenizer/Token.cpp
        src/tokenizer/Location.h
        src/tokenizer/Location.cpp
        src/tokenizer/Comment.h
        src/tokenizer/Comment.cpp
        src/tokenizer/Scanner.h
        src/tokenizer/Scanner.cpp
        ${parser_source_files}
        src/optimize/ConstantFolding.h
        src/optimize/ConstantFolding.cpp
        src/scope/ExportManager.h
        src/scope/ExportManager.cpp
        src/scope/ImportManager.h
        src/scope/ImportManager.cpp
        src/scope/Scope.h
        src/scope/Scope.cpp
        src/scope/Variable.h
        src/scope/Variable.cpp
        src/dumper/AstToJson.h
        src/macros.h
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
        src/codegen/AutoNodeTraverser.h
        src/codegen/AutoNodeTraverser.cpp
        src/codegen/CodeGenConfig.h
        src/codegen/CodeGen.h
        src/codegen/CodeGen.cpp)

if (EMSCRIPTEN)
    add_definitions(-DJETPACK_SINGLE_THREAD)
    set(CMAKE_C_FLAGS_DEBUG "-g3 -O0 -Werror -fexceptions")
    set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0 -Werror -fexceptions")
    set(CMAKE_C_FLAGS_RELEASE "-g0 -O3 -Werror -fexceptions")
    set(CMAKE_CXX_FLAGS_RELEASE "-g0 -O3 -Werror -fexceptions")
elseif(NOT WIN32)
    set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0 -Werror")
    set(CMAKE_CXX_FLAGS_RELEASE "-g0 -O3 -Werror")
endif()

include_directories(PUBLIC
        "../esparser/src"
        "./src"
        "../third_party/ThreadPool/"
        "../third_party/ordered-map/include"
        "../third_party/nlohmann_json/include"
        "../third_party/boost/"
        "../third_party/js-parser/src"
        "../third_party/cxxopts/include"
        "../third_party/robin-hood-hashing/src/include")
link_libraries(fmt)

add_library(jetpack ${SOURCE_FILES})
add_library(jetpackd SHARED ${SOURCE_FILES})

if (APPLE)
    set(DYLIB_SUFFIX "dylib")
else()
    set(DYLIB_SUFFIX "so")
endif()

if(JETPACK_BUILD_TESTS)
    add_executable(jetpack-test
            tests/memory.cpp
            tests/sourcemap.cpp
            tests/scope.cpp
            tests/resolve.cpp
            tests/codegen.cpp
            tests/jsx.cpp
            tests/simple_api.cpp
            tests/common_js.cpp
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
endif()
