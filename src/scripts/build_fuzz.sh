#!/bin/zsh

# cd fuzz/build
# cmake ../
# make

# clang -g -O1 -fsanitize=fuzzer fuzz.cpp -o fuzz_app
# cp fuzz_app ../../fuzz_app 

cd fuzz
./build.sh
cp bin/fuzz_app ../bin/fuzz_app