cmake_minimum_required(VERSION 3.16)

project(cvm CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES src/*.cpp)

add_executable(cvm ${SOURCES})

target_include_directories(cvm PRIVATE src)

target_compile_options(cvm PRIVATE -Wall -Wextra -O2)