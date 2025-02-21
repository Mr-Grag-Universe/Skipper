#!/bin/zsh

#####################################################################################
#   не забудьте вставить                                                            #
#   FUNCTION_PASS("skipper", SkipperPass()) в llvm/lib/Passes/PassRegistry.def      #
#   и                                                                               #
#   #include "llvm/Transforms/Utils/Skipper.h" в llvm/lib/Passes/PassBuilder.cpp    #
#   и                                                                               #
#   .cpp в llvm/lib/Transforms/Utils/CMakeLists.txt                                 #
#####################################################################################

PROJECT_ROOT_PATH="/home/debian/Skipper/LLVM/"
LLVM_ROOT_PATH="${PROJECT_ROOT_PATH}llvm-project/"
CUSTOM_LLVM_PATH_DIR="${PROJECT_ROOT_PATH}llvm_pass/"

cp ${CUSTOM_LLVM_PATH_DIR}MySkipperPass.cpp ${LLVM_ROOT_PATH}llvm/lib/Transforms/Utils/Skipper.cpp
cp ${CUSTOM_LLVM_PATH_DIR}MySkipperPass.h   ${LLVM_ROOT_PATH}llvm/include/llvm/Transforms/Utils/Skipper.h
