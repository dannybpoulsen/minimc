#include "model/checkers/typechecker.hpp"

#include "model/cfg.hpp"
#include "model/cfg.hpp"
#include "support/feedback.hpp"
#include "support/localisation.hpp"


namespace MiniMC {
  namespace Model {
    namespace Checkers {
      template<MiniMC::Model::InstructionCode i,typename t = void> 
      struct TypeCheck {
	static bool doCheck (MiniMC::Model::Instruction&, MiniMC::Support::Messager& mess) {
	  MiniMC::Support::Localiser no_typecheck_for ("No typechecking implemented for '%1%'");
	  mess.error (no_typecheck_for.format(i));
	  
	  return false;}
      };

      template<MiniMC::Model::InstructionCode i>
      struct TypeCheck<i,typename std::enable_if<MiniMC::Model::InstructionData<i>::isTAC>::type > {
	static bool doCheck (MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess) {
	  MiniMC::Support::Localiser loc ("All operands to '%1%' must have same type as the result."); 
	  InstHelper<i> h (inst);
	  auto resType = h.getResult ()->getType ();
	  auto lType = h.getLeftOp ()->getType ();
	  auto rType = h.getRightOp ()->getType ();
	  if (resType != lType ||
	      lType != rType ||
	      rType != resType) {
	    mess.error (loc.format (i));
	    return false;
	  }
	  return true;
	}
      };
      template<MiniMC::Model::InstructionCode i>
      struct TypeCheck<i,typename std::enable_if<MiniMC::Model::InstructionData<i>::isComparison>::type > {
	static bool doCheck (MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess) {
	  MiniMC::Support::Localiser loc ("All operands to '%1%' must have same type..");
	  MiniMC::Support::Localiser res_must_be_bool ("The result of '%1% must be boolean.");
	  
	  InstHelper<i> h (inst);
	  auto resType = h.getResult ()->getType ();
	  auto lType = h.getLeftOp ()->getType ();
	  auto rType = h.getRightOp ()->getType ();
	  if (lType != rType ) {
	    mess.error (loc.format (i));
	    return false;
	  }
	  else if (resType->getTypeID () != MiniMC::Model::TypeID::Bool) {
	    mess.error (res_must_be_bool.format (i));
	    return false;
	  }
	   
	  return true;
	}
      };
      
	  
      template<>
      struct TypeCheck<MiniMC::Model::InstructionCode::Trunc,void > {
	static bool doCheck (MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess) {
	  MiniMC::Support::Localiser trunc_must_be_integer ("'%1%' can only be applied to integer types. "); 
	  MiniMC::Support::Localiser trunc_must_be_larger ("From type must be larger that to type for '%1%'"); 
		  
	  InstHelper<MiniMC::Model::InstructionCode::Trunc> h (inst);
	  auto ftype = h.getCastee ()->getType ();
	  auto ttype = h.getResult ()->getType ();
	  if (ftype->getTypeID () != MiniMC::Model::TypeID::Integer ||
	      ttype->getTypeID () != MiniMC::Model::TypeID::Integer) {
	    mess.error (trunc_must_be_integer.format (MiniMC::Model::InstructionCode::Trunc));
	    return false;
	  }
	  else if (ftype->getSize() <= ttype ->getSize()) {
	    mess.error (trunc_must_be_larger.format (MiniMC::Model::InstructionCode::Trunc));
	    return false;
	  }

	  return true;
	}
      };
	  
      template<MiniMC::Model::InstructionCode i>
      struct TypeCheck<i,typename std::enable_if<
			   i == MiniMC::Model::InstructionCode::SExt ||
			   i == MiniMC::Model::InstructionCode::ZExt>::type>
      {
	static bool doCheck (MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess) {
	  MiniMC::Support::Localiser must_be_integer ("'%1%' can only be applied to integer types. "); 
	  MiniMC::Support::Localiser must_be_smaller ("From type must be smaller that to type for '%1%'"); 
		  
	  InstHelper<i> h (inst);
	  auto ftype = h.getCastee ()->getType();
	  auto ttype = h.getResult ()->getType();
	  if (ftype->getTypeID () != MiniMC::Model::TypeID::Integer ||
	      ttype->getTypeID () != MiniMC::Model::TypeID::Integer) {
	    mess.error (must_be_integer.format (i));
	    return false;
	  }
	  else if (ftype->getSize() <= ttype->getSize()) {
	    mess.error (must_be_smaller.format (i));
	    return false;
	  }

	  return true;
	}
      };

