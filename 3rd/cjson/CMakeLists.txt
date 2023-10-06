cmake_minimum_required(VERSION 3.10)

project(cjson)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../pocketpy/include)

set(CMAKE_CXX_FLAGS_RELEASE "-O2")

add_library(
    cjson
    ${CMAKE_CURRENT_LIST_DIR}/src/cJSON.cpp
)
