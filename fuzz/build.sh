#!/bin/zsh

clang -g -O1 -std=c++17 -fsanitize=fuzzer -rdynamic fuzz.cpp -o fuzz_app -L../bin/ -l:libbn256.a
mv fuzz_app bin/fuzz_app