      template<>
      struct TypeCheck<MiniMC::Model::InstructionCode::IntToPtr,void>
      {
	static bool doCheck (MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess) {
	  MiniMC::Support::Localiser must_be_integer ("'%1%' can only be applied to integer types. "); 
	  MiniMC::Support::Localiser must_be_pointer ("Return type has to be pointer for '%1%'"); 
		  
	  InstHelper<MiniMC::Model::InstructionCode::IntToPtr> h (inst);
	  auto ftype = h.getCastee ()->getType();
	  auto ttype = h.getResult ()->getType();
	  if (ftype->getTypeID () != MiniMC::Model::TypeID::Integer ) {
	    mess.error (must_be_integer.format (MiniMC::Model::InstructionCode::IntToPtr));
	    return false;
	  }
		  
	  else if (ttype->getTypeID() != MiniMC::Model::TypeID::Pointer) {
	    mess.error (must_be_pointer.format (MiniMC::Model::InstructionCode::IntToPtr));
	    return false;
	  }
		  
	  return true;
	}
      };
		
      template<>
      struct TypeCheck<MiniMC::Model::InstructionCode::PtrToInt>
      {
	static bool doCheck (MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess) {
	  MiniMC::Support::Localiser must_be_pointer ("'%1%' can only be applied to pointer types. "); 
	  MiniMC::Support::Localiser must_be_integer ("Return type has to be integer for '%1%'"); 
		  
	  InstHelper<MiniMC::Model::InstructionCode::PtrToInt> h (inst);
	  auto ftype = h.getCastee ()->getType();
	  auto ttype = h.getResult ()->getType();
	  if (ftype->getTypeID () != MiniMC::Model::TypeID::Pointer ) {
	    mess.error (must_be_pointer.format (MiniMC::Model::InstructionCode::PtrToInt));
	    return false;
	  }
		
	  else if (ttype->getTypeID() != MiniMC::Model::TypeID::Integer) {
	    mess.error (must_be_pointer.format (MiniMC::Model::InstructionCode::PtrToInt));
	    return false;
	  }
		
	  return true;
	}
      };

      template<>
      struct TypeCheck<MiniMC::Model::InstructionCode::Alloca>
      {
	static bool doCheck (MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess) {
	  MiniMC::Support::Localiser must_be_pointer ("'%1%' can only return pointer types. "); 
	  
	  InstHelper<MiniMC::Model::InstructionCode::Alloca> h (inst);
	  auto restype = h.getResult ()->getType();
	  auto alloc =   h.getResult ();
	  if (restype->getTypeID () != MiniMC::Model::TypeID::Pointer ) {
	    mess.error (must_be_pointer.format (MiniMC::Model::InstructionCode::Alloca));
	    return false;
	  }
		
	  return true;
	}
      };

      template<>
      struct TypeCheck<MiniMC::Model::InstructionCode::Store>
      {
	static bool doCheck (MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess) {
	  MiniMC::Support::Localiser must_be_pointer ("'%1%' can only store to pointer types. "); 
	 
	  
	  InstHelper<MiniMC::Model::InstructionCode::Store> h (inst);
	  auto addr = h.getAddress ()->getType();
	  if (addr->getTypeID () != MiniMC::Model::TypeID::Pointer ) {
	    mess.error (must_be_pointer.format (MiniMC::Model::InstructionCode::Store));
	    return false;
	  }
		
	  return true;
	}
      };

      template<>
      struct TypeCheck<MiniMC::Model::InstructionCode::Load>
      {
	static bool doCheck (MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess) {
	  MiniMC::Support::Localiser must_be_pointer ("'%1%' can only load from pointer types. "); 
	  
	  
	  InstHelper<MiniMC::Model::InstructionCode::Load> h (inst);
	  auto addr = h.getAddress ()->getType();
	  if (addr->getTypeID () != MiniMC::Model::TypeID::Pointer ) {
	    mess.error (must_be_pointer.format (MiniMC::Model::InstructionCode::Load));
	    return false;
	  }
		
	  return true;
	}
      };
      
      template<>
      struct TypeCheck<MiniMC::Model::InstructionCode::Skip>
      {
	static bool doCheck (MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess) {
	  return true;
	}
	  
      };

