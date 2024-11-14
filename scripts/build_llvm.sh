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

# ВАЖНО!!! версии llvm, opt и clang должны быть одинаковыми, ибо старый и новый форматы не совместимы!