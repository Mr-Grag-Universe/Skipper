#ifndef LLVM_TRANSFORMS_SKIPPER_PASS_H
#define LLVM_TRANSFORMS_SKIPPER_PASS_H

#include "llvm/IR/PassManager.h"
#include "llvm/IR/Module.h"

namespace llvm {

class SkipperPass : public PassInfoMixin<SkipperPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
  // PreservedAnalyses run(Module &M);
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_HELLONEW_HELLOWORLD_H