cmake_minimum_required(VERSION 3.10)

project(libhv_bindings)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_SHARED "build shared library" OFF)
option(BUILD_STATIC "build static library" ON)
option(BUILD_EXAMPLES "build examples" OFF)

option(WITH_OPENSSL "with openssl library" OFF)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/libhv)

AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_LIST_DIR}/src LIBHV_BINDINGS_SRC)

add_library(${PROJECT_NAME} STATIC ${LIBHV_BINDINGS_SRC})

target_link_libraries(${PROJECT_NAME} hv_static)

# define WITHOUT_HTTP_CONTENT
target_compile_definitions(libhv_bindings PRIVATE WITHOUT_HTTP_CONTENT)
target_compile_definitions(hv_static PRIVATE WITHOUT_HTTP_CONTENT)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../../include
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/libhv
    ${CMAKE_CURRENT_LIST_DIR}/libhv/base
    ${CMAKE_CURRENT_LIST_DIR}/libhv/evpp
    ${CMAKE_CURRENT_LIST_DIR}/libhv/event
    ${CMAKE_CURRENT_LIST_DIR}/libhv/http
    ${CMAKE_CURRENT_LIST_DIR}/libhv/ssl
    ${CMAKE_CURRENT_LIST_DIR}/libhv/cpputil
)

target_include_directories(${PROJECT_NAME} INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/include
)