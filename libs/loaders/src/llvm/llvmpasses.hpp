struct InstructionNamer : public llvm::PassInfoMixin<InstructionNamer> {
      llvm::PreservedAnalyses run(llvm::Function& F, llvm::FunctionAnalysisManager&) {
        for (auto& Arg : F.args())
          if (!Arg.hasName())
            Arg.setName("arg");
          else
            Arg.setName(Arg.getName());
        for (llvm::BasicBlock& BB : F) {
          if (!BB.hasName())
            BB.setName("bb");
          else {
            BB.setName(BB.getName());
          }
          for (llvm::Instruction& I : BB)
            if (!I.hasName())
              I.setName("tmp");
            else
              I.setName(I.getName());
        }
        return llvm::PreservedAnalyses::all();
      }
    };

    struct ConstExprRemover : public llvm::PassInfoMixin<ConstExprRemover> {
      llvm::PreservedAnalyses run(llvm::Function& F, llvm::FunctionAnalysisManager&) {

        for (llvm::BasicBlock& BB : F) {
          for (llvm::Instruction& I : BB) {
            handle(&I);
          }
        }
        return llvm::PreservedAnalyses::none();
      }

      void handle(llvm::Instruction* inst) {
        auto ops = inst->getNumOperands();
        for (std::size_t i = 0; i < ops; i++) {
          auto op = inst->getOperand(i);
          llvm::ConstantExpr* oop = nullptr;
          if ((oop = llvm::dyn_cast<llvm::ConstantExpr>(op))) {
            auto ninst = oop->getAsInstruction();
            ninst->insertBefore(inst);
            inst->setOperand(i, ninst);
            handle(ninst);
          }
        }
      }
    };

    struct RemoveUnusedInstructions : public llvm::PassInfoMixin<RemoveUnusedInstructions> {
      llvm::PreservedAnalyses run(llvm::Function& F, llvm::FunctionAnalysisManager&) {
        bool changed = true;
        do {
          changed = false;
          for (llvm::BasicBlock& BB : F) {
            for (auto I = BB.begin(), E = BB.end(); I != E; ++I) {
              llvm::Instruction& inst = *I;
              if (!inst.getNumUses() &&
                  deleteable(&inst)) {
                I = inst.eraseFromParent();
                changed = true;
              }
            }
          }
        } while (changed);

        return llvm::PreservedAnalyses::none();
      }

      bool deleteable(llvm::Instruction* inst) {
        switch (inst->getOpcode()) {
          case llvm::Instruction::Store:
          case llvm::Instruction::Call:
          case llvm::Instruction::Unreachable:
            return false;
          default:
            return !inst->isTerminator();
        }
      }
    };

    struct GetElementPtrSimplifier : public llvm::PassInfoMixin<InstructionNamer> {
      llvm::PreservedAnalyses run(llvm::Function& F, llvm::FunctionAnalysisManager&) {
        llvm::LLVMContext& context = F.getContext();
        auto i32Type = llvm::IntegerType::getInt32Ty(context);
        auto zero = llvm::ConstantInt::get(i32Type, 0);
        for (llvm::BasicBlock& BB : F) {
          llvm::BasicBlock::iterator BI = BB.begin();
          llvm::BasicBlock::iterator BE = BB.end();
          while (BI != BE) {
            auto& I = *BI++;
            if (I.getOpcode() == llvm::Instruction::GetElementPtr) {
              llvm::GetElementPtrInst* inst = llvm::dyn_cast<llvm::GetElementPtrInst>(&I);
              llvm::Value* indexList[1] = {inst->getOperand(1)};
              auto prev = llvm::GetElementPtrInst::Create(inst->getSourceElementType (), inst->getOperand(0), llvm::ArrayRef<llvm::Value*>(indexList, 1), "_gep__", inst);
              const std::size_t E = inst->getNumOperands();
              for (std::size_t oper = 2; oper < E; ++oper) {
                llvm::Value* indexList[2] = {zero, inst->getOperand(oper)};
                prev = llvm::GetElementPtrInst::Create(prev->getResultElementType (), prev, llvm::ArrayRef<llvm::Value*>(indexList, 2), "_gep__", inst);
              }
              I.replaceAllUsesWith(prev);
              I.eraseFromParent();
            }
          }
        }
        return llvm::PreservedAnalyses::none();
      }
    };
