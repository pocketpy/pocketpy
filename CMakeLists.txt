cmake_minimum_required(VERSION 3.10)

project(pocketpy)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /utf-8 /O2")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions -frtti -O2")

    # disable -Wshorten-64-to-32 for apple
    if(APPLE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions -frtti -Wno-shorten-64-to-32")
    endif()
endif()

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src POCKETPY_SRC)

option(PK_USE_CJSON "" OFF)
if(PK_USE_CJSON)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/3rd/cjson)
    add_definitions(-DPK_USE_CJSON)
endif()

option(PK_ENABLE_OS "" OFF)
if(PK_ENABLE_OS)
    add_definitions(-DPK_ENABLE_OS=1)
endif()

option(PK_NO_EXPORT_C_API "" OFF)
if(PK_NO_EXPORT_C_API)
    add_definitions(-DPK_NO_EXPORT_C_API)
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

target_include_directories(${PROJECT_NAME} PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(PK_USE_CJSON)
    target_link_libraries(${PROJECT_NAME} PRIVATE $<BUILD_INTERFACE:cjson>)
endif()

option(PK_INSTALL "Generate the install target" OFF)
if (PK_INSTALL)
    install(
        TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}_target
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
    )

    install(
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ 
        DESTINATION include 
        FILES_MATCHING PATTERN "*.h"
        PATTERN "typings" EXCLUDE
    )
    
    # generate config.cmake
    install(
        EXPORT ${PROJECT_NAME}_target
        FILE ${PROJECT_NAME}-config.cmake
        DESTINATION "share/${PROJECT_NAME}"
    )
endif()
