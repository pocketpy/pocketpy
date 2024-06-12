cmake_minimum_required(VERSION 3.21)

project(cjson)

add_library(
    cjson
    STATIC
    ${CMAKE_CURRENT_LIST_DIR}/src/cJSON.c
    ${CMAKE_CURRENT_LIST_DIR}/src/cJSONw.cpp
)

target_compile_features(cjson PUBLIC cxx_std_17 c_std_11)

target_include_directories(cjson PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_include_directories(cjson PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../../include)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
