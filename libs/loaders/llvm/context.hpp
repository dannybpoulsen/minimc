#include "minimc/model/variables.hpp"
#include "minimc/model/instructions.hpp"
#include "minimc/model/builder.hpp"
#include "minimc/support/overload.hpp"

#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Constants.h>
#include <unordered_map>
#include <iostream>

namespace MiniMC {
  namespace Loaders {
    struct GLoadContext {
      GLoadContext (
		    MiniMC::Model::Value_ptr mem
		    ) : heap_mem(mem) {}
      GLoadContext(const GLoadContext& g) : values(g.values),heap_mem(g.heap_mem) {
	
      }
      
      
      virtual ~GLoadContext () {}
      MiniMC::Model::Value_ptr findValue (const llvm::Value* val);
      void addValue (const llvm::Value* val, MiniMC::Model::Value_ptr vals) {values.emplace (val,vals);}
      bool hasValue (const llvm::Value* v) {return values.count (v);}
      MiniMC::BV32 computeSizeInBytes (llvm::Type* );
      MiniMC::Model::Type_ptr getType (llvm::Type*);
      auto getHeapMem () const {return heap_mem;};
    private:
      std::unordered_map<const llvm::Value*,MiniMC::Model::Value_ptr> values;
      MiniMC::Model::Value_ptr heap_mem;
    };

    template<class T>
    MiniMC::Model::Value_ptr makeInteger(T val, MiniMC::Model::TypeID types) {
      switch(types) {
      case MiniMC::Model::TypeID::I8:
	return MiniMC::Model::I8Integer::make(val);
      case MiniMC::Model::TypeID::I16:
	return MiniMC::Model::I16Integer::make(val);
      case MiniMC::Model::TypeID::I32:
	return MiniMC::Model::I32Integer::make(val);
      case MiniMC::Model::TypeID::I64:
	return MiniMC::Model::I64Integer::make(val);
      default:
	throw MiniMC::Support::Exception ("Not aninteger");
	
      }
	
    }
    
    
    struct LoadContext : public GLoadContext {
    public:

      LoadContext (const LoadContext& ) = delete;
      LoadContext ( const GLoadContext& c,
		    MiniMC::Model::RegisterDescr& descr,
		    const MiniMC::Model::Value_ptr& sp,
		    const MiniMC::Model::Value_ptr& sp_mem,
		    MiniMC::Model::Frame frame
		    ) : GLoadContext(c),stack(descr),sp(sp),sp_mem(sp_mem),frame(frame) {
	
      }
      auto& getStack () {return stack;}
      auto& getStackPointerMem () {return sp_mem;}
      auto& getStackPointer () {return sp;}
      auto& getFrame () {return frame;}
    private:
      
      MiniMC::Model::RegisterDescr& stack;
      MiniMC::Model::Value_ptr sp;
      MiniMC::Model::Value_ptr sp_mem;
      MiniMC::Model::Frame frame;
    };

    struct InstructionTranslator {
      InstructionTranslator (LoadContext& context) : context(context) {}
      template<class Gatherer>
      void operator() (llvm::Instruction*, Gatherer&& gather);
    private:

      template<MiniMC::Model::InstructionCode code,class Gatherer>
      void createInstruction (llvm::Instruction* inst, Gatherer&& gather);
      
