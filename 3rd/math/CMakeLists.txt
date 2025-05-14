cmake_minimum_required(VERSION 3.10)

project("musl_math")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_LIST_DIR}/src MUSL_LIBC_MATH_SRC)
if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /fp:precise /Od /Oi-")
    # mute warning C4723: div by 0 because some function returns nan when a input is nan.
    add_compile_options( /wd4723)
elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffp-model=strict")
endif()

add_library(${PROJECT_NAME} STATIC ${MUSL_LIBC_MATH_SRC})


# if(MSVC)
#     target_compile_options(${PROJECT_NAME} PRIVATE /fp:precise /Od)
#     message("MUSL_LIBC_MATH: MSVC set")
# elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang")
#     target_compile_options(${PROJECT_NAME} PRIVATE -ffp-model=strict)
#     message("MUSL_LIBC_MATH: CLANG set")
# elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
#     message("MUSL_LIBC_MATH: gcc set")
# endif()   
