cmake_minimum_required(VERSION 3.12)
project(bundle-utils)

set(CMAKE_CXX_STANDARD 17)

add_library(${PROJECT_NAME} STATIC
        src/string/UChar.h
        src/string/PrivateStringUtils.h
        src/string/PrivateStringUtils.cpp
        src/string/UString.h
        src/string/UString.cpp
        src/io/FileIO.h
        src/io/FileIO.cpp
        src/JetTime.h
        src/JetJSON.h
        src/JetJSON.cpp
        src/JetNumeric.h
        src/Path.h
        src/Utils.h
        src/Utils.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE xxHash::xxhash)

target_include_directories(${PROJECT_NAME} PUBLIC src)

target_include_directories(${PROJECT_NAME} PRIVATE
        ../third_party/xxHash
        ../third_party/boost
        ../third_party/fmt/include
        )
