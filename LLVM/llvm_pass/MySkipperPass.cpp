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

void add_global_guards(Module & M, Instruction & I) {
  LLVMContext & ctx = M.getContext();
  IRBuilder<> Builder(ctx);
  auto t = Type::getInt32Ty(ctx);

  // Value *start = Builder.CreateBitCast(I, t, "start_address");
  // Value *end   = Builder.CreateBitCast(I, t, "end_address"  );

  
  StoreInst *store_inst = dyn_cast<StoreInst>(&I);
  if (store_inst) {
    // Value* po = store_inst->getPointerOperand();
    // isa<GlobalVariable>(po);
  }

  Value * IValue = &I;
  print_type(IValue);
  if (auto *constant = dyn_cast<Constant>(IValue)) {
      print_constant(constant);
  } else {
      llvm::errs() << "IValue is not a constant!\n";
  }

  // Проверяем, является ли IValue указателем
  if (IValue->getType()->isPointerTy()) {
    // Преобразуем указатель в 8-битный указатель
    // Value *start = Builder.CreateBitCast(IValue, Type::getInt8PtrTy(ctx), "start_address");
    // errs() << "variable with address created\n";
    // print_type(start);
    // GlobalVariable *GV1 = new GlobalVariable(
    //         M, t, false,
    //         GlobalValue::InternalLinkage, start, "inline_asm_start_");
    errs() << "global variable with address created\n";
  } else {
    errs() << "IValue is not a pointer type!\n";
  }

  // Value * IValueNext = I.getNextNode();
  // ругается в runtime на getBitCast (./llvm-project/build/bin/opt+0x32e0a47)
  // видимо преобразовать не могёт
  // верояно надо тип указателя t поменять на что-то другое (мб 32->8)
  // auto v = dyn_cast<llvm::Constant>(IValue);
  // auto t_1 = IValue->getType();
  // Constant * start = ConstantExpr::getBitCast(v , t_1);// t);
  // Constant * end   = ConstantExpr::getBitCast(dyn_cast<llvm::Constant>(IValueNext), t);

  Constant *constValue = ConstantInt::get(Type::getInt32Ty(ctx), 42);
  // Constant *constant_addr = new Constant(IValue->getType(), IValue);
  // print_constant(constant_addr);
  // GlobalVariable *GV1 = new GlobalVariable(
  //           M, constant_addr->getType(), false,
  //           GlobalValue::InternalLinkage, constant_addr, "inline_asm_start_");
  // GlobalVariable *GV2 = new GlobalVariable(
  //           M, t, false,
  //           GlobalValue::InternalLinkage, end  , "inline_asm_end_"  );

  // Builder.SetInsertPoint(&I);
  // Builder.CreateStore(ConstantInt::get(t, 0), GV1);

  // Builder.SetInsertPoint(I.getNextNode());
  // Builder.CreateStore(ConstantInt::get(t, 0), GV2);
}

void storeInstructionAsGlobal(Module &M, Instruction *I, GlobalVariable * GV) {
    LLVMContext & ctx = M.getContext();
    IRBuilder<> Builder(I);

    Type* int8Ty = Type::getInt8Ty(ctx);
    Type* PtrType = int8Ty->getPointerTo();

    // Store the instruction address (casted to i8*) in the global variable
    // auto Addr = Builder.CreateBitCast(I, PtrType);
    Builder.CreateStore(ConstantInt::get(Type::getInt32Ty(ctx), 1), GV);

    // if (I->getType()->isPointerTy()) {
    //     outs() << "I is pointer\n";
    //     // Если I уже указатель, просто выполняем bitcast
    //     auto Addr = Builder.CreateBitCast(I, PtrType);
    //     Builder.CreateStore(Addr, GV);
    // } else {
    //     outs() << "I is not pointer\n";
    //     // Если I не указатель, создаем временную переменную
    //     AllocaInst *tempVar = Builder.CreateAlloca(I->getType(), nullptr, "temp_var");
    //     Builder.CreateStore(I, tempVar); // Сохраняем значение в временной переменной

    //     // Получаем адрес временной переменной и выполняем bitcast
    //     auto Addr = Builder.CreateBitCast(tempVar, PtrType);
    //     Builder.CreateStore(Addr, GV);
    // }
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
            // add_global_guards(M, I);
            storeInstructionAsGlobal(M, &I, GV);
            modified = true;
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

