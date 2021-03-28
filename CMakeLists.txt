cmake_minimum_required(VERSION 3.12)
project(rocket-bundle)

set(CMAKE_CXX_STANDARD 17)

set(JSON_BuildTests OFF CACHE INTERNAL "")

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
add_subdirectory(./third_party/googletest)

add_subdirectory(./bundle-utils)
add_subdirectory(./esparser)
add_subdirectory(bundler)
