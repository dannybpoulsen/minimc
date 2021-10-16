#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Intrinsics.h>

#include <unordered_map>
#include "support/exceptions.hpp"
#include "model/variables.hpp"
#include "model/instructions.hpp"


namespace MiniMC {
  namespace Loaders {

    using Val2ValMap = std::unordered_map<const llvm::Value*, MiniMC::Model::Value_ptr>;
    
    MiniMC::Model::Type_ptr getType (llvm::Type* type, MiniMC::Model::TypeFactory_ptr& tfactory);
    uint32_t computeSizeInBytes (llvm::Type* ty,MiniMC::Model::TypeFactory_ptr& tfactory);
    struct Types {
      MiniMC::Model::VariableStackDescr_ptr stack;
      MiniMC::Model::TypeFactory_ptr tfac;
      MiniMC::Model::Type_ptr getType (llvm::Type* type) {
		return  MiniMC::Loaders::getType (type,tfac);
      }
      std::size_t getSizeInBytes (llvm::Type* type) {
		return computeSizeInBytes (type,tfac);
      }
    };

    MiniMC::Model::Value_ptr findValue (llvm::Value* val, Val2ValMap& values, Types& tt, MiniMC::Model::ConstantFactory_ptr& );

	
	
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
	void translateAndAddInstruction (llvm::Instruction*, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>&, Types&, MiniMC::Model::ConstantFactory_ptr& cfac) {
	  throw MiniMC::Support::Exception ("Error");
	}

#define X(LLVM,OUR)														\
    template<>															\
	void translateAndAddInstruction<llvm::Instruction::LLVM> (llvm::Instruction* inst, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt, MiniMC::Model::ConstantFactory_ptr& cfac) { \
	  MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::OUR> builder; \
	  assert(inst->isBinaryOp ());										\
	  auto res = findValue (inst,values,tt,cfac);						\
	  auto left = findValue (inst->getOperand (0),values,tt,cfac);		\
	  auto right = findValue (inst->getOperand (1),values,tt,cfac);		\
	  builder.setRes (res);												\
	  builder.setLeft (left);											\
	  builder.setRight (right);											\
	  instr.push_back(builder.BuildInstruction ());						\
	}
	LLVMTAC
#undef LLVMTAC
#undef X
	
 
  
	template<>								
	void translateAndAddInstruction<llvm::Instruction::Alloca> (llvm::Instruction* inst, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt, MiniMC::Model::ConstantFactory_ptr& cfac) { 
	  MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::Alloca> builder;
	  
	  auto alinst = llvm::dyn_cast<llvm::AllocaInst> (inst);
	  auto llalltype = alinst->getAllocatedType ();
	  auto outalltype = tt.getType (llalltype);
	  auto res = findValue (inst,values,tt,cfac);
	  auto type = tt.tfac->makeIntegerType (64);
	  auto size = cfac->makeIntegerConstant(outalltype->getSize(),type);
	  builder.setRes (res);						
	  builder.setSize (size);
	  instr.push_back(builder.BuildInstruction ());			
	}
    
    template<>								
    void translateAndAddInstruction<llvm::Instruction::Ret> (llvm::Instruction* inst, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt, MiniMC::Model::ConstantFactory_ptr& cfac) { 
      auto retinst = llvm::dyn_cast<llvm::ReturnInst> (inst);
      assert(retinst);
      if (retinst->getReturnValue ()) {
		MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::Ret> builder;
		auto res = findValue (retinst->getReturnValue(),values,tt,cfac);		
		builder.setRetValue (res);
		instr.push_back(builder.BuildInstruction ());
      }
      else {
		MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::RetVoid> builder;
		instr.push_back(builder.BuildInstruction ());
      }			
    }

    template<>								\
    void translateAndAddInstruction<llvm::Instruction::Load> (llvm::Instruction* inst, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt, MiniMC::Model::ConstantFactory_ptr& cfac) { 
      MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::Load> builder; 
      auto res = findValue (inst,values,tt,cfac);		
      auto addr = findValue (inst->getOperand (0),values,tt,cfac);		
      builder.setRes (res);						
      builder.setAddress (addr);						
      instr.push_back(builder.BuildInstruction ());			
    }
    
