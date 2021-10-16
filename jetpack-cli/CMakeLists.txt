cmake_minimum_required(VERSION 3.12)
project(jetpack-cli)

add_executable(${PROJECT_NAME}
        main.cpp
        )

target_include_directories(${PROJECT_NAME} PRIVATE
        "../esparser/src"
        "../jetpack/src/"
        "../third_party/nlohmann_json/include"
        "../third_party/ordered-map/include"
        "../third_party/ThreadPool"
        )

target_link_libraries(${PROJECT_NAME} PRIVATE jetpack ${CLI_LIBS})
