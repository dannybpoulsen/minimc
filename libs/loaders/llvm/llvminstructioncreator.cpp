#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/Module.h>

#include "llvminstuctioncreator.hpp"
#include "model/instructions.hpp"
#include "model/variables.hpp"
#include "support/exceptions.hpp"
#include <unordered_map>

namespace MiniMC {
  namespace Loaders {

#define LLVMTAC \
  X(Add, Add)   \
  X(Sub, Sub)   \
  X(Mul, Mul)   \
  X(UDiv, UDiv) \
  X(SDiv, SDiv) \
  X(Shl, Shl)   \
  X(LShr, LShr) \
  X(AShr, AShr) \
  X(And, And)   \
  X(Or, Or)     \
  X(Xor, Xor)

#define X(LLVM, OUR)                                                                                                                                                                                                \
  template <>                                                                                                                                                                                                       \
  void translateAndAddInstruction<llvm::Instruction::LLVM>(llvm::Instruction * inst, Val2ValMap & values, std::vector<MiniMC::Model::Instruction> & instr, Types & tt, MiniMC::Model::ConstantFactory_ptr & cfac) { \
    assert(inst->isBinaryOp());                                                                                                                                                                                     \
    instr.push_back(MiniMC::Model::createInstruction<MiniMC::Model::InstructionCode::OUR>({.res = findValue(inst, values, tt, cfac),                                                                                \
                                                                                           .op1 = findValue(inst->getOperand(0), values, tt, cfac),                                                                 \
                                                                                           .op2 = findValue(inst->getOperand(1), values, tt, cfac)}));                                                              \
  }
    LLVMTAC
#undef LLVMTAC
#undef X

    template <>
    void translateAndAddInstruction<llvm::Instruction::Alloca>(llvm::Instruction* inst, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt, MiniMC::Model::ConstantFactory_ptr& cfac) {
      auto alinst = llvm::dyn_cast<llvm::AllocaInst>(inst);
      auto llalltype = alinst->getAllocatedType();
      auto outallsize = tt.getSizeInBytes(llalltype);
      auto res = findValue(inst, values, tt, cfac);
      auto size = cfac->makeIntegerConstant(outallsize, MiniMC::Model::TypeID::I64);
      auto skipsize = cfac->makeIntegerConstant(1, MiniMC::Model::TypeID::I64);
      
      instr.push_back(MiniMC::Model::createInstruction<MiniMC::Model::InstructionCode::Assign>({.res = res,
	    .op1 = tt.sp}));
      instr.push_back(MiniMC::Model::createInstruction<MiniMC::Model::InstructionCode::PtrAdd>({.res = res,
	    .ptr = tt.sp,
	    .skipsize = size,
	    .nbSkips = skipsize
	  }));
    }

    template <>
    void translateAndAddInstruction<llvm::Instruction::Ret>(llvm::Instruction* inst, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt, MiniMC::Model::ConstantFactory_ptr& cfac) {
      auto retinst = llvm::dyn_cast<llvm::ReturnInst>(inst);
      assert(retinst);
      if (retinst->getReturnValue()) {

        auto res = findValue(retinst->getReturnValue(), values, tt, cfac);
        instr.push_back(MiniMC::Model::createInstruction<MiniMC::Model::InstructionCode::Ret>({.value = res}));

      } else {
        instr.push_back(MiniMC::Model::createInstruction<MiniMC::Model::InstructionCode::RetVoid>(0));
      }
    }

    template <>
    void translateAndAddInstruction<llvm::Instruction::Load>(llvm::Instruction* inst, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt, MiniMC::Model::ConstantFactory_ptr& cfac) {
      auto res = findValue(inst, values, tt, cfac);
      auto addr = findValue(inst->getOperand(0), values, tt, cfac);
      instr.push_back(MiniMC::Model::createInstruction<MiniMC::Model::InstructionCode::Load>({
          .res = res,
          .addr = addr,
      }));
    }

    template <>
    void translateAndAddInstruction<llvm::Instruction::Store>(llvm::Instruction* inst, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt, MiniMC::Model::ConstantFactory_ptr& cfac) {
      auto value = findValue(inst->getOperand(0), values, tt, cfac);
      auto addr = findValue(inst->getOperand(1), values, tt, cfac);
      instr.push_back(MiniMC::Model::createInstruction<MiniMC::Model::InstructionCode::Store>({.addr = addr,
                                                                                               .storee = value}));
    }

    template <unsigned>
    void translateIntrinsicCall(llvm::Instruction*, Val2ValMap&, std::vector<MiniMC::Model::Instruction>&, Types&, MiniMC::Model::ConstantFactory_ptr&) {
      throw MiniMC::Support::Exception("Unsupported Intrinsic");
    }

