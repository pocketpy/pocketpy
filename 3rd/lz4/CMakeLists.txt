cmake_minimum_required(VERSION 3.10)

project(lz4)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_LIST_DIR}/lz4/lib)
set(LZ4_SRC lz4/lib/lz4.c)

add_library(lz4 STATIC ${LZ4_SRC})
