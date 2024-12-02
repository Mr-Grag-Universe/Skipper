#!/bin/zsh

cd ~/
echo "################################################################"
echo "LLVM repository clonning..."
git clone --depth 1 https://github.com/llvm/llvm-project.git
echo "[COMPLETE]"
cd llvm-project
echo "################################################################"

echo "################################################################"
echo "CMake configuration..."
cmake -S llvm -B build -G "Unix Makefiles" \
-DCMAKE_BUILD_TYPE=MinSizeRel \
-DLLVM_PARALLEL_COMPILE_JOBS=3 \
-DLLVM_PARALLEL_LINK_JOBS=3 \
-DLLVM_PARALLEL_TABLEGEN_JOBS=3 \
-DLLVM_ENABLE_PROJECTS="clang;lld;opt"
echo "[COMPLETE]"
echo "################################################################"

cmake --build build -j 3

# запускать надо так:
# ./llvm-project/build/bin/clang -emit-llvm -S cpp_file.cpp -o ll_file.ll
# ./llvm-project/build/bin/opt -passes=skipper ll_file.ll -disable-output
# cmake -S llvm -B build -G "Unix Makefiles" 
# -DCMAKE_BUILD_TYPE=MinSizeRel -DLLVM_PARALLEL_COMPILE_JOBS=6 
# -DLLVM_PARALLEL_LINK_JOBS=6 -DLLVM_PARALLEL_TABLEGEN_JOBS=6 
# -DLLVM_ENABLE_PROJECTS="clang;lld;clang-tools-extra;compiler-rt" -DLLVM_INSTALL_UTILS=ON

# LLVM/llvm-project/build/bin/clang -fsanitize=fuzzer -g -std=c++17 -O1 -c experimental_stands/fuzz/fuzz.cpp -rdynamic -o fuzz.o
# LLVM/llvm-project/build/bin/clang -fsanitize=fuzzer fuzz.o -g -std=c++17 -O1 -rdynamic -o fuzz

# LLVM/llvm-project/build/bin/clang -S -emit-llvm experimental_stands/fuzz/fuzz.cpp
# LLVM/llvm-project/build/bin/opt -passes=skipper -S fuzz.ll -o fuzz_new.ll
# LLVM/llvm-project/build/bin/clang -c experimental_stands/fuzz/fuzz_new.ll -o fuzz.o
# LLVM/llvm-project/build/bin/clang -fsanitize=fuzzer fuzz.o -g -std=c++17 -O1 -rdynamic -o fuzz

# ВАЖНО!!! версии llvm, opt и clang должны быть одинаковыми, ибо старый и новый форматы не совместимы!