    template <>
    void translateIntrinsicCall<llvm::Intrinsic::stackrestore>(llvm::Instruction* inst, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt, MiniMC::Model::ConstantFactory_ptr& cfac) {
      auto cinst = llvm::dyn_cast<llvm::CallInst>(inst);
      assert(cinst->arg_size() == 1);
      auto value = findValue(*cinst->arg_begin(), values, tt, cfac);
      instr.push_back(MiniMC::Model::createInstruction<MiniMC::Model::InstructionCode::Assign>({.res = tt.sp, .op1 = value}));
    }

    template <>
    void translateIntrinsicCall<llvm::Intrinsic::stacksave>(llvm::Instruction* inst, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt, MiniMC::Model::ConstantFactory_ptr& cfac) {
      assert(inst->getType()->isPointerTy());
      instr.push_back(MiniMC::Model::createInstruction<MiniMC::Model::InstructionCode::Assign>({.res = findValue(inst, values, tt, cfac),.op1 = tt.sp}));
    }

#define SUPPORTEDINTRIN            \
  X(llvm::Intrinsic::stackrestore) \
  X(llvm::Intrinsic::stacksave)    \
  X(llvm::Intrinsic::memcpy)

    template <>
    void translateAndAddInstruction<llvm::Instruction::Call>(llvm::Instruction* inst, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt, MiniMC::Model::ConstantFactory_ptr& cfac) {
      auto cinst = llvm::dyn_cast<llvm::CallInst>(inst);
      auto func = cinst->getCalledFunction();
      assert(func);
      if (func->isIntrinsic()) {
        switch (func->getIntrinsicID()) {
#define X(H)                                                  \
  case H:                                                     \
    translateIntrinsicCall<H>(inst, values, instr, tt, cfac); \
    return;
          SUPPORTEDINTRIN
          default:
            throw MiniMC::Support::Exception("Unsupported Intrinsic");

#undef X
        }
      }
      if (func->getName() == "assert") {
        assert(cinst->arg_size() == 1);
        auto val = findValue(*cinst->arg_begin(), values, tt, cfac);
        if (val->getType()->getTypeID() == MiniMC::Model::TypeID::Bool) {

          instr.push_back(MiniMC::Model::createInstruction<MiniMC::Model::InstructionCode::Assert>({.expr = val}));
        }

        else if (val->getType()->isInteger ()) {
          auto ntype = tt.tfac->makeBoolType();
          auto nvar = tt.stack->addRegister("BVar", ntype);
          instr.push_back(MiniMC::Model::createInstruction<MiniMC::Model::InstructionCode::IntToBool>({.res = nvar, .op1 = val}));
	  instr.push_back(MiniMC::Model::createInstruction<MiniMC::Model::InstructionCode::Assert>({.expr = nvar}));
	}
      }
       else if (func->isDeclaration()) {
        //We don't know what to do for this function
        if (inst->getType()->isIntegerTy()) {
          std::size_t bitwidth = inst->getType()->getIntegerBitWidth();
          auto type = tt.tfac->makeIntegerType(bitwidth);
          MiniMC::Model::Value_ptr min, max;
          MiniMC::Model::Value_ptr res = findValue(inst, values, tt, cfac);

          switch (bitwidth) {
            case 8:
              min = cfac->makeIntegerConstant(std::numeric_limits<MiniMC::BV8>::min(), MiniMC::Model::TypeID::I8);
              max = cfac->makeIntegerConstant(std::numeric_limits<MiniMC::BV8>::max(), MiniMC::Model::TypeID::I8);
              break;
            case 16:
              min = cfac->makeIntegerConstant(std::numeric_limits<MiniMC::BV16>::min(), MiniMC::Model::TypeID::I16);
              max = cfac->makeIntegerConstant(std::numeric_limits<MiniMC::BV16>::max(), MiniMC::Model::TypeID::I16);
              break;
            case 32:
              min = cfac->makeIntegerConstant(std::numeric_limits<MiniMC::BV32>::min(), MiniMC::Model::TypeID::I32);
              max = cfac->makeIntegerConstant(std::numeric_limits<MiniMC::BV32>::max(), MiniMC::Model::TypeID::I32);
              break;
            case 64:
              min = cfac->makeIntegerConstant(std::numeric_limits<MiniMC::BV64>::min(), MiniMC::Model::TypeID::I64);
              max = cfac->makeIntegerConstant(std::numeric_limits<MiniMC::BV64>::max(), MiniMC::Model::TypeID::I64);
              break;
            default:
              throw MiniMC::Support::Exception("Error");
          }

          instr.push_back(MiniMC::Model::createInstruction<MiniMC::Model::InstructionCode::NonDet>({.res = res, .min = min, .max = max}));
        }
      } else {
        std::vector<MiniMC::Model::Value_ptr> params;
        MiniMC::Model::Value_ptr func_ptr = findValue(func, values, tt, cfac);
        MiniMC::Model::Value_ptr res = nullptr;
        if (!inst->getType()->isVoidTy()) {
          res = findValue(inst, values, tt, cfac);
        }
        auto type = tt.tfac->makeIntegerType(64);
	params.push_back (tt.sp);
        for (auto it = cinst->arg_begin(); it != cinst->arg_end(); ++it) {
          params.push_back(findValue(*it, values, tt, cfac));
        }
        instr.push_back(MiniMC::Model::createInstruction<MiniMC::Model::InstructionCode::Call>({.res = res,
                                                                                                .function = func_ptr,
                                                                                                .params = params}));
      }
    }

