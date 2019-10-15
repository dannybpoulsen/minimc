#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Instruction.h>

#include <unordered_map>

#include "support/exceptions.hpp"

namespace MiniMC {
  namespace Loaders {
	struct Types {
	  MiniMC::Model::Type_ptr i8;
	  MiniMC::Model::Type_ptr i16;
	  MiniMC::Model::Type_ptr i32;
	  MiniMC::Model::Type_ptr i64;
	};

	
	MiniMC::Model::Value_ptr makeConstant (llvm::Constant* constant, Types& tt ) {
	  auto ltype = constant->getType ();
	  if (ltype->isIntegerTy ()) {
	    llvm::ConstantInt* csti = llvm::dyn_cast<llvm::ConstantInt> (constant);
	    assert(csti);
	    return std::make_shared<MiniMC::Model::IntegerConstant> (csti->getZExtValue ());
	  }
	  throw MiniMC::Support::Exception ("Error");
	}

	MiniMC::Model::Value_ptr findValue (llvm::Value* val, std::unordered_map<const llvm::Value*,MiniMC::Model::Variable_ptr>& values, Types& tt ) {
	  llvm::Constant* cst = llvm::dyn_cast<llvm::Constant> (val);
	  if (cst)
		return makeConstant (cst,tt);
	  else {
		return values.at(val); 
	  }
	}
	
	
#define LLVMTAC									\
	X(Add,Add)									\
	X(Sub,Sub)									\
	X(Mul,Mul)									\
	X(UDiv,UDiv)								\
	X(SDiv,SDiv)								\
	X(Shl,Shl)									\
	X(LShr,LShr)								\
	X(AShr,AShr)								\
	X(And,And)									\
	X(Or,Or)									\
	X(Xor,Xor)									\
   
	
	template<unsigned>
	void translateAndAddInstruction (llvm::Instruction*, std::unordered_map<const llvm::Value*,MiniMC::Model::Variable_ptr>& values, std::vector<MiniMC::Model::Instruction>&, Types& ) {
	  throw MiniMC::Support::Exception ("Error");
	}

#define X(LLVM,OUR)							\
    template<>								\
	void translateAndAddInstruction<llvm::Instruction::LLVM> (llvm::Instruction* inst, std::unordered_map<const llvm::Value*,MiniMC::Model::Variable_ptr>& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt) { \
	MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::OUR> builder; \
	assert(inst->isBinaryOp ());										\
	auto res = findValue (inst,values,tt);				\
	auto left = findValue (inst->getOperand (1),values,tt);		\
	auto right = findValue (inst->getOperand (2),values,tt);	\
	builder.setRes (res);						\
	builder.setLeft (left);						\
	builder.setRight (right);					\
	instr.push_back(builder.BuildInstruction ());						\
  }
  LLVMTAC
#undef LLVMTAC
#undef X
  
  template<>								
  void translateAndAddInstruction<llvm::Instruction::Alloca> (llvm::Instruction* inst, std::unordered_map<const llvm::Value*,MiniMC::Model::Variable_ptr>& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt) { 
    MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::Alloca> builder; 
    assert(inst->isBinaryOp ());					
    auto res = findValue (inst,values,tt);		
    auto size = findValue (inst->getOperand (0),values,tt);		
    builder.setRes (res);						
    builder.setSize (size);						
    instr.push_back(builder.BuildInstruction ());			
  }

    template<>								\
    void translateAndAddInstruction<llvm::Instruction::Load> (llvm::Instruction* inst, std::unordered_map<const llvm::Value*,MiniMC::Model::Variable_ptr>& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt) { 
      MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::Load> builder; 
      auto res = findValue (inst,values,tt);		
      auto addr = findValue (inst->getOperand (0),values,tt);		
      builder.setRes (res);						
      builder.setAddress (addr);						
      instr.push_back(builder.BuildInstruction ());			
    }
    
    template<>								\
    void translateAndAddInstruction<llvm::Instruction::Store> (llvm::Instruction* inst, std::unordered_map<const llvm::Value*,MiniMC::Model::Variable_ptr>& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt) { 
      MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::Store> builder; 
      auto value = findValue (inst->getOperand (0),values,tt);		
      auto addr = findValue (inst->getOperand (1),values,tt);		
      builder.setValue (value);						
      builder.setAddress (addr);						
      instr.push_back(builder.BuildInstruction ());			
    }

#define LLVMICMP					\
    X(ICMP_SGT,ICMP_SGT)				\
    X(ICMP_UGT,ICMP_UGT)				\
    X(ICMP_SGE,ICMP_SGE)				\
    X(ICMP_UGE,ICMP_UGE)				\
    X(ICMP_SLT,ICMP_SLT)				\
    X(ICMP_ULT,ICMP_ULT)				\
    X(ICMP_SLE,ICMP_SLE)				\
    X(ICMP_ULE,ICMP_ULE)				

    template<>								\
    void translateAndAddInstruction<llvm::Instruction::ICmp> (llvm::Instruction* inst, std::unordered_map<const llvm::Value*,MiniMC::Model::Variable_ptr>& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt) { 
      auto ins = llvm::dyn_cast<llvm::ICmpInst> (inst);
      switch (ins->getPredicate ()) {
#define X(LLVM,OUR)				\
	case llvm::CmpInst::LLVM:	{				\
	  MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::LLVM> builder; \
	  auto res = findValue (inst,values,tt);			\
	  auto left = findValue (inst->getOperand (0),values,tt);	\
	  auto right = findValue (inst->getOperand (1),values,tt);	\
	  builder.setRes (res);						\
	  builder.setLeft (left);					\
	  builder.setRight (right);					\
	  instr.push_back(builder.BuildInstruction ());			\
	  break;			\
	}	
	LLVMICMP
	
	    }	
    }
#undef LLVMICMP
#undef X

#define LLVMCASTOPS				\
    X(Trunc,Trunc)					\
    X(ZExt,ZExt)					\
    X(SExt,SExt)					\
    X(PtrToInt,PtrToInt)				\
    X(IntToPtr,IntToPtr)				\
    X(BitCast,BitCast)					\

    #define X(LLVM,OUR)							\
    template<>								\
	void translateAndAddInstruction<llvm::Instruction::LLVM> (llvm::Instruction* inst, std::unordered_map<const llvm::Value*,MiniMC::Model::Variable_ptr>& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt) { \
    MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::OUR> builder; \
	auto res = findValue (inst,values,tt);				\
	auto left = findValue (inst->getOperand (1),values,tt);		\
	builder.setRes (res);						\
	builder.setCastee (left);					\
	instr.push_back(builder.BuildInstruction ());			\
    }
    LLVMCASTOPS
    #undef X
  }
}






