#!/bin/zsh

clang -g -O1 -fsanitize=fuzzer -rdynamic fuzz.cpp -o fuzz_app -L/home/stepan/Desktop/fedora_files/fuzzing/Skipper/Skipper_1.0/bin -l:libbn256.a
mv fuzz_app bin/fuzz_app