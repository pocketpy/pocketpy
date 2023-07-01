cmake_minimum_required(VERSION 3.10)

project(pocketpy)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

execute_process(
    COMMAND python prebuild.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    RESULT_VARIABLE PREBUILD_RESULT
)

if(NOT ${PREBUILD_RESULT} EQUAL 0)
    message(FATAL_ERROR "Prebuild failed with code ${PREBUILD_RESULT}")
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR- /EHsc /utf-8 /O2")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fexceptions -O2")
endif()

find_program(CLANGPP clang++)
if(CLANGPP)
    message(STATUS "Using clang with libc++")
    set(CMAKE_CXX_COMPILER ${CLANGPP})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src POCKETPY_SRC)

option(PK_BUILD_EXE "Build executable" OFF)
option(PK_BUILD_SHARED_LIB "Build shared library" OFF)
option(PK_BUILD_STATIC_LIB "Build static library" OFF)

if(NOT PK_BUILD_EXE AND NOT PK_BUILD_SHARED_LIB AND NOT PK_BUILD_STATIC_LIB)
    set(PK_BUILD_EXE ON)

if(PK_BUILD_EXE)
    message(STATUS "Building executable")
    add_executable(${PROJECT_NAME} ${POCKETPY_SRC} src2/main.cpp)
elseif(PK_BUILD_SHARED_LIB)
    message(STATUS "Building shared library")
    add_library(${PROJECT_NAME} SHARED ${POCKETPY_SRC} src2/lib.cpp)
elseif(PK_BUILD_STATIC_LIB)
    message(STATUS "Building static library")
    add_library(${PROJECT_NAME} STATIC ${POCKETPY_SRC} src2/lib.cpp)
endif()