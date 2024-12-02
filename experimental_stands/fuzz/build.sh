#!/bin/zsh

mkdir -p build/bin
mkdir -p build/o
mkdir -p build/ll

clang -g -O1 -std=c++17 -fsanitize=fuzzer -rdynamic fuzz.cpp -o fuzz_app -L../bin/ -l:libbn256.a
# compile llvm-ir code
# clang -g -O1 -S -std=c++17 -rdynamic -emit-llvm fuzz.cpp -o factorial.ll -L../bin/ -l:libbn256.a
mv fuzz_app bin/fuzz_app

# clang -std=c++17 -fsanitize=fuzzer -Xclang -load -Xclang ./llvm_pass/SkipperPass.o \
# ./build/object/fuzz.o ./build/object/factorial.o -o a

# clang -std=c++17 -O3 -funroll-loops -Wall -D_FORTIFY_SOURCE=2 -g -Wno-pointer-sign -fPIC -c Skipper.cpp -o SkipperPass.o \
# -I/usr/include/llvm-11/ -I/usr/include/llvm-c-11