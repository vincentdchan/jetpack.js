cmake_minimum_required(VERSION 3.12)
project(jetpack-cli)

add_executable(${PROJECT_NAME}
        main.cpp
        )

target_include_directories(${PROJECT_NAME} PRIVATE
        "../esparser/src"
        "../jetpack/src/"
        )
target_link_libraries(${PROJECT_NAME} PRIVATE jetpack)
