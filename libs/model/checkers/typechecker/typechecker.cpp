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
	bool doCheck (MiniMC::Model::Instruction&, MiniMC::Support::Messager& ) {return true;}
      };

      template<MiniMC::Model::InstructionCode i>
      struct TypeCheck<i,typename std::enable_if<MiniMC::Model::InstructionData<i>::isTAC> > {
	bool doCheck (MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess) {
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

      template<MiniMC::Model::InstructionCode i>
      struct TypeCheck<i,typename std::enable_if<MiniMC::Model::InstructionData<i>::isComparison> > {
	bool doCheck (MiniMC::Model::Instruction& inst, MiniMC::Support::Messager& mess) {
	  MiniMC::Support::Localiser optype ("All operands to '{}' must have same type."); 
	  MiniMC::Support::Localiser restype ("The result of  '{}' must have boolean type."); 
	  
	  InstHelper<i> h (inst);
	  auto resType = h.getResult ()->getType ();
	  auto lType = h.getLeftOp ()->getType ();
	  auto rType = h.getRightOp ()->getType ();
	  if (resType->getTypeID () != MiniMC::Model::TypeID::Bool) {
	    mess.error (restype.format(i));; 
	    return false;
	  }
	  if (lType != rType) {
	    mess.error (optype.format (i));
	    return false;
	  }
	  return true;
	}
      };
      
      bool TypeChecker::run (const MiniMC::Model::Program&  prgm){
	messager->message ("Initiating Typechecking");
	messager->message ("Typechecking finished");
	return true;
      }
    }
  }
}
