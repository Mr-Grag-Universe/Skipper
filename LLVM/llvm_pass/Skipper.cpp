#include "llvm/IR/InlineAsm.h"
#include "llvm/Support/raw_ostream.h"
// #include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Metadata.h"
#include "llvm/Pass.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

using namespace llvm;

namespace {
    class SkipperPass : public ModulePass {
        public:
        static char ID;
        SkipperPass() : ModulePass(ID) {}
        bool runOnModule(Module &M) override {
            bool modified = false;

            // Итерируемся по всем функциям в модуле
            for (Function &F : M) {
                // Проверяем, является ли функция определенной (не является декларацией)
                if (!F.isDeclaration()) {
                    // Итерируемся по всем базовым блокам в функции
                    for (BasicBlock &BB : F) {
                        // Итерируемся по всем инструкциям в базовом блоке
                        for (Instruction &I : BB) {
                            // Проверяем, содержит ли инструкция ассемблерный код
                            if (auto *asmInst = dyn_cast<InlineAsm>(&I)) {
                                // Получаем текст ассемблерного кода
                                StringRef asmCode = asmInst->getAsmString();

                                // Создаем метаданные для начала и конца секции
                                // MDNode *startMD = MDNode::get(M.getContext(), {MDString::get(M.getContext(), "asm_start")});
                                // MDNode *endMD = MDNode::get(M.getContext(), {MDString::get(M.getContext(), "asm_end")});

                                // Добавляем метаданные к инструкции
                                // I.add
                                // I.addMetadata("asm_start", *startMD);
                                // I.addMetadata("asm_end", *endMD);

                                // Выводим информацию о найденной секции
                                errs() << "Found asm section in function " << F.getName() << ": " << asmCode << "\n";
                                modified = true;
                            }
                        }
                    }
                }
            }

            return modified;
        }
    };
}

char SkipperPass::ID = 0;
static RegisterPass<SkipperPass> X("skipper-pass", "Find ASM Sections Pass", false, false);