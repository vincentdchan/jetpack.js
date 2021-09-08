cmake_minimum_required(VERSION 3.12)
project(rocket-bundle)

set(CMAKE_CXX_STANDARD 17)

set(JSON_BuildTests OFF CACHE INTERNAL "")

option(JETPACK_BUILD_CLI "Build CLI tool" ON)
option(JETPACK_BUILD_TESTS "Build jetpack tests" ON)

IF (CMAKE_BUILD_TYPE STREQUAL Debug)
    message("Debug mode")
    add_definitions(-D_DEBUG)
ELSE()
    message("Release mode")
    add_definitions(-D_RELEASE)
ENDIF()

include_directories(./third_party/robin-hood-hashing/src/include)

add_subdirectory(./third_party/nlohmann_json)
add_subdirectory(./third_party/ordered-map)
add_subdirectory(./third_party/fmt)
if(JETPACK_BUILD_TESTS)
    add_subdirectory(./third_party/googletest)
endif()
option(BUILD_SHARED_LIBS "Build shared libs" OFF)
add_subdirectory(./third_party/xxHash/cmake_unofficial)

add_subdirectory(./jetpack)
if(JETPACK_BUILD_CLI)
    add_subdirectory(./jetpack-cli)
endif()