    size_t calcSkip(llvm::Type* t, size_t index, Types& tt) {
      if (t->isArrayTy()) {
        return tt.getSizeInBytes(static_cast<llvm::ArrayType*>(t)->getElementType()) * index;
      }

      else if (t->isStructTy()) {
        size_t size = 0;
        auto strucTy = static_cast<llvm::StructType*>(t);
        for (size_t i = 0; i < index; ++i) {
          size += tt.getSizeInBytes(strucTy->getElementType(i));
        }
        return size;
      } else {
        throw MiniMC::Support::Exception("Can't calculate size");
      }
    }

    template <>
    void translateAndAddInstruction<llvm::Instruction::ExtractValue>(llvm::Instruction* inst, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt, MiniMC::Model::ConstantFactory_ptr& cfac) {
      llvm::ExtractValueInst* extractinst = llvm::dyn_cast<llvm::ExtractValueInst>(inst);
      auto extractfrom = extractinst->getAggregateOperand();
      if (llvm::Constant* cstextract = llvm::dyn_cast<llvm::Constant>(extractfrom)) {
        llvm::Constant* cur = cstextract;

        for (auto i : extractinst->getIndices()) {
          cur = cur->getAggregateElement(i);
          assert(cur);
        }
        auto value = findValue(cur, values, tt, cfac);
        auto res = findValue(inst, values, tt, cfac);
        instr.push_back(MiniMC::Model::createInstruction<MiniMC::Model::InstructionCode::Assign>({.res = res,
                                                                                                  .op1 = value}));

      } else {
        auto aggre = findValue(extractfrom, values, tt, cfac);
        size_t skip = 0;
        auto cur = extractfrom->getType();
        for (auto i : extractinst->getIndices()) {
          skip += calcSkip(cur, i, tt);
        }

        auto skipee = cfac->makeIntegerConstant(skip, MiniMC::Model::TypeID::I32);
        auto res = findValue(inst, values, tt, cfac);

        instr.push_back(MiniMC::Model::createInstruction<MiniMC::Model::InstructionCode::ExtractValue>({
            .res = res,
            .aggregate = aggre,
            .offset = skipee,
        })

        );
      }
    }

    template <>
    void translateAndAddInstruction<llvm::Instruction::InsertValue>(llvm::Instruction* inst, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt, MiniMC::Model::ConstantFactory_ptr& cfac) {
      llvm::InsertValueInst* insertinst = llvm::dyn_cast<llvm::InsertValueInst>(inst);
      auto insertfrom = insertinst->getAggregateOperand();
      auto insertval = insertinst->getInsertedValueOperand();
      
      auto aggre = findValue(insertfrom, values, tt, cfac);
      auto insertee = findValue(insertval, values, tt, cfac);
      size_t skip = 0;
      auto cur = insertfrom->getType();
      for (auto i : insertinst->getIndices()) {
	skip += calcSkip(cur, i, tt);
      }

      auto skipee = cfac->makeIntegerConstant(skip, MiniMC::Model::TypeID::I32);

      instr.push_back(MiniMC::Model::createInstruction<MiniMC::Model::InstructionCode::InsertValue>({.res = findValue(inst, values, tt, cfac),
	    .aggregate = aggre,
	    .offset = skipee,
	    .insertee = insertee}));
    }

