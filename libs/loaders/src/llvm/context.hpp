#include "model/variables.hpp"
#include "model/instructions.hpp"
#include "model/builder.hpp"

#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <unordered_map>


namespace MiniMC {
  namespace Loaders {
    struct GLoadContext {
      GLoadContext (MiniMC::Model::ConstantFactory& cfact,
		    MiniMC::Model::TypeFactory& tfact) : cfact(cfact),tfact(tfact) {}
      GLoadContext (MiniMC::Model::ConstantFactory& cfact,
		    MiniMC::Model::TypeFactory& tfact,
		    std::unordered_map<const llvm::Value*,MiniMC::Model::Value_ptr> values) : values(std::move(values)),cfact(cfact),tfact(tfact) {}
      GLoadContext(const GLoadContext& g) : values(g.values),cfact(g.cfact),tfact(g.tfact) {
	
      }
      
      
      virtual ~GLoadContext () {}
      MiniMC::Model::Value_ptr findValue (const llvm::Value* val);
      void addValue (const llvm::Value* val, MiniMC::Model::Value_ptr vals) {values.emplace (val,vals);}
      bool hasValue (const llvm::Value* v) {return values.count (v);}
      MiniMC::BV32 computeSizeInBytes (llvm::Type* );
      auto& getConstantFactory () const {return cfact;}
      auto& getTypeFactory () const {return tfact;}
      MiniMC::Model::Type_ptr getType (llvm::Type*);
    private:
      std::unordered_map<const llvm::Value*,MiniMC::Model::Value_ptr> values;
      MiniMC::Model::ConstantFactory& cfact;
      MiniMC::Model::TypeFactory& tfact;
    };
    
    struct LoadContext : public GLoadContext {
    public:
      LoadContext (  MiniMC::Model::ConstantFactory& cfact,
		     MiniMC::Model::TypeFactory& tfact,
		     std::unordered_map<const llvm::Value*,MiniMC::Model::Value_ptr> values,
		     MiniMC::Model::RegisterDescr& descr,
		     const MiniMC::Model::Value_ptr& sp
		     ) : GLoadContext(cfact,tfact,std::move(values)),stack(descr),sp(sp) {}
      LoadContext (const LoadContext& ) = delete;
      LoadContext ( const GLoadContext& c,
		    MiniMC::Model::RegisterDescr& descr,
		    const MiniMC::Model::Value_ptr& sp) : GLoadContext(c),stack(descr),sp(sp) {
	
      }
      auto& getStack () {return stack;}
      auto& getStackPointer () {return sp;}
    private:
      
      MiniMC::Model::RegisterDescr& stack;
      MiniMC::Model::Value_ptr sp;
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
	gather.template addInstr<code> ({
	  .res = res,
	  .op1 = op1,
	  .op2 = op2
	  });
      }

      else if constexpr (MiniMC::Model::InstructionData<code>::isCast ) {
	gather.template addInstr<code> ({
	    .res = context.findValue (inst),
	    .op1 = context.findValue (inst->getOperand (0))
	  });
      }

      else if constexpr (MiniMC::Model::InstructionCode::Load == code) {
	gather.template addInstr<MiniMC::Model::InstructionCode::Load>({
	    .res = context.findValue (inst),
	    .addr = context.findValue (inst->getOperand (0))
	  });
      }

      else if constexpr (MiniMC::Model::InstructionCode::Store == code) {
	gather.template addInstr<MiniMC::Model::InstructionCode::Store>({
	    .addr = context.findValue (inst->getOperand(1)),
	    .storee = context.findValue (inst->getOperand (0))
	  });
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
	 
	 auto skipee = context.getConstantFactory ().makeIntegerConstant(skip, MiniMC::Model::TypeID::I32);
	 
	 gather.template addInstr<MiniMC::Model::InstructionCode::InsertValue>({
	     .res = context.findValue(inst),
	     .aggregate = aggre,
	     .offset = skipee,
	     .insertee = insertee});;
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
	  gather.template addInstr<MiniMC::Model::InstructionCode::Assign>(
									   {.res = res,
									    .op1 = value});
	  
	}
	else {
	auto aggre = context.findValue(extractfrom);
	  size_t skip = 0;
	  auto cur = extractfrom->getType();
	  for (auto i : extractinst->getIndices()) {
	    skip += calcSkip(cur, i);
	  }
	  
	  auto skipee = context.getConstantFactory().makeIntegerConstant(skip, MiniMC::Model::TypeID::I32);
	  auto res = context.findValue(inst);
	  
	  gather.template addInstr<MiniMC::Model::InstructionCode::ExtractValue>({
	      .res = res,
	      .aggregate = aggre,
	      .offset = skipee,
	    });
	  
	}
      }
	
