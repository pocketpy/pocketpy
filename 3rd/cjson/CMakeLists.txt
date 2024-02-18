cmake_minimum_required(VERSION 3.10)

project(cjson)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../../include)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(
    cjson
    STATIC
    ${CMAKE_CURRENT_LIST_DIR}/src/cJSON.c
    ${CMAKE_CURRENT_LIST_DIR}/src/cJSONw.cpp
)
