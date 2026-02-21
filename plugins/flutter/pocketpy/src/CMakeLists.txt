# The Flutter tooling requires that developers have CMake 3.10 or later
# installed. You should not increase this version, as doing so will cause
# the plugin to fail to compile for some customers of the plugin.
cmake_minimum_required(VERSION 3.10)

project(pocketpy_library VERSION 0.0.1 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(FetchContent)

set(PK_ENABLE_OS OFF CACHE BOOL "" FORCE)
set(PK_BUILD_MODULE_LZ4 ON CACHE BOOL "" FORCE)
set(PK_BUILD_MODULE_CUTE_PNG ON CACHE BOOL "" FORCE)
set(PK_BUILD_MODULE_MSGPACK ON CACHE BOOL "" FORCE)
set(PK_BUILD_SHARED_LIB ON CACHE BOOL "" FORCE)

FetchContent_Declare(
  pocketpy
  GIT_REPOSITORY https://github.com/pocketpy/pocketpy.git
  GIT_TAG        v2.1.8
)

FetchContent_MakeAvailable(pocketpy)

set_target_properties(pocketpy PROPERTIES
  PUBLIC_HEADER ${pocketpy_SOURCE_DIR}/include/pocketpy.h
  OUTPUT_NAME "pocketpy"
)

target_compile_definitions(pocketpy PUBLIC DART_SHARED_LIB)
