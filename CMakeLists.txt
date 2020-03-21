cmake_minimum_required(VERSION 3.14)
project(rocket-bundle)

add_subdirectory(./third_party/nlohmann_json)
add_subdirectory(./third_party/ordered-map)
add_subdirectory(./third_party/fmt)

add_subdirectory(./bundle-utils)
add_subdirectory(./js-parser)
add_subdirectory(./js-bundler)
