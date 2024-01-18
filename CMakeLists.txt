cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
project(Cpp VERSION 1.9.0)

if(NOT MSVC)
    add_compile_options(-Wall -Wextra)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(v1 lam.cpp)
add_executable(v2 lam.cpp)

target_compile_definitions(v2 PUBLIC ENABLE_SMALLER_SUBTREE)
