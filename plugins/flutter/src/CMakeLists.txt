# The Flutter tooling requires that developers have CMake 3.10 or later
# installed. You should not increase this version, as doing so will cause
# the plugin to fail to compile for some customers of the plugin.
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(ANDROID_STL c++_static)

project(pocketpy_library VERSION 0.0.1 LANGUAGES CXX)

add_library(pocketpy SHARED
  "pocketpy.cpp"
)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

set_target_properties(pocketpy PROPERTIES
  PUBLIC_HEADER pocketpy.h
  OUTPUT_NAME "pocketpy"
)

target_compile_definitions(pocketpy PUBLIC DART_SHARED_LIB)
