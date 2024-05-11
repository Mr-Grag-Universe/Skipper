#!/bin/zsh

clang -g -O1 -fsanitize=fuzzer -rdynamic fuzz.cpp -o fuzz_app -Lbin -l:libbn256.a
mv fuzz_app bin/fuzz_app