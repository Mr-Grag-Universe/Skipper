cmake_minimum_required(VERSION 3.11)

# Укажите стандарт C++
set(CMAKE_CXX_STANDARD 17)           # Замените 11 на нужную вам версию (14, 17, 20 и т.д.)
set(CMAKE_CXX_STANDARD_REQUIRED ON)   # Обязательное использование указанного стандарта

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang)
project(fuzz_app)

add_executable(fuzz_app fuzz.cpp)
