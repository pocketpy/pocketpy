cmake_minimum_required(VERSION 3.10)

project(pocketpy)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8 /jumptablerdata /GS-")
    add_compile_options(/wd4267 /wd4244)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)

    if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Ox")
        add_definitions(-DNDEBUG)
    endif()
else()
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
        add_definitions(-DNDEBUG)
    endif()

    # disable -Wshorten-64-to-32 for apple
    if(APPLE)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-shorten-64-to-32")
    endif()
endif()

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
file(GLOB_RECURSE POCKETPY_SRC ${CMAKE_CURRENT_LIST_DIR}/src/*.c)

option(PK_ENABLE_OS "" OFF)
if(PK_ENABLE_OS)
    add_definitions(-DPK_ENABLE_OS=1)
endif()

option(PK_ENABLE_PROFILER "" OFF)
if(PK_ENABLE_PROFILER)
    add_definitions(-DPK_ENABLE_PROFILER=1)
endif()

# PK_IS_MAIN determines whether the project is being used from root
# or if it is added as a dependency (through add_subdirectory for example).
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    set(PK_IS_MAIN TRUE)
    option(PK_BUILD_SHARED_LIB "Build shared library" OFF)
    option(PK_BUILD_STATIC_LIB "Build static library" OFF)
else()
    set(PK_IS_MAIN FALSE)
    option(PK_BUILD_SHARED_LIB "Build shared library" OFF)
    option(PK_BUILD_STATIC_LIB "Build static library" ON)
endif()

if(PK_BUILD_SHARED_LIB)
    add_library(${PROJECT_NAME} SHARED ${POCKETPY_SRC})
elseif(PK_BUILD_STATIC_LIB)
    add_library(${PROJECT_NAME} STATIC ${POCKETPY_SRC})
else()
    set(PROJECT_EXE_NAME main)
    add_executable(${PROJECT_EXE_NAME} src2/main.c)
    # shared linked main
    add_library(${PROJECT_NAME} SHARED ${POCKETPY_SRC})
    target_link_libraries(${PROJECT_EXE_NAME} ${PROJECT_NAME})
endif()

# link math library
if(UNIX)
    target_link_libraries(${PROJECT_NAME} PRIVATE m)
endif()