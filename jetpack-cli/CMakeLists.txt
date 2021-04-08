cmake_minimum_required(VERSION 3.12)
project(jetpack-cli)

add_executable(${PROJECT_NAME}
        main.cpp
        )

target_include_directories(${PROJECT_NAME} PRIVATE
        "../esparser/src"
        "../jetpack/src/"
        )

set(CLI_LIBS "")

if (UNIX AND NOT EMSCRIPTEN)

    if (APPLE)
        execute_process(COMMAND uname -m OUTPUT_VARIABLE MACHINE_ARCH OUTPUT_STRIP_TRAILING_WHITESPACE)
        message("CLI ARCH: ${MACHINE_ARCH}")
        set(CLI_LIBS ${PROJECT_SOURCE_DIR}/../third_party/jemalloc/lib/mac/${MACHINE_ARCH}/libjemalloc.a)
    else()

	set(CLI_LIBS "-lpthread -ldl ${PROJECT_SOURCE_DIR}/../third_party/jemalloc/lib/linux/libjemalloc_pic.a" )

    endif()

ENDIF()

target_link_libraries(${PROJECT_NAME} PRIVATE jetpack ${CLI_LIBS})