      LoadContext& context;
    };

    
    template<MiniMC::Model::InstructionCode code,class Gatherer>
    void InstructionTranslator::createInstruction (llvm::Instruction* inst, Gatherer&& gather)       
    {
      auto calcSkip = [this](auto & ty, auto index) {
	if (ty->isArrayTy()) {
	  return context.computeSizeInBytes(static_cast<llvm::ArrayType*>(ty)->getElementType()) * index;
	}
	
	else if (ty->isStructTy()) {
	  MiniMC::BV32 size = 0;
	  auto strucTy = static_cast<llvm::StructType*>(ty);
	  for (size_t i = 0; i < index; ++i) {
	    size += context.computeSizeInBytes(strucTy->getElementType(i));
	  }
	  return size;
	} else {
	  throw MiniMC::Support::Exception("Can't calculate size");
	   }
      };
      
      if constexpr (MiniMC::Model::InstructionData<code>::isTAC ||
		    MiniMC::Model::InstructionData<code>::isComparison) {
	auto res = context.findValue (inst);
	auto op1 = context.findValue (inst->getOperand(0));
	auto op2 = context.findValue (inst->getOperand(1));
	gather.template addInstr<code> (
	  res,
	  op1,
	  op2
	  );
      }

      else if constexpr (MiniMC::Model::InstructionData<code>::isCast ) {
	gather.template addInstr<code> (
	    context.findValue (inst),
	    context.findValue (inst->getOperand (0))
	  );
      }

      else if constexpr (MiniMC::Model::InstructionCode::Load == code) {
	gather.template addInstr<MiniMC::Model::InstructionCode::Load>(
	    context.findValue (inst),
	    context.getHeapMem(),
	    context.findValue (inst->getOperand (0))
	  );
      }

      else if constexpr (MiniMC::Model::InstructionCode::Store == code) {
	gather.template addInstr<MiniMC::Model::InstructionCode::Store>(
									context.getHeapMem(),
									context.getHeapMem(),
									context.findValue (inst->getOperand(1)),
									context.findValue (inst->getOperand (0))
									);
      }

      else if constexpr (MiniMC::Model::InstructionCode::InsertValue == code) {
	 llvm::InsertValueInst* insertinst = llvm::dyn_cast<llvm::InsertValueInst>(inst);
	 auto insertfrom = insertinst->getAggregateOperand();
	 auto insertval = insertinst->getInsertedValueOperand();
      
	 auto aggre = context.findValue(insertfrom);
	 auto insertee = context.findValue(insertval);
	 size_t skip = 0;
	 auto cur = insertfrom->getType();
	 for (auto i : insertinst->getIndices()) {
	   skip += calcSkip(cur, i);
	 }
	 
	 auto skipee = makeInteger(skip, MiniMC::Model::TypeID::I32);

	 auto res = context.findValue(inst);
	  
	 gather.template addInstr<MiniMC::Model::InstructionCode::InsertValue>(
	     context.findValue(inst),
	     aggre,
	     skipee,
	     insertee);;
      }

      else if constexpr (MiniMC::Model::InstructionCode::ExtractValue == code) {
	llvm::ExtractValueInst* extractinst = llvm::dyn_cast<llvm::ExtractValueInst>(inst);
	auto extractfrom = extractinst->getAggregateOperand();
	if (llvm::Constant* cstextract = llvm::dyn_cast<llvm::Constant>(extractfrom)) {
	  llvm::Constant* cur = cstextract;
	  
	  for (auto i : extractinst->getIndices()) {
	    cur = cur->getAggregateElement(i);
	    assert(cur);
	  }
	  auto value = context.findValue(cur);
	  auto res = context.findValue(inst);
	  gather.template addInstr<MiniMC::Model::InstructionCode::Assign>( res,
									    value);
	  
	}
	else {
	auto aggre = context.findValue(extractfrom);
	  size_t skip = 0;
	  auto cur = extractfrom->getType();
	  for (auto i : extractinst->getIndices()) {
	    skip += calcSkip(cur, i);
	  }
	  
	  auto skipee = makeInteger(skip, MiniMC::Model::TypeID::I32);
	  auto res = context.findValue(inst);
	  
	  gather.template addInstr<MiniMC::Model::InstructionCode::ExtractValue>(
	      res,
	      aggre,
	      skipee
	    );
	  
	}
      }
	
      else if constexpr (MiniMC::Model::InstructionCode::Call == code) {
	
	auto cinst = llvm::dyn_cast<llvm::CallInst>(inst);
	auto func = cinst->getCalledFunction();
	if (func && func->getName() == "assert") {
	  auto val = context.findValue(*cinst->arg_begin());
	  if (val->getType()->getTypeID() == MiniMC::Model::TypeID::Bool) {
	    
	    gather.template addInstr<MiniMC::Model::InstructionCode::Assert>(val);
	  }
	  
	  else if (val->getType()->isInteger ()) {
	    auto ntype = MiniMC::Model::BoolType::get();
	    auto nvar = context.getStack().addRegister(context.getFrame ().makeFresh ("bool"), ntype);
	    gather.
	      template addInstr<MiniMC::Model::InstructionCode::IntToBool>(nvar, val).
	      template addInstr<MiniMC::Model::InstructionCode::Assert>(nvar);
	  }
	  
	    
	}
	else {
	  std::vector<MiniMC::Model::Value_ptr> params;
	  MiniMC::Model::Value_ptr func_ptr = context.findValue(cinst->getCalledOperand ());
	  MiniMC::Model::Value_ptr res = nullptr;
	  if (!inst->getType()->isVoidTy()) {
	    res = context.findValue(inst);
	  }
	  auto type = MiniMC::Model::I64Type::get();
	  for (auto it = cinst->arg_begin(); it != cinst->arg_end(); ++it) {
	    params.push_back(context.findValue(*it));
	  }
	  gather.template addInstr<MiniMC::Model::InstructionCode::Call>(
	      res,
	      func_ptr,
	      params);
	}
      }

      else if constexpr (MiniMC::Model::InstructionCode::Ret == code) {
	auto retinst = llvm::dyn_cast<llvm::ReturnInst>(inst);
	gather.template addInstr<MiniMC::Model::InstructionCode::Assign>(context.getStackPointer (),context.getStackPointerMem());
	if (retinst->getReturnValue()) {
	  
	  auto res = context.findValue(retinst->getReturnValue());
	  gather.template addInstr<MiniMC::Model::InstructionCode::Ret>(res);
	  
	} else {
	  gather.template addInstr<MiniMC::Model::InstructionCode::RetVoid>();
	}
      }
      
      else {
	[]<bool b= false>() {static_assert(b);}();
      }
      
    }
    
