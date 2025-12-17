cmake_minimum_required(VERSION 3.10)

project(periphery-ffi)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(BUILD_TESTS "" OFF)
add_subdirectory(c-periphery)

add_library(${PROJECT_NAME} STATIC ${CMAKE_CURRENT_LIST_DIR}/src/periphery.c)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(${PROJECT_NAME} periphery)
