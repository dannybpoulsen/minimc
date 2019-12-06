#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Instruction.h>

#include <unordered_map>

#include "support/exceptions.hpp"


namespace MiniMC {
  namespace Loaders {

    using Val2ValMap = std::unordered_map<const llvm::Value*, MiniMC::Model::Value_ptr>;
    
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

    MiniMC::Model::Value_ptr findValue (llvm::Value* val, Val2ValMap& values, Types& tt );
    
    
    MiniMC::Model::Value_ptr makeConstant (llvm::Value* val, Types& tt ) {
	  auto constant = llvm::dyn_cast<llvm::Constant> (val);
	  assert(constant);
	  auto ltype = constant->getType ();
	  if (ltype->isIntegerTy ()) {
	    llvm::ConstantInt* csti = llvm::dyn_cast<llvm::ConstantInt> (constant);
	    assert(csti);
	    auto cst = std::make_shared<MiniMC::Model::IntegerConstant> (csti->getZExtValue ());
	    cst->setType (tt.getType(csti->getType()));
	    return cst;
	  }
	  else if (ltype->isStructTy()) {
	    std::vector<MiniMC::Model::Value_ptr> vals;
	    const size_t oper = constant->getNumOperands ();
	    for (size_t i = 0; i < oper;++i) {
	      auto elem = constant->getOperand(i);
	      vals.push_back(makeConstant(elem,tt));
	    }
	    auto type = tt.getType (constant->getType ());
	    auto cst = std::make_shared<MiniMC::Model::AggregateConstant> (vals,false);
	    cst->setType (type);
	    return cst;
	  }

	  else if (ltype->isArrayTy()) {
	    std::vector<MiniMC::Model::Value_ptr> vals;
	    const size_t oper = constant->getNumOperands ();
	    for (size_t i = 0; i < oper;++i) {
	      auto elem = constant->getOperand(i);
	      vals.push_back(makeConstant(elem,tt));
	    }
	    auto type = tt.getType (constant->getType());
	    auto cst = std::make_shared<MiniMC::Model::AggregateConstant> (vals,true);
	    cst->setType (type);
	    return cst;
	  }
	  
	  throw MiniMC::Support::Exception ("Error");
	}
    

	
	
#define LLVMTAC								\
    X(Add,Add)								\
    X(Sub,Sub)								\
    X(Mul,Mul)								\
    X(UDiv,UDiv)							\
    X(SDiv,SDiv)							\
    X(Shl,Shl)								\
    X(LShr,LShr)							\
    X(AShr,AShr)							\
    X(And,And)								\
    X(Or,Or)								\
    X(Xor,Xor)								\
    
	
	template<unsigned>
	void translateAndAddInstruction (llvm::Instruction*, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>&, Types& ) {
	  throw MiniMC::Support::Exception ("Error");
	}

#define X(LLVM,OUR)							\
    template<>								\
	void translateAndAddInstruction<llvm::Instruction::LLVM> (llvm::Instruction* inst, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt) { \
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
  void translateAndAddInstruction<llvm::Instruction::Alloca> (llvm::Instruction* inst, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt) { 
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
    
    template<>								
    void translateAndAddInstruction<llvm::Instruction::Ret> (llvm::Instruction* inst, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt) { 
      auto retinst = llvm::dyn_cast<llvm::ReturnInst> (inst);
      assert(retinst);
      if (retinst->getReturnValue ()) {
	MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::Ret> builder;
	auto res = findValue (retinst->getReturnValue(),values,tt);		
	builder.setRetValue (res);
	instr.push_back(builder.BuildInstruction ());
      }
      else {
	MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::RetVoid> builder;
	instr.push_back(builder.BuildInstruction ());
      }			
    }

    template<>								\
    void translateAndAddInstruction<llvm::Instruction::Load> (llvm::Instruction* inst, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt) { 
      MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::Load> builder; 
      auto res = findValue (inst,values,tt);		
      auto addr = findValue (inst->getOperand (0),values,tt);		
      builder.setRes (res);						
      builder.setAddress (addr);						
      instr.push_back(builder.BuildInstruction ());			
    }
    
    template<>								\
    void translateAndAddInstruction<llvm::Instruction::Store> (llvm::Instruction* inst, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt) { 
      MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::Store> builder; 
      auto value = findValue (inst->getOperand (0),values,tt);		
      auto addr = findValue (inst->getOperand (1),values,tt);		
      builder.setValue (value);						
      builder.setAddress (addr);						
      instr.push_back(builder.BuildInstruction ());			
    }

    template<>								\
    void translateAndAddInstruction<llvm::Instruction::Call> (llvm::Instruction* inst, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt) { 
      auto cinst = llvm::dyn_cast<llvm::CallInst> (inst);
      auto func = cinst->getCalledFunction ();
      assert(func);
      if (func->getName () == "assert") {
	MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::Assert> builder;
	assert(cinst->arg_size () == 1);
	builder.setAssert (findValue(*cinst->arg_begin(),values,tt));
	instr.push_back(builder.BuildInstruction ());
      }
      else {
	MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::Call> builder; 
	builder.setFunctionPtr (findValue(func,values,tt));
	if (!inst->getType ()->isVoidTy ()) {
	  builder.setRes (findValue(inst,values,tt));
	}
	builder.setNbParamters (std::make_shared<MiniMC::Model::IntegerConstant> (cinst->arg_size ()));
	for (auto it = cinst->arg_begin(); it!=cinst->arg_end(); ++it) {
	  builder.addParam (findValue(*it,values,tt));
	}
	instr.push_back(builder.BuildInstruction ());
      }
    }

    size_t calcSkip (llvm::Type* t, size_t index,Types& tt) {
      if (t->isArrayTy ()) {
	return tt.getSizeInBytes (static_cast<llvm::ArrayType*> (t)->getElementType())*index; 
      }

      else if (t->isStructTy ()) {
	size_t size = 0;
	auto strucTy = static_cast<llvm::StructType*> (t);
	for (size_t i = 0; i < index; ++i) {
	  size+=tt.getSizeInBytes (strucTy->getElementType(i));
	}
	return size;
      }
      else {
	throw MiniMC::Support::Exception ("Can't calculate size");
      }
    }
    
    template<>								\
    void translateAndAddInstruction<llvm::Instruction::ExtractValue> (llvm::Instruction* inst, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt) {
      llvm::ExtractValueInst* extractinst = llvm::dyn_cast<llvm::ExtractValueInst> (inst);
      auto extractfrom = extractinst->getAggregateOperand();
      if (llvm::Constant* cstextract = llvm::dyn_cast<llvm::Constant> (extractfrom)) {
	llvm::Constant* cur = cstextract;
	
	for (auto i : extractinst->getIndices ()) {
	  cur = cur->getAggregateElement (i);
	  assert(cur);
	}
	auto value = findValue (cur,values,tt);
	auto res = findValue (inst,values,tt);
	MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::Assign> builder;
	builder.setResult (res);
	builder.setValue (value);
	instr.push_back(builder.BuildInstruction ());
      }
      else {
	auto aggre = findValue (extractfrom,values,tt);
	size_t skip = 0;
	auto cur = extractfrom->getType ();
	for (auto i : extractinst->getIndices ()) {
	  skip+=calcSkip (cur,i,tt);
	}
	auto skipee = std::make_shared<MiniMC::Model::IntegerConstant> (skip);
	auto type = tt.tfac->makeIntegerType(32);
	skipee->setType (type);
	MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::ExtractValue> builder; 
	builder.setResult (findValue(inst,values,tt));
	builder.setOffset (skipee);
	builder.setAggregate (aggre);
	
	instr.push_back(builder.BuildInstruction ());
      }
      
    }

    template<>								\
    void translateAndAddInstruction<llvm::Instruction::InsertValue> (llvm::Instruction* inst, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt) {
       llvm::InsertValueInst* insertinst = llvm::dyn_cast<llvm::InsertValueInst> (inst);
      auto insertfrom = insertinst->getAggregateOperand();
      auto insertval = insertinst->getInsertedValueOperand ();
      if (llvm::Constant* cstextract = llvm::dyn_cast<llvm::Constant> (insertfrom)) {

	auto aggregate = findValue (insertfrom,values,tt);
	auto value = findValue (insertval,values,tt);
	auto res = findValue (inst,values,tt);
	MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::InsertValueFromConst> builder;
	builder.setAggregate (aggregate);
	builder.setResult (res);
	builder.setInsertee (value);
	instr.push_back(builder.BuildInstruction ());
      }
      else {
	auto aggre = findValue (insertfrom,values,tt);
	size_t skip = 0;
	auto cur = insertfrom->getType ();
	for (auto i : insertinst->getIndices ()) {
	  skip+=calcSkip (cur,i,tt);
	}
	auto skipee = std::make_shared<MiniMC::Model::IntegerConstant> (skip);
	auto type = tt.tfac->makeIntegerType(32);
	skipee->setType (type);
	MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::InsertValue> builder; 
	builder.setResult (findValue(inst,values,tt));
	builder.setOffset (skipee);
	builder.setAggregate (aggre);
	builder.setInsertee (aggre);
	
	
	instr.push_back(builder.BuildInstruction ());
      }
    }
    
    template<>								\
    void translateAndAddInstruction<llvm::Instruction::GetElementPtr> (llvm::Instruction* inst, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt) {
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
      builder.setResult (findValue (inst,values,tt));
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
    X(ICMP_ULE,ICMP_ULE)				\
    X(ICMP_EQ,ICMP_EQ)					\
    X(ICMP_NE,ICMP_NEQ)					\
    
    
    template<>								\
    void translateAndAddInstruction<llvm::Instruction::ICmp> (llvm::Instruction* inst, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt) { 
      auto ins = llvm::dyn_cast<llvm::ICmpInst> (inst);
      switch (ins->getPredicate ()) {
#define X(LLVM,OUR)				\
	case llvm::CmpInst::LLVM:	{				\
	  MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::OUR> builder; \
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
      default:
	throw MiniMC::Support::Exception ("Unsupported Operation");
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
	void translateAndAddInstruction<llvm::Instruction::LLVM> (llvm::Instruction* inst, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt) { \
    MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::OUR> builder; \
	auto res = findValue (inst,values,tt);				\
	auto left = findValue (inst->getOperand (0),values,tt);		\
	builder.setRes (res);						\
	builder.setCastee (left);					\
	instr.push_back(builder.BuildInstruction ());			\
    }
    LLVMCASTOPS
    #undef X
  }
}






