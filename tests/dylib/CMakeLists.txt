cmake_minimum_required(VERSION 3.17)

project(test)

set(CMAKE_C_STANDARD 11)

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/../../include
)

add_library(
    ${PROJECT_NAME}
    SHARED
    test.c
)

add_library(
    pocketpy
    SHARED
    ${CMAKE_CURRENT_LIST_DIR}/../../src2/pocketpy_c.cpp
)

target_link_libraries(
    ${PROJECT_NAME}
    pocketpy
)
