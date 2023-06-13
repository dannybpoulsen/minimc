#include "model/modifications/insertboolcasts.hpp"
#include "support/overload.hpp"

namespace MiniMC {
  namespace Model {
    namespace Modifications {
      template <MiniMC::Model::InstructionCode From, MiniMC::Model::InstructionCode To>
      void modifyExt(MiniMC::Model::Instruction& I) {	
	auto& content = I.getAs<From> ().getOps ();
	if (content.op1->getType()->getTypeID() == MiniMC::Model::TypeID::Bool) {
	  I = MiniMC::Model::Instruction::make<To>(content.res, content.op1);
	}
      }
      
      
      bool InsertBoolCasts::runFunction(const MiniMC::Model::Function_ptr& F) {
	for (auto& E : F->getCFA().getEdges()) {
	  if (E->getInstructions () ) {
	    for (auto& I : E->getInstructions()) {
	      I.visit (MiniMC::Support::Overload {
		  [&I](const MiniMC::Model::TInstruction<MiniMC::Model::InstructionCode::ZExt>& ) {
		    modifyExt<MiniMC::Model::InstructionCode::ZExt,MiniMC::Model::InstructionCode::BoolZExt> (I);
		  },
		  [&I](const MiniMC::Model::TInstruction<MiniMC::Model::InstructionCode::SExt>&) {
		    modifyExt<MiniMC::Model::InstructionCode::SExt,MiniMC::Model::InstructionCode::BoolSExt> (I);
		  },
		  [](auto&) {}
		}
		);
	    }
	  }

	}
	return true;
      }
    }
  }
}
