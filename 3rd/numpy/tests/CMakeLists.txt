cmake_minimum_required(VERSION 3.10)
project(numpy_bindings)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add pocketpy as a subdirectory
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../../.." "${CMAKE_CURRENT_BINARY_DIR}/pocketpy")

# Include pybind11 and numpy
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../../include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../include")

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
file (GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../src/numpy.cpp")
file (GLOB TESTS "${CMAKE_CURRENT_SOURCE_DIR}/test_numpy.cpp")

# Create numpy executables
add_executable(numpy_bindings ${TESTS} ${SOURCES})

# Set VS debugger working directory (if relevant)
set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY CMAKE_CURRENT_LIST_DIR)

# Link numpy with pocketpy
target_link_libraries(${PROJECT_NAME} PRIVATE pocketpy)