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
    add_compile_options("/utf-8")
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O2")

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src POCKETPY_SRC)

option(PK_BUILD_SHARED_LIB "Build shared library" OFF)
option(PK_BUILD_STATIC_LIB "Build static library" OFF)

option(PK_EXPORT_CXX_SYMBOLS "Export C++ symbols" OFF)

if(PK_BUILD_SHARED_LIB)
    add_library(${PROJECT_NAME} SHARED ${POCKETPY_SRC})
elseif(PK_BUILD_STATIC_LIB)
    add_library(${PROJECT_NAME} STATIC ${POCKETPY_SRC})
else()
    add_library(${PROJECT_NAME} SHARED ${POCKETPY_SRC})
    set(PROJECT_EXE_NAME main)
    add_executable(${PROJECT_EXE_NAME} src2/main.cpp)
    target_link_libraries(${PROJECT_EXE_NAME} ${PROJECT_NAME})
    target_link_libraries(${PROJECT_EXE_NAME} ${CMAKE_DL_LIBS})
endif()

if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
    target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS})
endif()

if(PK_EXPORT_CXX_SYMBOLS AND MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()