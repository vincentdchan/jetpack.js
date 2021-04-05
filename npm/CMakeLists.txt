project (jetpp)
cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 17)

include_directories(
  ${CMAKE_JS_INC}
  "../bundle-utils/src"
  "../jetpack/src"
)

add_library(${PROJECT_NAME} SHARED module.cc ${CMAKE_JS_SRC})

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

link_directories("${CMAKE_CURRENT_SOURCE_DIR}/../cmake-build-release/jetpack/")

target_link_libraries(
  ${PROJECT_NAME}
  ${CMAKE_JS_LIB}
  "${CMAKE_CURRENT_SOURCE_DIR}/../cmake-build-release/jetpack/libjetpackd.dylib")

IF(APPLE)
message("build dir ${CMAKE_CURRENT_BINARY_DIR}")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND
      cp "${CMAKE_CURRENT_SOURCE_DIR}/../cmake-build-release/jetpack/libjetpackd.dylib" ./
    COMMAND
      cp "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/jemalloc/lib/mac/x64/libjemalloc.2.dylib" ./
    COMMAND
      install_name_tool -change "/usr/local/lib/libjemalloc.2.dylib" "@rpath/libjemalloc.2.dylib" libjetpackd.dylib
    COMMAND
      install_name_tool -add_rpath "@loader_path" libjetpackd.dylib
    COMMAND
      install_name_tool -delete_rpath "${CMAKE_CURRENT_SOURCE_DIR}/../cmake-build-release/jetpack" jetpp.node
    COMMAND
      install_name_tool -add_rpath "@loader_path" jetpp.node
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Release
    COMMENT "Running CRC32 check..."
)
ENDIF()
