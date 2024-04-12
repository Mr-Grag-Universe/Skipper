#!/bin/zsh

nasm -f elf64 func.asm -o func.o
clang -fsanitize=fuzzer-no-link -w -c -rdynamic fuzz.cpp -o fuzz.o 
clang -g -O1 -fsanitize=fuzzer -lssl -lcrypto -rdynamic fuzz.o func.o -o fuzz_app 