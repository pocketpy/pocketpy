cmake_minimum_required(VERSION 3.10)

project(lz4)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

target_include_directories(lz4 PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/lz4/lib
)

target_include_directories(lz4 INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

add_library(lz4 STATIC lz4/lib/lz4.c)
