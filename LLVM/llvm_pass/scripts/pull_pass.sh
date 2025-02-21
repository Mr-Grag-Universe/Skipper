#!/bin/zsh

PROJECT_ROOT_PATH="/workspaces/Skipper/"
LLVM_ROOT_PATH="${PROJECT_ROOT_PATH}llvm-project/"
CUSTOM_LLVM_PATH_DIR="${PROJECT_ROOT_PATH}fuzz/llvm_pass/"

cp ${LLVM_ROOT_PATH}llvm/lib/Transforms/Utils/MySkipperPass.cpp ${CUSTOM_LLVM_PATH_DIR}MySkipperPass.cpp
cp ${LLVM_ROOT_PATH}llvm/include/llvm/Transforms/Utils/MySkipperPass.h ${CUSTOM_LLVM_PATH_DIR}MySkipperPass.h