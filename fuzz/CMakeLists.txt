cmake_minimum_required(VERSION 3.11)
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang)
project(fuzz_app)

find_package(OpenSSL REQUIRED)

add_executable(fuzz_app fuzz.cpp include_funcs.cpp)

target_compile_options(fuzz_app
            PRIVATE $<$<C_COMPILER_ID:Clang>:-g -O1 -fsanitize=fuzzer>
            )

target_link_libraries(fuzz_app
    PRIVATE $<$<C_COMPILER_ID:Clang>:-fsanitize=fuzzer> OpenSSL::SSL
            )
target_include_directories(fuzz_app PUBLIC "openssl/include")
