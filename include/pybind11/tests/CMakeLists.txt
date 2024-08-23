cmake_minimum_required(VERSION 3.10)
project(PKBIND_TEST)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../../.." "${CMAKE_CURRENT_BINARY_DIR}/pocketpy")

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
  DOWNLOAD_EXTRACT_TIMESTAMP true
)

if(WIN32)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

FetchContent_MakeAvailable(googletest)

file(GLOB CPP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_executable(PKBIND_TEST ${CPP_SOURCES})

target_include_directories(PKBIND_TEST PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../..")
target_link_libraries(PKBIND_TEST PRIVATE pocketpy gtest_main)