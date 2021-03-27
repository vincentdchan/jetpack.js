cmake_minimum_required(VERSION 3.12)
project(js-bundler)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES
        src/OutputStream.h
        src/OutputStream.cpp
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
        src/FileIO.h
        src/FileIO.cpp
        src/Error.h
        src/Error.cpp
        src/SourceMapGenerator.h
        src/SourceMapGenerator.cpp
        src/codegen/NodeTraverser.h
        src/codegen/NodeTraverser.cpp
        src/codegen/CodeGen.h
        src/codegen/CodeGen.cpp)

if (UNIX AND NOT EMSCRIPTEN)

    include_directories("../third_party/jemalloc/include")

    if (APPLE)

        link_libraries(${PROJECT_SOURCE_DIR}/../third_party/jemalloc/lib/mac/libjemalloc.a)

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
        "../third_party/cxxopts/include"
        "../third_party/ThreadPool/"
        "../third_party/js-parser/src"
        "../third_party/robin-hood-hashing/src/include")
link_libraries(bundle-utils esparser fmt)

add_executable(jetpack ${SOURCE_FILES} src/main.cpp)

add_executable(jetpack-test ${SOURCE_FILES}
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
target_link_libraries(jetpack-test gtest_main)

target_compile_definitions(jetpack-test PUBLIC
        JETPACK_SINGLE_THREAD
)

set(CMAKE_CXX_FLAGS_RELEASE "-g3 -O3")
