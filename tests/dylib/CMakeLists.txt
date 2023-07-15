cmake_minimum_required(VERSION 3.17)

project(test)

set(CMAKE_C_STANDARD 11)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR- /EHsc /utf-8 /O2")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fexceptions -O2")
endif()

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/../../include
)

add_library(
    ${PROJECT_NAME}
    SHARED
    test.c
)


if (WIN32)
    add_library(
        pocketpy
        SHARED
        ${CMAKE_CURRENT_LIST_DIR}/../../src2/pocketpy_c.cpp
    )

    target_link_libraries(
        ${PROJECT_NAME}
        pocketpy
    )
endif()