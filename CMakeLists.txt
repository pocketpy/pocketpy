cmake_minimum_required(VERSION 3.10)

project(pocketpy)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (${CMAKE_VERSION} VERSION_LESS "3.12.0") 
    set(Python3_EXECUTABLE "python3")
else()
    find_package(Python3 COMPONENTS Interpreter)
endif()

execute_process(
    COMMAND ${Python3_EXECUTABLE} prebuild.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    RESULT_VARIABLE PREBUILD_RESULT
)

if(NOT ${PREBUILD_RESULT} EQUAL 0)
    message(FATAL_ERROR "prebuild.py: ${PREBUILD_RESULT}")
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /utf-8 /O2")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions -O2")

    # disable -Wshorten-64-to-32 for apple
    if(APPLE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-shorten-64-to-32")
    endif()
endif()

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src POCKETPY_SRC)

option(PK_USE_CJSON "Use cJSON" OFF)
if(PK_USE_CJSON)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/3rd/cjson)
    add_definitions(-DPK_USE_CJSON)
endif()

# PK_IS_MAIN determines whether the project is being used from root
# or if it is added as a dependency (through add_subdirectory for example).
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
  set(PK_IS_MAIN TRUE)
  option(PK_BUILD_SHARED_LIB "Build shared library" OFF)
  option(PK_BUILD_STATIC_LIB "Build static library" OFF)
else()
  set(PK_IS_MAIN FALSE)
  option(PK_BUILD_SHARED_LIB "Build shared library" OFF)
  option(PK_BUILD_STATIC_LIB "Build static library" ON)
endif()

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

if(PK_USE_CJSON)
    target_link_libraries(${PROJECT_NAME} cjson)
endif()
