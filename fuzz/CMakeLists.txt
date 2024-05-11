cmake_minimum_required(VERSION 3.11)
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang)
project(fuzz_app)

add_executable(fuzz_app fuzz.cpp)
