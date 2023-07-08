cmake_minimum_required(VERSION 3.17)

project(test_c_bindings)

set(CMAKE_C_STANDARD 11)

option(PK_BUILD_STATIC_LIB "Build static library" ON)

add_subdirectory(
    ${CMAKE_CURRENT_LIST_DIR}/../
    ${CMAKE_CURRENT_LIST_DIR}/build/pocketpy/
)

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/../include
)

add_executable(${PROJECT_NAME} test.c)

target_link_libraries(
    ${PROJECT_NAME}
    pocketpy
)