      else if constexpr (MiniMC::Model::InstructionCode::Call == code) {
	
	auto cinst = llvm::dyn_cast<llvm::CallInst>(inst);
	auto func = cinst->getCalledFunction();
	assert(func);
	if (func->isIntrinsic()) {
	  throw MiniMC::Support::Exception ("No intrinsics atm");
	}
	if (func->getName() == "assert") {
	  assert(cinst->arg_size() == 1);
	  auto val = context.findValue(*cinst->arg_begin());
	  if (val->getType()->getTypeID() == MiniMC::Model::TypeID::Bool) {
	    
	    gather.template addInstr<MiniMC::Model::InstructionCode::Assert>({.expr = val});
	  }
	  
	  else if (val->getType()->isInteger ()) {
	    auto ntype = context.getTypeFactory ().makeBoolType();
	    auto nvar = context.getStack().addRegister(MiniMC::Model::Symbol{"BVar"}, ntype);
	    gather.
	      template addInstr<MiniMC::Model::InstructionCode::IntToBool>({.res = nvar, .op1 = val}).
	      template addInstr<MiniMC::Model::InstructionCode::Assert>({.expr = nvar});
	  }
	}
       else if (func->isDeclaration()) {
        //We don't know what to do for this function
        if (inst->getType()->isIntegerTy()) {
          std::size_t bitwidth = inst->getType()->getIntegerBitWidth();
          auto type = context.getTypeFactory().makeIntegerType(bitwidth);
          MiniMC::Model::Value_ptr min, max;
          MiniMC::Model::Value_ptr res = context.findValue(inst);

          switch (bitwidth) {
            case 8:
              min = context.getConstantFactory().makeIntegerConstant(std::numeric_limits<MiniMC::BV8>::min(), MiniMC::Model::TypeID::I8);
              max = context.getConstantFactory().makeIntegerConstant(std::numeric_limits<MiniMC::BV8>::max(), MiniMC::Model::TypeID::I8);
              break;
            case 16:
              min = context.getConstantFactory().makeIntegerConstant(std::numeric_limits<MiniMC::BV16>::min(), MiniMC::Model::TypeID::I16);
              max = context.getConstantFactory().makeIntegerConstant(std::numeric_limits<MiniMC::BV16>::max(), MiniMC::Model::TypeID::I16);
              break;
            case 32:
              min = context.getConstantFactory().makeIntegerConstant(std::numeric_limits<MiniMC::BV32>::min(), MiniMC::Model::TypeID::I32);
              max = context.getConstantFactory().makeIntegerConstant(std::numeric_limits<MiniMC::BV32>::max(), MiniMC::Model::TypeID::I32);
              break;
            case 64:
              min = context.getConstantFactory().makeIntegerConstant(std::numeric_limits<MiniMC::BV64>::min(), MiniMC::Model::TypeID::I64);
              max = context.getConstantFactory().makeIntegerConstant(std::numeric_limits<MiniMC::BV64>::max(), MiniMC::Model::TypeID::I64);
              break;
            default:
              throw MiniMC::Support::Exception("Error");
          }

          gather.template addInstr<MiniMC::Model::InstructionCode::NonDet>({.res = res, .min = min, .max = max});
        }
      } else {
        std::vector<MiniMC::Model::Value_ptr> params;
        MiniMC::Model::Value_ptr func_ptr = context.findValue(func);
        MiniMC::Model::Value_ptr res = nullptr;
        if (!inst->getType()->isVoidTy()) {
          res = context.findValue(inst);
        }
        auto type = context.getTypeFactory().makeIntegerType(64);
	params.push_back (context.getStackPointer());
        for (auto it = cinst->arg_begin(); it != cinst->arg_end(); ++it) {
          params.push_back(context.findValue(*it));
        }
        gather.template addInstr<MiniMC::Model::InstructionCode::Call>({
	    .res = res,
	    .function = func_ptr,
	    .params = params});	  
       }
      }

