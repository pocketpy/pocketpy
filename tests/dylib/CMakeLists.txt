cmake_minimum_required(VERSION 3.17)

project(test)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/../../include
)

add_library(
    ${PROJECT_NAME}
    SHARED
    test.cpp
)