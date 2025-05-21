cmake_minimum_required(VERSION 3.10)

project("musl_math")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_LIST_DIR}/src MUSL_LIBC_MATH_SRC)
if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Ox /Oi- /wd4244 /wd4723")
    # mute warning C4723: div by 0 because some function returns nan when a input is nan.
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -fexcess-precision=standard -ffp-contract=off")
endif()

add_library(${PROJECT_NAME} STATIC ${MUSL_LIBC_MATH_SRC})
