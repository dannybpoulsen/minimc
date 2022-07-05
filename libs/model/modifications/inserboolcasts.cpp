#include "model/modifications/insertboolcasts.hpp"

namespace MiniMC {
  namespace Model {
    namespace Modifications {
      template <MiniMC::Model::InstructionCode From, MiniMC::Model::InstructionCode To>
      void modifyExt(MiniMC::Model::Instruction& I) {
	if (I.getOpcode() == From) {
	  auto& content = I.getOps<To>();
	  if (content.op1->getType()->getTypeID() == MiniMC::Model::TypeID::Bool) {
	    I.replace(MiniMC::Model::createInstruction<To>({.res = content.res, .op1 = content.op1}));
	  }
	}
      }
      
      
      bool InsertBoolCasts::runFunction(const MiniMC::Model::Function_ptr& F) {
	for (auto& E : F->getCFA().getEdges()) {
	  if (E->getInstructions () ) {
	    for (auto& I : E->getInstructions()) {
	      modifyExt<MiniMC::Model::InstructionCode::ZExt, MiniMC::Model::InstructionCode::BoolZExt>(I);
	      modifyExt<MiniMC::Model::InstructionCode::SExt, MiniMC::Model::InstructionCode::BoolSExt>(I);
	    }
	  }
	}
          return true;
      }
    }
  }
}
