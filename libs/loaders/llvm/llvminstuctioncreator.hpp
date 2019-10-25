#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Instruction.h>

#include <unordered_map>

#include "support/exceptions.hpp"

namespace MiniMC {
  namespace Loaders {
    MiniMC::Model::Type_ptr getType (llvm::Type* type, MiniMC::Model::TypeFactory_ptr& tfactory);
    uint32_t computeSizeInBytes (llvm::Type* ty,MiniMC::Model::TypeFactory_ptr& tfactory);
    struct Types {
      MiniMC::Model::TypeFactory_ptr tfac;
      MiniMC::Model::Type_ptr getType (llvm::Type* type) {
	return  MiniMC::Loaders::getType (type,tfac);
      }
      std::size_t getSizeInBytes (llvm::Type* type) {
	return computeSizeInBytes (type,tfac);
      }
    };

    
    
	MiniMC::Model::Value_ptr makeConstant (llvm::Constant* constant, Types& tt ) {
	  auto ltype = constant->getType ();
	  if (ltype->isIntegerTy ()) {
	    llvm::ConstantInt* csti = llvm::dyn_cast<llvm::ConstantInt> (constant);
	    assert(csti);
	    auto cst = std::make_shared<MiniMC::Model::IntegerConstant> (csti->getZExtValue ());
	    cst->setType (tt.getType(csti->getType()));
	    return cst;
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
   
    auto alinst = llvm::dyn_cast<llvm::AllocaInst> (inst);
    auto llalltype = alinst->getAllocatedType ();
    auto outalltype = tt.getType (llalltype);
    auto res = findValue (inst,values,tt);		
    auto size = std::make_shared<MiniMC::Model::IntegerConstant> (outalltype->getSize());		
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

    template<>								\
    void translateAndAddInstruction<llvm::Instruction::GetElementPtr> (llvm::Instruction* inst, std::unordered_map<const llvm::Value*,MiniMC::Model::Variable_ptr>& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt) {
      auto gep = static_cast<llvm::GetElementPtrInst*> (inst);
      MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::PtrAdd> builder;
      auto source = gep->getSourceElementType ();
      if (gep->getNumIndices () == 1) {
	auto size = tt.getSizeInBytes(source);
	auto sizec = std::make_shared<MiniMC::Model::IntegerConstant> (size);
	auto val = findValue (gep->getOperand (1),values,tt);
	auto ptr = findValue (gep->getOperand (0),values,tt);
	sizec->setType (val->getType ());
	builder.setSkipSize (sizec);
	builder.setValue (val);
	builder.setAddress (ptr);
      }
      else {
	auto i64 = tt.tfac->makeIntegerType (64);
	auto one = std::make_shared<MiniMC::Model::IntegerConstant> (1);
	
	if (source->isArrayTy () ) {
	  auto elemType = tt.getType ( static_cast<llvm::ArrayType*> (source)->getElementType());
	  auto sizec = std::make_shared<MiniMC::Model::IntegerConstant> (elemType->getSize());
	  auto val = findValue (gep->getOperand (2),values,tt);
	  auto ptr = findValue (gep->getOperand (0),values,tt);
	  sizec->setType (val->getType ());
	  builder.setSkipSize (sizec);
	  builder.setValue (val);
	  builder.setAddress (ptr);
	}
	else if (source->isStructTy ()) {
	  auto strucTy = static_cast<llvm::StructType*> (source);
	  auto ptr = findValue (gep->getOperand (0),values,tt);
	  size_t size = 0;
	  auto cinst = llvm::dyn_cast<llvm::ConstantInt> (gep->getOperand(2));
	  assert (cinst);
	  auto t = cinst->getZExtValue ();
	  for (size_t i = 0; i < t; ++i) {
	    size+=tt.getSizeInBytes (strucTy->getElementType(i));
	  }
	  auto sizec = std::make_shared<MiniMC::Model::IntegerConstant> (size);
	  sizec->setType (i64);
	  builder.setValue (one);
	  builder.setAddress (ptr);
	  builder.setSkipSize (sizec);
	}

      }
      
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






