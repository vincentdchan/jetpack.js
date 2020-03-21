cmake_minimum_required(VERSION 3.14)
project(bundle-utils)

add_library(${PROJECT_NAME} STATIC
        "src/Utils.h"
        "src/Utils.cpp")
