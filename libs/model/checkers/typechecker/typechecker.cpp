#include "model/checkers/typechecker.hpp"

#include "model/cfg.hpp"
#include "model/cfg.hpp"
#include "support/feedback.hpp"
#include "support/localisation.hpp"


namespace MiniMC {
  namespace Model {
    namespace Checkers {
      template<MiniMC::Model::InstructionCode,typename t = void> 
      struct TypeCheck {
		static bool doCheck (MiniMC::Model::Instruction&, MiniMC::Support::Messager& ) {return false;}
      };

      template<MiniMC::Model::InstructionCode i>
      struct TypeCheck<i,typename std::enable_if<MiniMC::Model::InstructionData<i>::isTAC> > {
		static bool doCheck (MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess) {
		  MiniMC::Support::Localiser loc ("All operands to '{}' must have same type as the result."); 
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
	  
      template<>
      struct TypeCheck<MiniMC::Model::InstructionCode::Trunc,void > {
		static bool doCheck (MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess) {
		  MiniMC::Support::Localiser trunc_must_be_integer ("'{}' can only be applied to integer types. "); 
		  MiniMC::Support::Localiser trunc_must_be_larger ("From type must be larger that to type for '{}'"); 
		  
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
		  MiniMC::Support::Localiser must_be_integer ("'{}' can only be applied to integer types. "); 
		  MiniMC::Support::Localiser must_be_smaller ("From type must be smaller that to type for '{}'"); 
		  
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
		  MiniMC::Support::Localiser must_be_integer ("'{}' can only be applied to integer types. "); 
		  MiniMC::Support::Localiser must_be_pointer ("Return type has to be pointer for '{}'"); 
		  
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
		  MiniMC::Support::Localiser must_be_pointer ("'{}' can only be applied to pointer types. "); 
		  MiniMC::Support::Localiser must_be_integer ("Return type has to be integer for '{}'"); 
		  
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
  
  
	  bool TypeChecker::run (MiniMC::Model::Program&  prgm){
		messager.message ("Initiating Typechecking");
		bool res = true;
		for (auto& F : prgm.getFunctions ()) {
		  for (auto& E : F->getCFG()->getEdges ()) {
			if (E->hasAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
			  for (auto& I : E->getAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
				
				switch (I.getOpcode ()) {
#define X(OP)															\
				  case MiniMC::Model::InstructionCode::OP:				\
					if (!TypeCheck<MiniMC::Model::InstructionCode::OP>::doCheck (I,messager)) { \
					  res = false;										\
					}													\
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

