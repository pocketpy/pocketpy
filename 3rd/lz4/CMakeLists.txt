cmake_minimum_required(VERSION 3.10)

project(lz4)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_LIST_DIR}/lz4libs)
file(GLOB_RECURSE LZ4_SRC ${CMAKE_CURRENT_LIST_DIR}/lz4libs/*.c)

add_library(lz4 STATIC ${LZ4_SRC})
