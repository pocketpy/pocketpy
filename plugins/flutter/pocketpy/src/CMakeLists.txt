# The Flutter tooling requires that developers have CMake 3.10 or later
# installed. You should not increase this version, as doing so will cause
# the plugin to fail to compile for some customers of the plugin.
cmake_minimum_required(VERSION 3.10)

project(pocketpy_library VERSION 0.0.1 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(pocketpy SHARED
  "pocketpy.c"
)

if(APPLE)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-shorten-64-to-32")
endif()

set_target_properties(pocketpy PROPERTIES
  PUBLIC_HEADER pocketpy.h
  OUTPUT_NAME "pocketpy"
)

target_compile_definitions(pocketpy PUBLIC DART_SHARED_LIB)
