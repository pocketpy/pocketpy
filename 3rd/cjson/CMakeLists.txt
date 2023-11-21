cmake_minimum_required(VERSION 3.10)

project(cjson)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../pocketpy/include)

if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8 /O2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /utf-8 /O2")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions -O2")
endif()

add_library(
    cjson
    STATIC
    ${CMAKE_CURRENT_LIST_DIR}/src/cJSON.c
    ${CMAKE_CURRENT_LIST_DIR}/src/cJSONw.cpp
)