    template<>								\
    void translateAndAddInstruction<llvm::Instruction::Store> (llvm::Instruction* inst, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt, MiniMC::Model::ConstantFactory_ptr& cfac) { 
      MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::Store> builder; 
      auto value = findValue (inst->getOperand (0),values,tt,cfac);		
      auto addr = findValue (inst->getOperand (1),values,tt,cfac);		
      builder.setValue (value);						
      builder.setAddress (addr);						
      instr.push_back(builder.BuildInstruction ());			
    }
    
    template<unsigned>
    void translateIntrinsicCall (llvm::Instruction* inst, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt, MiniMC::Model::ConstantFactory_ptr& cfac) {
      throw MiniMC::Support::Exception ("Unsupported Intrinsic");
    }

    template<>
    void translateIntrinsicCall<llvm::Intrinsic::stackrestore> (llvm::Instruction* inst, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt, MiniMC::Model::ConstantFactory_ptr& cfac) {
      MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::StackRestore> builder; 
      auto cinst = llvm::dyn_cast<llvm::CallInst> (inst);
      assert(cinst->arg_size()==1);
      builder.setValue (findValue(*cinst->arg_begin(),values,tt,cfac));
      instr.push_back(builder.BuildInstruction ());
    }

    template<>
    void translateIntrinsicCall<llvm::Intrinsic::stacksave> (llvm::Instruction* inst, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt, MiniMC::Model::ConstantFactory_ptr& cfac) {
      MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::StackSave> builder; 
      assert(inst->getType ()->isPointerTy ());
      builder.setResult (findValue(inst,values,tt,cfac));
      instr.push_back(builder.BuildInstruction ());
    }

