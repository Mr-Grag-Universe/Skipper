#include "llvm/Transforms/Utils/Skipper.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Pass.h"
#include "llvm/IR/TypedPointerType.h"

using namespace llvm;

static long long asm_section_index = 0;

void print_type(Value *value) {
    if (value) {
        llvm::outs() << "value_type: ";
        value->getType()->print(llvm::outs());
        llvm::outs() << "\n"; // Печатаем новую строку для удобства
    } else {
        llvm::outs() << "Value is null\n";
    }
}
void print_constant(Constant *constant) {
    if (constant) {
        llvm::outs() << "constant_type: ";
        constant->getType()->print(llvm::outs());
        llvm::outs() << "\n"; // Печатаем новую строку для удобства
    } else {
        llvm::outs() << "Constant is null\n";
    }
}

void storeInstructionAsGlobal(Module &M, Instruction *I, GlobalVariable * GV) {
    LLVMContext & ctx = M.getContext();
    IRBuilder<> Builder(I);

    Type* int8Ty = Type::getInt8Ty(ctx);
    Type* PtrType = int8Ty->getPointerTo();

    IRBuilder<> Builder_1(I);
    Builder_1.CreateStore(ConstantInt::get(Type::getInt32Ty(ctx), 1), GV);
    IRBuilder<> Builder_2(I->getNextNode());
    Builder_2.CreateStore(ConstantInt::get(Type::getInt32Ty(ctx), 0), GV);
}

PreservedAnalyses SkipperPass::run(Module &M, ModuleAnalysisManager &AM) {
  LLVMContext & ctx = M.getContext();
  GlobalVariable *GV = new GlobalVariable(
      M, Type::getInt32Ty(ctx), false, llvm::GlobalValue::ExternalLinkage,
      ConstantInt::get(Type::getInt32Ty(ctx), 0), "instr_global");

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
            storeInstructionAsGlobal(M, &I, GV);
            modified = true;
          }
        }
      }
    }
  }
  return modified ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

