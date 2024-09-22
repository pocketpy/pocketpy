cmake_minimum_required(VERSION 3.10)
project(numpy)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include pybind11 and numpy
include_directories("${CMAKE_CURRENT_LIST_DIR}/../../include")
include_directories("${CMAKE_CURRENT_LIST_DIR}/include")

# Control xtensor warnings (OFF by default)
option(SHOW_XTENSOR_WARNINGS "Show warnings from xtensor" OFF)

if(MSVC)
    add_compile_options(/wd4018 /wd4172 /wd4819)
    add_compile_options(/bigobj)
endif()

# Suppress xtensor warnings if SHOW_XTENSOR_WARNINGS is OFF
if(NOT SHOW_XTENSOR_WARNINGS)
    add_compile_definitions(SUPPRESS_XTENSOR_WARNINGS)
    if(MSVC)
        add_compile_options(/wd4244 /wd4267 /wd4018)
    else()
        add_compile_options(-Wno-sign-compare -Wno-conversion -Wno-unused-variable -Wno-unused-parameter)
    endif()
endif()

# Add numpy source and test files
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)

# Create numpy dynamic library
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Define PY_DYNAMIC_MODULE for numpy
target_compile_definitions(${PROJECT_NAME} PRIVATE PY_DYNAMIC_MODULE)

# Link numpy with pocketpy
if(WIN32)
    target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/../../build/Release/pocketpy.lib
    )
else()
    target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/../../build/libpocketpy.so
    )
endif()

