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
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    endif()
endif()

option(PK_EXPORT_C_API "Export C API" ON)

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

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

if(PK_BUILD_SHARED_LIB)
    add_library(${PROJECT_NAME} SHARED ${POCKETPY_SRC} ${PK_LIB_CPP})
elseif(PK_BUILD_STATIC_LIB)
    add_library(${PROJECT_NAME} STATIC ${POCKETPY_SRC} ${PK_LIB_CPP})
else()
    add_library(${PROJECT_NAME} SHARED ${POCKETPY_SRC} ${PK_LIB_CPP})
    set(PROJECT_EXE_NAME main)
    add_executable(${PROJECT_EXE_NAME} src2/main.cpp)
    target_link_libraries(${PROJECT_EXE_NAME} ${PROJECT_NAME})
    target_link_libraries(${PROJECT_EXE_NAME} ${CMAKE_DL_LIBS})
endif()

target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS})