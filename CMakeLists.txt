cmake_minimum_required(VERSION 3.10)

project(pocketpy)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# use IPO
option(PK_BUILD_WITH_IPO "" TRUE)
include(CheckIPOSupported)
check_ipo_supported(RESULT result)

if(result AND PK_BUILD_WITH_IPO)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(WARNING ">> IPO disabled. You will not get the best performance.")
endif()

if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8 /jumptablerdata /GS-")
    add_compile_options(/wd4267 /wd4244)

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

option(PK_BUILD_MODULE_LZ4 "" OFF)
if(PK_BUILD_MODULE_LZ4)
    add_subdirectory(3rd/lz4)
    include_directories(3rd/lz4)
    add_definitions(-DPK_BUILD_MODULE_LZ4)
endif()

option(PK_BUILD_MODULE_LIBHV "" OFF)
if(PK_BUILD_MODULE_LIBHV)
    add_subdirectory(3rd/libhv)
    include_directories(3rd/libhv/include)
    add_definitions(-DPK_BUILD_MODULE_LIBHV)
endif()

# PK_IS_MAIN determines whether the project is being used from root
# or if it is added as a dependency (through add_subdirectory for example).
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    set(PK_IS_MAIN TRUE)
    option(PK_BUILD_SHARED_LIB "Build shared library" OFF)
    option(PK_BUILD_STATIC_LIB "Build static library" OFF)
else()
    set(PK_IS_MAIN FALSE)
    option(PK_BUILD_SHARED_LIB "Build shared library" OFF)
    option(PK_BUILD_STATIC_LIB "Build static library" ON)
endif()

option(PK_BUILD_STATIC_MAIN "Build static main" OFF)

if(PK_BUILD_SHARED_LIB)
    message(">> Building shared library")
    add_library(${PROJECT_NAME} SHARED ${POCKETPY_SRC})
elseif(PK_BUILD_STATIC_LIB)
    message(">> Building static library")
    add_library(${PROJECT_NAME} STATIC ${POCKETPY_SRC})
else()
    if(PK_BUILD_STATIC_MAIN)
        message(">> Building static library + executable")
        add_library(${PROJECT_NAME} STATIC ${POCKETPY_SRC})
    else()
        message(">> Building shared library + executable")
        add_library(${PROJECT_NAME} SHARED ${POCKETPY_SRC})
    endif()
    add_executable(main src2/main.c)
    target_link_libraries(main ${PROJECT_NAME})
endif()

if(UNIX)
    target_link_libraries(${PROJECT_NAME} m)
    if(PK_ENABLE_OS)
        target_link_libraries(${PROJECT_NAME} dl)
    endif()
endif()

if(PK_BUILD_MODULE_LZ4)
    target_link_libraries(${PROJECT_NAME} lz4)
endif()

if(PK_BUILD_MODULE_LIBHV)
    target_link_libraries(${PROJECT_NAME} libhv_bindings)
endif()