#include "llvm/Transforms/Utils/Skipper.h"

using namespace llvm;

PreservedAnalyses SkipperPass::run(Function &F, FunctionAnalysisManager &AM) {
    errs() << F.getName() << "\n";
    return PreservedAnalyses::all();
}