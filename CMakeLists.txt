cmake_minimum_required(VERSION 3.10)

project(pocketpy)

if(PK_BUILD_SHARED_LIB OR NOT PK_BUILD_STATIC_MAIN)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(CMakeOptions.txt)

if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8 /jumptablerdata")
    add_compile_options(/wd4267 /wd4244 /wd4146 /wd4819 /experimental:c11atomics)

    if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Ox")
        add_definitions(-DNDEBUG)
    endif()

    # if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    #     message(">> Enable Address Sanitizer")
    #     set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /fsanitize=address /Zi")
    # endif()
else()
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
        add_definitions(-DNDEBUG)
    endif()

    if(APPLE)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-shorten-64-to-32")
    else()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast")
    endif()

    if(PK_ENABLE_DETERMINISM)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffp-contract=off")
    endif()
endif()

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
file(GLOB_RECURSE POCKETPY_SRC ${CMAKE_CURRENT_LIST_DIR}/src/*.c)


if(PK_ENABLE_OS)
    add_definitions(-DPK_ENABLE_OS=1)
else()
    add_definitions(-DPK_ENABLE_OS=0)
endif()

if(PK_ENABLE_THREADS)
    add_definitions(-DPK_ENABLE_THREADS=1)
else()
    add_definitions(-DPK_ENABLE_THREADS=0)
endif()

if(PK_ENABLE_DETERMINISM)
    add_subdirectory(3rd/dmath/dmath)
    add_definitions(-DPK_ENABLE_DETERMINISM=1)
else()
    add_definitions(-DPK_ENABLE_DETERMINISM=0)
endif()

if(PK_ENABLE_WATCHDOG)
    add_definitions(-DPK_ENABLE_WATCHDOG=1)
else()
    add_definitions(-DPK_ENABLE_WATCHDOG=0)
endif()

if(PK_ENABLE_CUSTOM_SNAME)
    add_definitions(-DPK_ENABLE_CUSTOM_SNAME=1)
else()
    add_definitions(-DPK_ENABLE_CUSTOM_SNAME=0)
endif()

if(PK_ENABLE_MIMALLOC)
    message(">> Fetching mimalloc")
    include(FetchContent)
    
    FetchContent_Declare(
        mimalloc
        GIT_REPOSITORY https://github.com/microsoft/mimalloc.git
        GIT_TAG        v3.1.5
    )

    set(MI_OVERRIDE OFF CACHE BOOL "" FORCE)
    set(MI_NO_USE_CXX ON CACHE BOOL "" FORCE)
    set(MI_BUILD_SHARED OFF CACHE BOOL "" FORCE)
    set(MI_BUILD_OBJECT OFF CACHE BOOL "" FORCE)
    set(MI_BUILD_STATIC ON CACHE BOOL "" FORCE)
    set(MI_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(mimalloc)
    include_directories(${mimalloc_SOURCE_DIR}/include)

    add_definitions(-DPK_ENABLE_MIMALLOC=1)
else()
    add_definitions(-DPK_ENABLE_MIMALLOC=0)
endif()


if(PK_BUILD_MODULE_LZ4)
    add_subdirectory(3rd/lz4)
    add_definitions(-DPK_BUILD_MODULE_LZ4)
endif()

if(PK_BUILD_MODULE_LIBHV)
    add_subdirectory(3rd/libhv)
    add_definitions(-DPK_BUILD_MODULE_LIBHV)
endif()

if(PK_BUILD_MODULE_CUTE_PNG)
    add_subdirectory(3rd/cute_png)
    add_definitions(-DPK_BUILD_MODULE_CUTE_PNG)
endif()


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

target_include_directories(
    ${PROJECT_NAME}
    INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/include
)

if(PK_ENABLE_DETERMINISM)
    target_link_libraries(${PROJECT_NAME} dmath)
    if(MSVC)
        target_link_options(${PROJECT_NAME} PRIVATE /FORCE:MULTIPLE)
    endif()
endif()

if(PK_ENABLE_THREADS)
    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME} Threads::Threads)
endif()

if(UNIX AND NOT APPLE)
    if(NOT PK_ENABLE_DETERMINISM)
        # use platform libm
        target_link_libraries(${PROJECT_NAME} m)
    endif()

    if(PK_ENABLE_OS)
        target_link_libraries(${PROJECT_NAME} dl)
    endif()
elseif(WIN32)
    target_link_libraries(${PROJECT_NAME} ws2_32)
endif()

if(PK_BUILD_WITH_UNITY)
    set_source_files_properties(${POCKETPY_SRC} PROPERTIES UNITY_GROUP "pocketpy")
    set_target_properties(${PROJECT_NAME} PROPERTIES UNITY_BUILD_MODE GROUP)
    set_target_properties(${PROJECT_NAME} PROPERTIES UNITY_BUILD ON)
endif()

############################################
if(PK_BUILD_MODULE_LZ4)
    target_link_libraries(${PROJECT_NAME} lz4)
endif()

if(PK_BUILD_MODULE_LIBHV)
    target_link_libraries(${PROJECT_NAME} libhv_bindings)
endif()

if(PK_BUILD_MODULE_CUTE_PNG)
    target_link_libraries(${PROJECT_NAME} cute_png)
endif()


if(PK_ENABLE_MIMALLOC)
    target_link_libraries(${PROJECT_NAME} mimalloc-static)
endif()