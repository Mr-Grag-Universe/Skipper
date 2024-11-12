#ifndef LLVM_TRANSFORMS_SKIPPER_H
#define LLVM_TRANSFORMS_SKIPPER_H

#include "llvm/IR/PassManager.h"

namespace llvm {
    class SkipperPass : public PassInfoMixin<SkipperPass> {
        public:
        PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
    };
}

#endif // LLVM_TRANSFORMS_SKIPPER_H