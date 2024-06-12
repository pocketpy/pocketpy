cmake_minimum_required(VERSION 3.21)

project(
    pocketpy
    VERSION 2.0.0
    DESCRIPTION "Open Source Python Interpreter for Game Scripting"
    HOMEPAGE_URL "https://github.com/pocketpy/pocketpy"
    LANGUAGES C CXX
)

# PK_IS_MAIN determines whether the project is being used from root
# or if it is added as a dependency (through add_subdirectory for example).
if (PROJECT_IS_TOP_LEVEL)
  set(PK_IS_MAIN TRUE)
  option(PK_BUILD_SHARED_LIB "Build shared library" OFF)
  option(PK_BUILD_STATIC_LIB "Build static library" OFF)
else()
  set(PK_IS_MAIN FALSE)
  option(PK_BUILD_SHARED_LIB "Build shared library" OFF)
  option(PK_BUILD_STATIC_LIB "Build static library" ON)
endif()

if(PK_BUILD_SHARED_LIB)
    add_library(${PROJECT_NAME} SHARED)
elseif(PK_BUILD_STATIC_LIB)
    add_library(${PROJECT_NAME} STATIC)
else()
    add_executable(main src2/main.cpp)
    target_include_directories(main PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
    target_compile_features(main PRIVATE cxx_std_17)
    # shared linked main
    add_library(${PROJECT_NAME} SHARED)
    target_link_libraries(main PRIVATE ${PROJECT_NAME} ${CMAKE_DL_LIBS})
    # static linked main
    # add_library(${PROJECT_NAME} STATIC)
    # target_link_libraries(${PROJECT_EXE_NAME} ${PROJECT_NAME})
endif()

configure_file("include/pocketpy/common/version.h.in" "pocketpy/common/version.h")

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17 c_std_11)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include ${PROJECT_BINARY_DIR})
file(GLOB_RECURSE POCKETPY_SRC_CPP ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
file(GLOB_RECURSE POCKETPY_SRC_C ${CMAKE_CURRENT_LIST_DIR}/src/*.c)
target_sources(${PROJECT_NAME} PRIVATE ${POCKETPY_SRC_CPP} ${POCKETPY_SRC_C})

set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    DEBUG_POSTFIX d
)

# TODO: use target_compile_options instead
#       because this will change compile options in other projects
#       when pocketpy is added as a subdictory
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /utf-8 /Od /jumptablerdata /GS-")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8 /Od /jumptablerdata /GS-")
    add_compile_options(/wd4267 /wd4244)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions -frtti -O2")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")

    # disable -Wshorten-64-to-32 for apple
    if(APPLE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-shorten-64-to-32")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-shorten-64-to-32")
    endif()
endif()

option(PK_USE_CJSON "" OFF)
if(PK_USE_CJSON)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/3rd/cjson)
    add_definitions(-DPK_USE_CJSON)
endif()

option(PK_ENABLE_OS "" OFF)
if(PK_ENABLE_OS)
    add_definitions(-DPK_ENABLE_OS=1)
endif()

option(PK_ENABLE_PROFILER "" OFF)
if(PK_ENABLE_PROFILER)
    add_definitions(-DPK_ENABLE_PROFILER=1)
endif()

option(PK_NO_EXPORT_C_API "" OFF)
if(PK_NO_EXPORT_C_API)
    add_definitions(-DPK_NO_EXPORT_C_API)
endif()

if(PK_USE_CJSON)
    target_link_libraries(${PROJECT_NAME} PRIVATE cjson)
endif()
