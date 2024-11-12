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
cmake -S llvm -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug \
-DLLVM_PARALLEL_COMPILE_JOBS=4 -DLLVM_PARALLEL_LINK_JOBS=4 -DLLVM_PARALLEL_TABLEGEN_JOBS=4
echo "[COMPLETE]"
echo "################################################################"

cmake --build build -j 4