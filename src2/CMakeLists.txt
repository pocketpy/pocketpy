cmake_minimum_required(VERSION 3.21)

project(pocketpyrepl)

target_sources(main PRIVATE ${CMAKE_CURRENT_LIST_DIR}/main.cpp)
target_include_directories(main PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../include)
target_compile_features(main PRIVATE cxx_std_17)
