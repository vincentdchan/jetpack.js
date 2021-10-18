cmake_minimum_required(VERSION 3.12)
project(jetpack-wasm)

if (EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".js")
    add_executable(${PROJECT_NAME} main.cpp)
    target_include_directories(${PROJECT_NAME} PRIVATE
            "../esparser/src"
            "../jetpack/src/"
            "../third_party/nlohmann_json/include"
            "../third_party/ordered-map/include"
            "../third_party/ThreadPool"
            )
    target_link_libraries(${PROJECT_NAME} PRIVATE jetpack ${CLI_LIBS})

    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS
            "-s ALLOW_MEMORY_GROWTH=1 \
            -s LLD_REPORT_UNDEFINED=1 \
            -s EXPORTED_RUNTIME_METHODS='[\"ccall\",\"cwrap\", \"allocateUTF8\", \"UTF8ToString\"]' \
            -s EXPORTED_FUNCTIONS='[\"_jetpack_parse_and_codegen\",\"_jetpack_parse_to_ast\", \"_jetpack_free_string\", \"_jetpack_error_message\"]' \
            -s EXPORT_ES6=1 \
            -s ENVIRONMENT='web'")

    set(CMAKE_C_FLAGS_DEBUG "-g3 -O0 -Werror -fexceptions")
    set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0 -Werror -fexceptions")
    set(CMAKE_C_FLAGS_RELEASE "-g0 -O3 -Werror -fexceptions")
    set(CMAKE_CXX_FLAGS_RELEASE "-g0 -O3 -Werror -fexceptions")
endif()
