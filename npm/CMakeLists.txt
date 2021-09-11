project (jetpp)
cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 17)

include_directories(
  ${CMAKE_JS_INC}
  "../bundle-utils/src"
  "../jetpack/src"
)

add_library(${PROJECT_NAME} SHARED module.cc ${CMAKE_JS_SRC})

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

link_directories("${CMAKE_CURRENT_SOURCE_DIR}/../cmake-build-release/jetpack/")


IF(APPLE)
target_link_libraries(
  ${PROJECT_NAME}
  ${CMAKE_JS_LIB}
  "${CMAKE_CURRENT_SOURCE_DIR}/../cmake-build-release/jetpack/libjetpackd.dylib")

execute_process(COMMAND uname -m OUTPUT_VARIABLE MACHINE_ARCH OUTPUT_STRIP_TRAILING_WHITESPACE)
message("ARCH: ${MACHINE_ARCH}")

message("build dir ${CMAKE_CURRENT_BINARY_DIR}")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND
      cp "${CMAKE_CURRENT_SOURCE_DIR}/../cmake-build-release/jetpack/libjetpackd.dylib" ./
    COMMAND
      cp "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/jemalloc/lib/mac/${MACHINE_ARCH}/libjemalloc.2.dylib" ./
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
ELSEIF(UNIX)
message("Linux")

target_link_libraries(
  ${PROJECT_NAME}
  ${CMAKE_JS_LIB}
  "${CMAKE_CURRENT_SOURCE_DIR}/../cmake-build-release/jetpack/libjetpack.a"
  "${CMAKE_CURRENT_SOURCE_DIR}/../cmake-build-release/bundle-utils/libbundle-utils.a"
  "${CMAKE_CURRENT_SOURCE_DIR}/../cmake-build-release/esparser/libesparser.a"
  "${CMAKE_CURRENT_SOURCE_DIR}/../cmake-build-release/third_party/fmt/libfmt.a"
  "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/jemalloc/lib/linux/libjemalloc_pic.a"
  )

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND
      cp "${CMAKE_CURRENT_SOURCE_DIR}/../cmake-build-release/jetpack/libjetpackd.so" ./
    COMMAND
      cp "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/jemalloc/lib/linux/libjemalloc.so.2" ./
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Release
)

ENDIF()