    template<class Gatherer>
    void InstructionTranslator::operator() (llvm::Instruction* inst, Gatherer&& gather) {
      
      switch (inst->getOpcode ()) {
      case llvm::Instruction::Add:
	createInstruction<MiniMC::Model::InstructionCode::Add> (inst,gather);
	break;
      case llvm::Instruction::Sub:
	createInstruction<MiniMC::Model::InstructionCode::Sub> (inst,gather);
	break;
      case llvm::Instruction::Mul:
	createInstruction<MiniMC::Model::InstructionCode::Mul> (inst,gather);
	break;
      case llvm::Instruction::UDiv:
	createInstruction<MiniMC::Model::InstructionCode::UDiv> (inst,gather);
	break;
      case llvm::Instruction::SDiv:
	createInstruction<MiniMC::Model::InstructionCode::UDiv> (inst,gather);
	break;
      case llvm::Instruction::Shl:
	createInstruction<MiniMC::Model::InstructionCode::Shl> (inst,gather);
	break;
      case llvm::Instruction::LShr:
	createInstruction<MiniMC::Model::InstructionCode::LShr> (inst,gather);
	break;
      case llvm::Instruction::AShr:
	createInstruction<MiniMC::Model::InstructionCode::AShr> (inst,gather);
	break;
      case llvm::Instruction::And:
	createInstruction<MiniMC::Model::InstructionCode::And> (inst,gather);
	break;
      case llvm::Instruction::Or:
	createInstruction<MiniMC::Model::InstructionCode::Or> (inst,gather);
	break;
      case llvm::Instruction::Xor:
	createInstruction<MiniMC::Model::InstructionCode::Xor> (inst,gather);
	break;
      case llvm::Instruction::ICmp:
	{
	        auto ins = llvm::dyn_cast<llvm::ICmpInst>(inst);
		switch (ins->getPredicate()) {
		case llvm::CmpInst::ICMP_SGT:
		  createInstruction<MiniMC::Model::InstructionCode::SGt> (inst,gather);
		  break;
		case llvm::CmpInst::ICMP_UGT:
		  createInstruction<MiniMC::Model::InstructionCode::UGt> (inst,gather);
		  break;
		case llvm::CmpInst::ICMP_SGE:
		  createInstruction<MiniMC::Model::InstructionCode::SGe> (inst,gather);
		  break;
		case llvm::CmpInst::ICMP_UGE:
		  createInstruction<MiniMC::Model::InstructionCode::UGe> (inst,gather);
		  break;
		case llvm::CmpInst::ICMP_SLT:
		  createInstruction<MiniMC::Model::InstructionCode::SLt> (inst,gather);
		  break;
		case llvm::CmpInst::ICMP_ULT:
		  createInstruction<MiniMC::Model::InstructionCode::ULt> (inst,gather);
		  break;
		case llvm::CmpInst::ICMP_SLE:
		  createInstruction<MiniMC::Model::InstructionCode::SLe> (inst,gather);
		  break;
		case llvm::CmpInst::ICMP_ULE:
		  createInstruction<MiniMC::Model::InstructionCode::ULe> (inst,gather);
		  break;
		case llvm::CmpInst::ICMP_EQ:
		  createInstruction<MiniMC::Model::InstructionCode::Eq> (inst,gather);
		  break;
		case llvm::CmpInst::ICMP_NE:
		  createInstruction<MiniMC::Model::InstructionCode::NEq> (inst,gather);
		  break;
		default:
		  throw MiniMC::Support::Exception ("Not handled");
		}
		break;
	}

      case llvm::Instruction::Trunc:
	createInstruction<MiniMC::Model::InstructionCode::Trunc> (inst,gather);
	break;
      case llvm::Instruction::ZExt:
	createInstruction<MiniMC::Model::InstructionCode::ZExt> (inst,gather);
	break;
      case llvm::Instruction::SExt:
	createInstruction<MiniMC::Model::InstructionCode::SExt> (inst,gather);
	break;
      case llvm::Instruction::PtrToInt:
	createInstruction<MiniMC::Model::InstructionCode::PtrToInt> (inst,gather);
	break;
      case llvm::Instruction::IntToPtr:
	createInstruction<MiniMC::Model::InstructionCode::IntToPtr> (inst,gather);
	break;
      case llvm::Instruction::Load:
	createInstruction<MiniMC::Model::InstructionCode::Load> (inst,gather);
	break;
      case llvm::Instruction::Store:
	createInstruction<MiniMC::Model::InstructionCode::Store> (inst,gather);
	break;
      case llvm::Instruction::Alloca: {
	 auto alinst = llvm::dyn_cast<llvm::AllocaInst>(inst);
	 auto llalltype = alinst->getAllocatedType();
	 auto outallsize = context.computeSizeInBytes(llalltype);
	 auto res = context.findValue(inst);
	 auto size = makeInteger(outallsize, MiniMC::Model::TypeID::I32);
	 auto skipsize = makeInteger(1, MiniMC::Model::TypeID::I32);
	 
	 

	 gather.template addInstr<MiniMC::Model::InstructionCode::PtrSub>(
	     context.getStackPointer(),
	     context.getStackPointer(),
	     std::make_shared<MiniMC::Model::MulExpr>(skipsize,
						     size)
	   );
	 gather.template addInstr<MiniMC::Model::InstructionCode::Assign>(
	     res,
	     context.getStackPointer()
	   );
	 
      }
	
	//createInstruction<MiniMC::Model::InstructionCode::Alloc> (inst,gather);
	break;
      case llvm::Instruction::GetElementPtr: {
	auto gep = static_cast<llvm::GetElementPtrInst*>(inst);
	MiniMC::Model::Value_ptr skipsize;
	MiniMC::Model::Value_ptr nbSkips;
	MiniMC::Model::Value_ptr address = context.findValue(gep->getOperand(0));
	;
	MiniMC::Model::Value_ptr result = context.findValue(inst);
		
	auto source = gep->getSourceElementType();
	if (gep->getNumIndices() == 1) {
	  nbSkips = context.findValue(gep->getOperand(1));
	  auto size = context.computeSizeInBytes(source);
	  skipsize = makeInteger(size, nbSkips->getType()->getTypeID());
	  
	} else {
	  auto one = makeInteger(1, MiniMC::Model::TypeID::I32);
	  
	  if (source->isArrayTy()) {
	    auto elemSize = context.computeSizeInBytes(static_cast<llvm::ArrayType*>(source)->getElementType());
	    nbSkips = context.findValue(gep->getOperand(2));
	    skipsize = makeInteger(elemSize, nbSkips->getType()->getTypeID());
	  } else if (source->isStructTy()) {
	    auto strucTy = static_cast<llvm::StructType*>(source);
	    size_t size = 0;
	    auto cinst = llvm::dyn_cast<llvm::ConstantInt>(gep->getOperand(2));
	    assert(cinst);
	    auto t = cinst->getZExtValue();
	    for (size_t i = 0; i < t; ++i) {
	      size += context.computeSizeInBytes(strucTy->getElementType(i));
	    }
	    skipsize = makeInteger(size, MiniMC::Model::TypeID::I32);
	    nbSkips = one;
	  }
	}
	gather.template addInstr<MiniMC::Model::InstructionCode::PtrAdd>(
									 result,
									 address,
									 std::make_shared<MiniMC::Model::MulExpr>(skipsize,
														  nbSkips));
	
      }
	//createInstruction<MiniMC::Model::InstructionCode::Alloc> (inst,gather);
	break;
      case llvm::Instruction::InsertValue:
	createInstruction<MiniMC::Model::InstructionCode::InsertValue> (inst,gather);
	break;
      case llvm::Instruction::ExtractValue:
	createInstruction<MiniMC::Model::InstructionCode::ExtractValue> (inst,gather);
	break;
      case llvm::Instruction::Call:
	createInstruction<MiniMC::Model::InstructionCode::Call> (inst,gather);
	break;
      case llvm::Instruction::Ret:
	createInstruction<MiniMC::Model::InstructionCode::Ret> (inst,gather);
	break;
	
      }
    }

    
   
    
  }
}
