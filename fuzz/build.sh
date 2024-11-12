#!/bin/zsh

mkdir -p build/bin
mkdir -p build/o
mkdir -p build/ll

clang -g -O1 -std=c++17 -fsanitize=fuzzer -rdynamic fuzz.cpp -o fuzz_app -L../bin/ -l:libbn256.a
# compile llvm-ir code
# clang -g -O1 -S -std=c++17 -rdynamic -emit-llvm fuzz.cpp -o factorial.ll -L../bin/ -l:libbn256.a
mv fuzz_app bin/fuzz_app