      template<>
      struct TypeCheck<MiniMC::Model::InstructionCode::Call>
      {
	static const MiniMC::Model::InstructionCode OpCode = MiniMC::Model::InstructionCode::Call;
	static bool doCheck (MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess) {
	  InstHelper<OpCode> h (inst);
	  auto func = h.getFunctionPtr ();
	  if (!func->isConstant ()) {
	    MiniMC::Support::Localiser must_be_constant ("'%1%' can only use constant function pointers. "); 
	    
	    mess.error (must_be_constant.format (OpCode)); 
	  }
	  mess.warning ("Type check of calls not properly implemented");
	  return true;
	}
	  
      };

      template<>
      struct TypeCheck<MiniMC::Model::InstructionCode::Assign>
      {
	static const MiniMC::Model::InstructionCode OpCode = MiniMC::Model::InstructionCode::Assign;
	static bool doCheck (MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess) {
	  MiniMC::Support::Localiser must_be_same_type ("Result and assignee must be same type for '%1%' "); 
	  InstHelper<OpCode> h (inst);
	  auto valT = h.getValue()->getType();
	  auto resT = h.getValue()->getType();
	  if (valT != resT) {
	    mess.error (must_be_same_type.format(OpCode));
	    return false;
	  }
	  return true;
	}
	
      };
      
      template<>
      struct TypeCheck<MiniMC::Model::InstructionCode::Ret>
      {
	static const MiniMC::Model::InstructionCode OpCode = MiniMC::Model::InstructionCode::Ret;
	static bool doCheck (MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess) {
	  return true;
	}
	  
      };

      template<>
      struct TypeCheck<MiniMC::Model::InstructionCode::RetVoid>
      {
	static const MiniMC::Model::InstructionCode OpCode = MiniMC::Model::InstructionCode::Call;
	static bool doCheck (MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess) {
	  return true;
	}
	  
      };

      template<>
      struct TypeCheck<MiniMC::Model::InstructionCode::NonDet>
      {
	static const MiniMC::Model::InstructionCode OpCode = MiniMC::Model::InstructionCode::NonDet;
	static bool doCheck (MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess) {
	  return true;
	}
	
      };

      template<>
      struct TypeCheck<MiniMC::Model::InstructionCode::Assert>
      {
	static const MiniMC::Model::InstructionCode OpCode = MiniMC::Model::InstructionCode::Assert;
	static bool doCheck (MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess) {
	  InstHelper<OpCode> h (inst);
	  MiniMC::Support::Localiser must_be_bool ("'%1%' must take boolean as inputs. "); 
	  
	  auto type = h.getAssert ()->getType ();
	  if (type->getTypeID () != MiniMC::Model::TypeID::Bool) {
	    mess.error (must_be_bool.format (OpCode));
	    return false;
	  }
	  return true;
	}
	  
      };

      template<>
      struct TypeCheck<MiniMC::Model::InstructionCode::StackRestore>
      {
	static const MiniMC::Model::InstructionCode OpCode = MiniMC::Model::InstructionCode::StackRestore;
	static bool doCheck (MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess) {
	  return true;
	}
	  
      };

      template<>
      struct TypeCheck<MiniMC::Model::InstructionCode::StackSave>
      {
	static const MiniMC::Model::InstructionCode OpCode = MiniMC::Model::InstructionCode::StackSave;
	static bool doCheck (MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess) {
	  return true;
	}
	  
      };
      
      bool TypeChecker::run (MiniMC::Model::Program&  prgm){
	messager.message ("Initiating Typechecking");
	bool res = true;
	for (auto& F : prgm.getFunctions ()) {
	  for (auto& E : F->getCFG()->getEdges ()) {
	    if (E->hasAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
	      for (auto& I : E->getAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
				
		switch (I.getOpcode ()) {
#define X(OP)								\
		  case MiniMC::Model::InstructionCode::OP:		\
		    if (!TypeCheck<MiniMC::Model::InstructionCode::OP>::doCheck (I,messager)) { \
		      res = false;					\
		    }							\
		    break;							
		  TACOPS
		    COMPARISONS
		    CASTOPS
		    MEMORY
		    INTERNAL
		    POINTEROPS
		    AGGREGATEOPS
		    }
	      }
	    }
	  }
	}
	if (!res) {
	  messager.error ("Type check not passing");
	}
	messager.message ("Typechecking finished");
	return res;
      }
    }
  }
}

