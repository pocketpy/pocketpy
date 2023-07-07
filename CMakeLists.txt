cmake_minimum_required(VERSION 3.10)

project(pocketpy)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

execute_process(
    COMMAND ${Python3_EXECUTABLE} prebuild.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    RESULT_VARIABLE PREBUILD_RESULT
)

if(NOT ${PREBUILD_RESULT} EQUAL 0)
    message(FATAL_ERROR "prebuild.py: ${PREBUILD_RESULT}")
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR- /EHsc /utf-8 /O2")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fexceptions -O2")
endif()

option(PK_EXPORT_C_API "Export C API" OFF)

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

if(PK_EXPORT_C_API)
    message(STATUS "Exporting C API")
    include_directories(${CMAKE_CURRENT_LIST_DIR}/c_bindings)
    set(PK_LIB_CPP ${CMAKE_CURRENT_LIST_DIR}/c_bindings/pocketpy_c.cpp)
else()
    set(PK_LIB_CPP ${CMAKE_CURRENT_LIST_DIR}/src2/lib.cpp)
endif()

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src POCKETPY_SRC)

option(PK_BUILD_SHARED_LIB "Build shared library" OFF)
option(PK_BUILD_STATIC_LIB "Build static library" OFF)

if(PK_BUILD_SHARED_LIB)
    add_library(${PROJECT_NAME} SHARED ${POCKETPY_SRC} ${PK_LIB_CPP})
elseif(PK_BUILD_STATIC_LIB)
    add_library(${PROJECT_NAME} STATIC ${POCKETPY_SRC} ${PK_LIB_CPP})
else()
    add_executable(${PROJECT_NAME} ${POCKETPY_SRC} src2/main.cpp)
endif()