    template <>
    void translateAndAddInstruction<llvm::Instruction::GetElementPtr>(llvm::Instruction* inst, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt, MiniMC::Model::ConstantFactory_ptr& cfac) {
      auto gep = static_cast<llvm::GetElementPtrInst*>(inst);
      MiniMC::Model::Value_ptr skipsize;
      MiniMC::Model::Value_ptr nbSkips;
      MiniMC::Model::Value_ptr address = findValue(gep->getOperand(0), values, tt, cfac);
      ;
      MiniMC::Model::Value_ptr result = findValue(inst, values, tt, cfac);

      auto source = gep->getSourceElementType();
      if (gep->getNumIndices() == 1) {
        nbSkips = findValue(gep->getOperand(1), values, tt, cfac);
        auto size = tt.getSizeInBytes(source);
        skipsize = cfac->makeIntegerConstant(size, nbSkips->getType()->getTypeID ());

      } else {
        auto one = cfac->makeIntegerConstant(1, MiniMC::Model::TypeID::I64);

        if (source->isArrayTy()) {
          auto elemSize = tt.getSizeInBytes(static_cast<llvm::ArrayType*>(source)->getElementType());
          nbSkips = findValue(gep->getOperand(2), values, tt, cfac);
          skipsize = cfac->makeIntegerConstant(elemSize, nbSkips->getType()->getTypeID());
        } else if (source->isStructTy()) {
          auto strucTy = static_cast<llvm::StructType*>(source);
          size_t size = 0;
          auto cinst = llvm::dyn_cast<llvm::ConstantInt>(gep->getOperand(2));
          assert(cinst);
          auto t = cinst->getZExtValue();
          for (size_t i = 0; i < t; ++i) {
            size += tt.getSizeInBytes(strucTy->getElementType(i));
          }
          skipsize = cfac->makeIntegerConstant(size, MiniMC::Model::TypeID::I64);
          nbSkips = one;
        }
      }
      instr.push_back(MiniMC::Model::createInstruction<MiniMC::Model::InstructionCode::PtrAdd>(
          {.res = result,
           .ptr = address,
           .skipsize = skipsize,
           .nbSkips = nbSkips}));
    }

#define LLVMICMP        \
  X(ICMP_SGT, ICMP_SGT) \
  X(ICMP_UGT, ICMP_UGT) \
  X(ICMP_SGE, ICMP_SGE) \
  X(ICMP_UGE, ICMP_UGE) \
  X(ICMP_SLT, ICMP_SLT) \
  X(ICMP_ULT, ICMP_ULT) \
  X(ICMP_SLE, ICMP_SLE) \
  X(ICMP_ULE, ICMP_ULE) \
  X(ICMP_EQ, ICMP_EQ)   \
  X(ICMP_NE, ICMP_NEQ)

    template <>
    void translateAndAddInstruction<llvm::Instruction::ICmp>(llvm::Instruction* inst, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt, MiniMC::Model::ConstantFactory_ptr& cfac) {
      auto ins = llvm::dyn_cast<llvm::ICmpInst>(inst);
      switch (ins->getPredicate()) {
#define X(LLVM, OUR)                                                                                                                                   \
  case llvm::CmpInst::LLVM: {                                                                                                                          \
    instr.push_back(MiniMC::Model::createInstruction<MiniMC::Model::InstructionCode::OUR>({.res = findValue(inst, values, tt, cfac),                   \
                                                                                           .op1 = findValue(inst->getOperand(0), values, tt, cfac),    \
                                                                                           .op2 = findValue(inst->getOperand(1), values, tt, cfac)})); \
    break;                                                                                                                                             \
  }
        LLVMICMP
        default:
          throw MiniMC::Support::Exception("Unsupported Operation");
      }
    }
#undef LLVMICMP
#undef X

#define LLVMCASTOPS     \
  X(Trunc, Trunc)       \
  X(ZExt, ZExt)         \
  X(SExt, SExt)         \
  X(PtrToInt, PtrToInt) \
  X(IntToPtr, IntToPtr) \
  X(BitCast, BitCast)

#define X(LLVM, OUR)                                                                                                                                                                                                \
  template <>                                                                                                                                                                                                       \
  void translateAndAddInstruction<llvm::Instruction::LLVM>(llvm::Instruction * inst, Val2ValMap & values, std::vector<MiniMC::Model::Instruction> & instr, Types & tt, MiniMC::Model::ConstantFactory_ptr & cfac) { \
    instr.push_back(MiniMC::Model::createInstruction<MiniMC::Model::InstructionCode::OUR>({.res = findValue(inst, values, tt, cfac),                                                                                \
                                                                                           .op1 = findValue(inst->getOperand(0), values, tt, cfac)}));                                                              \
  }

    LLVMCASTOPS

#undef X
  } // namespace Loaders
} // namespace MiniMC