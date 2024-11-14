#include "llvm/Transforms/Utils/MySkipperPass.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"

using namespace llvm;

PreservedAnalyses SkipperPass::run(Module &M, ModuleAnalysisManager &AM) {
  bool modified = false;
  for (auto & F : M) {
    errs() << "Skipper func: " << F.getName() << "\n";
    if (!F.isDeclaration()) {
      errs() << "function is not a declaration!\n";
    }
    for (auto & B : F) {
      for (auto & I : B) {
        I.print(errs()); // Выводит инструкцию в стандартный поток ошибок
        errs() << "\n";  // Добавляем новую строку для удобства

        if (auto *Call = dyn_cast<CallInst>(&I)) {
          if (auto *IA = dyn_cast<InlineAsm>(Call->getCalledOperand()->stripPointerCasts())) {
            // Если это inline assembly, выводим его
            errs() << "Found inline assembly: " << IA->getAsmString() << "\n";
          }
        }

        // if (auto *asmInst = dyn_cast<InlineAsm>(&I)) {
        //   StringRef asmCode = asmInst->getAsmString();
        //   errs() << "Found asm section: " << asmCode << "\n";

        //   // modified = true;
        // }
      }
    }
  }
  return modified ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

