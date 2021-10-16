project (jetpp)
cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 17)

include_directories(
  ${CMAKE_JS_INC}
  "../jetpack/src"
)

add_library(${PROJECT_NAME} SHARED module.cc node_helper.hpp ${CMAKE_JS_SRC})

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

link_directories("${CMAKE_CURRENT_SOURCE_DIR}/../cmake-build-release/jetpack/")

IF(UNIX)
message("Linux")

target_link_libraries(
  ${PROJECT_NAME}
  ${CMAKE_JS_LIB}
  "${CMAKE_CURRENT_SOURCE_DIR}/../cmake-build-release/jetpack/libjetpack.a"
  "${CMAKE_CURRENT_SOURCE_DIR}/../cmake-build-release/bundle-utils/libbundle-utils.a"
  "${CMAKE_CURRENT_SOURCE_DIR}/../cmake-build-release/esparser/libesparser.a"
  "${CMAKE_CURRENT_SOURCE_DIR}/../cmake-build-release/third_party/fmt/libfmt.a"
  )

ENDIF()