      else if constexpr (MiniMC::Model::InstructionCode::Ret == code) {
	auto retinst = llvm::dyn_cast<llvm::ReturnInst>(inst);
	if (retinst->getReturnValue()) {
	  
	  auto res = context.findValue(retinst->getReturnValue());
	  gather.template addInstr<MiniMC::Model::InstructionCode::Ret>({.value = res});
	  
	} else {
	  gather.template addInstr<MiniMC::Model::InstructionCode::RetVoid>(0);
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
		  createInstruction<MiniMC::Model::InstructionCode::ICMP_SGT> (inst,gather);
		  break;
		case llvm::CmpInst::ICMP_UGT:
		  createInstruction<MiniMC::Model::InstructionCode::ICMP_UGT> (inst,gather);
		  break;
		case llvm::CmpInst::ICMP_SGE:
		  createInstruction<MiniMC::Model::InstructionCode::ICMP_SGE> (inst,gather);
		  break;
		case llvm::CmpInst::ICMP_UGE:
		  createInstruction<MiniMC::Model::InstructionCode::ICMP_UGE> (inst,gather);
		  break;
		case llvm::CmpInst::ICMP_SLT:
		  createInstruction<MiniMC::Model::InstructionCode::ICMP_SLT> (inst,gather);
		  break;
		case llvm::CmpInst::ICMP_ULT:
		  createInstruction<MiniMC::Model::InstructionCode::ICMP_ULT> (inst,gather);
		  break;
		case llvm::CmpInst::ICMP_SLE:
		  createInstruction<MiniMC::Model::InstructionCode::ICMP_SLE> (inst,gather);
		  break;
		case llvm::CmpInst::ICMP_ULE:
		  createInstruction<MiniMC::Model::InstructionCode::ICMP_ULE> (inst,gather);
		  break;
		case llvm::CmpInst::ICMP_EQ:
		  createInstruction<MiniMC::Model::InstructionCode::ICMP_EQ> (inst,gather);
		  break;
		case llvm::CmpInst::ICMP_NE:
		  createInstruction<MiniMC::Model::InstructionCode::ICMP_NEQ> (inst,gather);
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
	 auto size = context.getConstantFactory ().makeIntegerConstant(outallsize, MiniMC::Model::TypeID::I32);
	 auto skipsize = context.getConstantFactory ().makeIntegerConstant(1, MiniMC::Model::TypeID::I32);
	 
	 gather.template addInstr<MiniMC::Model::InstructionCode::Assign>({
	     .res = res,
	     .op1 = context.getStackPointer()
	   });
	   
	   gather.template addInstr<MiniMC::Model::InstructionCode::PtrAdd>({.res = res,
	       .ptr = context.getStackPointer(),
	       .skipsize = size,
	       .nbSkips = skipsize
	     });
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
	  skipsize = context.getConstantFactory().makeIntegerConstant(size, nbSkips->getType()->getTypeID());
	  
	} else {
	  auto one = context.getConstantFactory().makeIntegerConstant(1, MiniMC::Model::TypeID::I32);
	  
	  if (source->isArrayTy()) {
	    auto elemSize = context.computeSizeInBytes(static_cast<llvm::ArrayType*>(source)->getElementType());
	    nbSkips = context.findValue(gep->getOperand(2));
	    skipsize = context.getConstantFactory().makeIntegerConstant(elemSize, nbSkips->getType()->getTypeID());
	  } else if (source->isStructTy()) {
	    auto strucTy = static_cast<llvm::StructType*>(source);
	    size_t size = 0;
	    auto cinst = llvm::dyn_cast<llvm::ConstantInt>(gep->getOperand(2));
	    assert(cinst);
	    auto t = cinst->getZExtValue();
	    for (size_t i = 0; i < t; ++i) {
	      size += context.computeSizeInBytes(strucTy->getElementType(i));
	    }
	    skipsize = context.getConstantFactory ().makeIntegerConstant(size, MiniMC::Model::TypeID::I32);
	    nbSkips = one;
	  }
	}
	gather.template addInstr<MiniMC::Model::InstructionCode::PtrAdd>(
									 {.res = result,
									  .ptr = address,
									  .skipsize = skipsize,
									  .nbSkips = nbSkips});
	
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