	template<>
    void translateIntrinsicCall<llvm::Intrinsic::memcpy> (llvm::Instruction* inst, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt, MiniMC::Model::ConstantFactory_ptr& cfac) {
      MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::MemCpy> builder; 
      auto cinst = llvm::dyn_cast<llvm::CallInst> (inst);
      assert(cinst->arg_size()==4);
	  auto arg = cinst->arg_begin();
      builder.setTarget (findValue((*arg++),values,tt,cfac));
	  builder.setSource (findValue((*arg++),values,tt,cfac));
	  builder.setSize (findValue((*arg++),values,tt,cfac));
	  
	  
      instr.push_back(builder.BuildInstruction ());
    }

#define SUPPORTEDINTRIN							\
    X(llvm::Intrinsic::stackrestore)			\
    X(llvm::Intrinsic::stacksave)				\
    X(llvm::Intrinsic::memcpy)				\
	
	
    template<>															
    void translateAndAddInstruction<llvm::Instruction::Call> (llvm::Instruction* inst, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt, MiniMC::Model::ConstantFactory_ptr& cfac) { 
      auto cinst = llvm::dyn_cast<llvm::CallInst> (inst);
      auto func = cinst->getCalledFunction ();
	  assert(func);
      if (func->isIntrinsic()) {
		switch (func->getIntrinsicID ()) {
#define X(H)														\
		  case H:													\
			translateIntrinsicCall<H> (inst,values,instr,tt,cfac);	\
			return ;
		  SUPPORTEDINTRIN
		default:
		  throw MiniMC::Support::Exception ("Unsupported Intrinsic");
			
#undef X
			}
      }
      if (func->getName () == "assert") {
		MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::Assert> builder;
		assert(cinst->arg_size () == 1);
		auto val = findValue(*cinst->arg_begin(),values,tt,cfac);
		if (val->getType ()->getTypeID () == MiniMC::Model::TypeID::Bool) {
		  builder.setAssert (val);
		  instr.push_back(builder.BuildInstruction ());
		}
	
		else if (val->getType ()->getTypeID () == MiniMC::Model::TypeID::Integer) {
		  auto ntype = tt.tfac->makeBoolType ();
		  auto nvar = tt.stack->addVariable ("BVar",ntype);
		  MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::IntToBool> intbboolbuilder;
		  MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::Assert> assbuilder;
		  intbboolbuilder.setRes (nvar);
		  intbboolbuilder.setCastee (val);
		  assbuilder.setAssert (nvar);
		  instr.push_back (intbboolbuilder.BuildInstruction ());
		  instr.push_back (assbuilder.BuildInstruction ());
	  
		}
      }
      if (func->getName () == "malloc") {
		MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::Malloc> builder;
		MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::FindSpace> sbuilder;
	
		assert(cinst->arg_size () == 1);
		auto val = findValue(*cinst->arg_begin(),values,tt,cfac);
		if (val->getType ()->getTypeID () == MiniMC::Model::TypeID::Integer) {
		  builder.setSize (val);
		  sbuilder.setSize (val);
		  assert(inst->getType ()->isPointerTy ());
		  builder.setPointer (findValue(inst,values,tt,cfac));
		  sbuilder.setResult (findValue(inst,values,tt,cfac));
	  
		}
		instr.push_back(sbuilder.BuildInstruction ());
		instr.push_back(builder.BuildInstruction ());
      }
      else if (func->isDeclaration ()) {
		//We don't know what to do for this function
		MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::NonDet> builder;
		if (inst->getType ()->isIntegerTy ()) {
		  std::size_t bitwidth = inst->getType ()->getIntegerBitWidth ();
		  auto type = tt.tfac->makeIntegerType(bitwidth);
		  switch (bitwidth) {
		  case 8:
			builder.setMin (cfac->makeIntegerConstant (std::numeric_limits<MiniMC::uint8_t>::min (),type));
			builder.setMax (cfac->makeIntegerConstant (std::numeric_limits<MiniMC::uint8_t>::max (),type));
			break;
		  case 16:
			builder.setMin (cfac->makeIntegerConstant (std::numeric_limits<MiniMC::uint16_t>::min (),type));
			builder.setMax (cfac->makeIntegerConstant (std::numeric_limits<MiniMC::uint16_t>::max (),type));
			break;
		  case 32:
			builder.setMin (cfac->makeIntegerConstant (std::numeric_limits<MiniMC::uint32_t>::min (),type));
			builder.setMax (cfac->makeIntegerConstant (std::numeric_limits<MiniMC::uint32_t>::max (),type));
			break;
		  case 64:
			builder.setMin (cfac->makeIntegerConstant (std::numeric_limits<MiniMC::uint64_t>::min (),type));
			builder.setMax (cfac->makeIntegerConstant (std::numeric_limits<MiniMC::uint64_t>::max (),type));
			break;
		  default:
			throw MiniMC::Support::Exception ("Error");
			  
		  }
			
		  builder.setResult (findValue(inst,values,tt,cfac));
		  instr.push_back(builder.BuildInstruction ());
		}
      }
      else {
		MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::Call> builder; 
		builder.setFunctionPtr (findValue(func,values,tt,cfac));
		if (!inst->getType ()->isVoidTy ()) {
		  builder.setRes (findValue(inst,values,tt,cfac));
		}
		auto type = tt.tfac->makeIntegerType(64);
		builder.setNbParamters (cfac->makeIntegerConstant (cinst->arg_size (),type));
		for (auto it = cinst->arg_begin(); it!=cinst->arg_end(); ++it) {
		  builder.addParam (findValue(*it,values,tt,cfac));
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
    void translateAndAddInstruction<llvm::Instruction::ExtractValue> (llvm::Instruction* inst, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt, MiniMC::Model::ConstantFactory_ptr& cfac) {
      llvm::ExtractValueInst* extractinst = llvm::dyn_cast<llvm::ExtractValueInst> (inst);
      auto extractfrom = extractinst->getAggregateOperand();
      if (llvm::Constant* cstextract = llvm::dyn_cast<llvm::Constant> (extractfrom)) {
		llvm::Constant* cur = cstextract;
	
		for (auto i : extractinst->getIndices ()) {
		  cur = cur->getAggregateElement (i);
		  assert(cur);
		}
		auto value = findValue (cur,values,tt,cfac);
		auto res = findValue (inst,values,tt,cfac);
		MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::Assign> builder;
		builder.setResult (res);
		builder.setValue (value);
		instr.push_back(builder.BuildInstruction ());
      }
      else {
		auto aggre = findValue (extractfrom,values,tt,cfac);
		size_t skip = 0;
		auto cur = extractfrom->getType ();
		for (auto i : extractinst->getIndices ()) {
		  skip+=calcSkip (cur,i,tt);
		}
		auto type = tt.tfac->makeIntegerType(32);
		auto skipee = cfac->makeIntegerConstant (skip,type);
		MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::ExtractValue> builder; 
		builder.setResult (findValue(inst,values,tt,cfac));
		builder.setOffset (skipee);
		builder.setAggregate (aggre);
	
		instr.push_back(builder.BuildInstruction ());
      }
      
    }

    template<>								\
    void translateAndAddInstruction<llvm::Instruction::InsertValue> (llvm::Instruction* inst, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt, MiniMC::Model::ConstantFactory_ptr& cfac) {
	  llvm::InsertValueInst* insertinst = llvm::dyn_cast<llvm::InsertValueInst> (inst);
      auto insertfrom = insertinst->getAggregateOperand();
      auto insertval = insertinst->getInsertedValueOperand ();
      if (llvm::Constant* cstextract = llvm::dyn_cast<llvm::Constant> (insertfrom)) {
	throw MiniMC::Support::Exception ("Not supported");
	/*auto aggregate = findValue (insertfrom,values,tt,cfac);
		auto value = findValue (insertval,values,tt,cfac);
		auto res = findValue (inst,values,tt,cfac);
		MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::InsertValueFromConst> builder;
		builder.setAggregate (aggregate);
		builder.setResult (res);
		builder.setInsertee (value);
		instr.push_back(builder.BuildInstruction ());*/
      }
      else {
		auto aggre = findValue (insertfrom,values,tt,cfac);
		size_t skip = 0;
		auto cur = insertfrom->getType ();
		for (auto i : insertinst->getIndices ()) {
		  skip+=calcSkip (cur,i,tt);
		}
		auto type = tt.tfac->makeIntegerType(32);
		auto skipee = cfac->makeIntegerConstant (skip,type);
		MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::InsertValue> builder; 
		builder.setResult (findValue(inst,values,tt,cfac));
		builder.setOffset (skipee);
		builder.setAggregate (aggre);
		builder.setInsertee (aggre);
	
	
		instr.push_back(builder.BuildInstruction ());
      }
    }
    
    template<>								\
    void translateAndAddInstruction<llvm::Instruction::GetElementPtr> (llvm::Instruction* inst, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt, MiniMC::Model::ConstantFactory_ptr& cfac) {
      auto gep = static_cast<llvm::GetElementPtrInst*> (inst);
      MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::PtrAdd> builder;
      auto source = gep->getSourceElementType ();
      if (gep->getNumIndices () == 1) {
		auto val = findValue (gep->getOperand (1),values,tt,cfac);
		auto size = tt.getSizeInBytes(source);
		auto sizec = cfac->makeIntegerConstant (size,val->getType());
		auto ptr = findValue (gep->getOperand (0),values,tt,cfac);
		builder.setSkipSize (sizec);
		builder.setValue (val);
		builder.setAddress (ptr);
      }
      else {
		auto i64 = tt.tfac->makeIntegerType (64);
		auto one = cfac->makeIntegerConstant (1,i64);
	
		if (source->isArrayTy () ) {
		  auto elemType = tt.getType ( static_cast<llvm::ArrayType*> (source)->getElementType());
		  auto val = findValue (gep->getOperand (2),values,tt,cfac);
		  auto sizec = cfac->makeIntegerConstant (elemType->getSize(),val->getType());
	  
		  auto ptr = findValue (gep->getOperand (0),values,tt,cfac);
		  sizec->setType (val->getType ());
		  builder.setSkipSize (sizec);
		  builder.setValue (val);
		  builder.setAddress (ptr);
		}
		else if (source->isStructTy ()) {
		  auto strucTy = static_cast<llvm::StructType*> (source);
		  auto ptr = findValue (gep->getOperand (0),values,tt,cfac);
		  size_t size = 0;
		  auto cinst = llvm::dyn_cast<llvm::ConstantInt> (gep->getOperand(2));
		  assert (cinst);
		  auto t = cinst->getZExtValue ();
		  for (size_t i = 0; i < t; ++i) {
			size+=tt.getSizeInBytes (strucTy->getElementType(i));
		  }
		  auto sizec = cfac->makeIntegerConstant (size,i64);
		  builder.setValue (one);
		  builder.setAddress (ptr);
		  builder.setSkipSize (sizec);
		}

      }
      builder.setResult (findValue (inst,values,tt,cfac));
      instr.push_back(builder.BuildInstruction ());			
    }
    
#define LLVMICMP								\
    X(ICMP_SGT,ICMP_SGT)						\
    X(ICMP_UGT,ICMP_UGT)						\
    X(ICMP_SGE,ICMP_SGE)						\
    X(ICMP_UGE,ICMP_UGE)						\
    X(ICMP_SLT,ICMP_SLT)						\
    X(ICMP_ULT,ICMP_ULT)						\
    X(ICMP_SLE,ICMP_SLE)						\
    X(ICMP_ULE,ICMP_ULE)						\
    X(ICMP_EQ,ICMP_EQ)							\
    X(ICMP_NE,ICMP_NEQ)							\
    
    
    template<>								\
    void translateAndAddInstruction<llvm::Instruction::ICmp> (llvm::Instruction* inst, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt, MiniMC::Model::ConstantFactory_ptr& cfac) { 
      auto ins = llvm::dyn_cast<llvm::ICmpInst> (inst);
      switch (ins->getPredicate ()) {
#define X(LLVM,OUR)														\
		case llvm::CmpInst::LLVM:	{									\
		  MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::OUR> builder; \
		  auto res = findValue (inst,values,tt,cfac);					\
		  auto left = findValue (inst->getOperand (0),values,tt,cfac);	\
		  auto right = findValue (inst->getOperand (1),values,tt,cfac);	\
		  builder.setRes (res);											\
		  builder.setLeft (left);										\
		  builder.setRight (right);										\
		  instr.push_back(builder.BuildInstruction ());					\
		  break;														\
		}	
		LLVMICMP
      default:
		throw MiniMC::Support::Exception ("Unsupported Operation");
	  }	
    }
#undef LLVMICMP
#undef X

#define LLVMCASTOPS								\
    X(Trunc,Trunc)								\
    X(ZExt,ZExt)								\
    X(SExt,SExt)								\
    X(PtrToInt,PtrToInt)						\
    X(IntToPtr,IntToPtr)						\
    X(BitCast,BitCast)							\
    
#define X(LLVM,OUR)														\
    template<>															\
    void translateAndAddInstruction<llvm::Instruction::LLVM> (llvm::Instruction* inst, Val2ValMap& values, std::vector<MiniMC::Model::Instruction>& instr, Types& tt, MiniMC::Model::ConstantFactory_ptr& cfac) { \
	  MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::OUR> builder; \
	  auto res = findValue (inst,values,tt,cfac);						\
	  auto left = findValue (inst->getOperand (0),values,tt,cfac);		\
	  builder.setRes (res);												\
	  builder.setCastee (left);											\
	  instr.push_back(builder.BuildInstruction ());						\
    }
    LLVMCASTOPS
#undef X
  }
}






