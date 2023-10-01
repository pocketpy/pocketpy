cmake_minimum_required(VERSION 3.10)

project(box2d)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../../pocketpy/include)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src/collision BOX2D_SRC_0)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src/common BOX2D_SRC_1)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src/dynamics BOX2D_SRC_2)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src/rope BOX2D_SRC_3)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src BOX2D_BINDINGS_SRC)

set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(
    box2d
    STATIC
    ${BOX2D_SRC_0} ${BOX2D_SRC_1} ${BOX2D_SRC_2} ${BOX2D_SRC_3}
    ${BOX2D_BINDINGS_SRC}
)