cmake_minimum_required(VERSION 3.12)
project(rocket-bundle)

set(CMAKE_CXX_STANDARD 17)

set(JSON_BuildTests OFF CACHE INTERNAL "")

include_directories(./third_party/robin-hood-hashing/src/include)

add_subdirectory(./third_party/nlohmann_json)
add_subdirectory(./third_party/ordered-map)
add_subdirectory(./third_party/fmt)
add_subdirectory(./third_party/googletest)

add_subdirectory(./bundle-utils)
add_subdirectory(./esparser)
add_subdirectory(bundler)
