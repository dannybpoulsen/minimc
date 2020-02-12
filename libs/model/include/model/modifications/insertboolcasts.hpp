#ifndef _INSERTBOOLCASTS__
#define _INSERTBOOLCASTS__

#include "model/cfg.hpp"
#include "support/sequencer.hpp"


namespace MiniMC {
  namespace Model {
    namespace Modifications {
      struct InsertBoolCasts : public MiniMC::Support::Sink<MiniMC::Model::Program> {

		template<MiniMC::Model::InstructionCode From,MiniMC::Model::InstructionCode To>
		void modifyExt (MiniMC::Model::Instruction& I) {
		  if (I.getOpcode () == From) {
			MiniMC::Model::InstHelper<To>  helper(I);
		  
			if (helper.getCastee()->getType()->getTypeID () == MiniMC::Model::TypeID::Bool) {  
			  MiniMC::Model::InstBuilder<To> builder;
			  builder.setRes (helper.getResult ());
			  builder.setCastee (helper.getCastee ());
			  I.replace (builder.BuildInstruction ());
			}
		  }
		}

		
		
		virtual bool run (MiniMC::Model::Program&  prgm) {
		  for (auto& F : prgm.getFunctions ()) {
			for (auto& E : F->getCFG()->getEdges ()) {
			  if (E->hasAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
				for (auto& I : E->getAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
				  modifyExt<MiniMC::Model::InstructionCode::ZExt,MiniMC::Model::InstructionCode::BoolZExt> (I);
				  modifyExt<MiniMC::Model::InstructionCode::SExt,MiniMC::Model::InstructionCode::BoolSExt> (I);
				}
			  }
			}
		  }
		  return true;
	}
		
      };
	  
	  
    }
  }
}

#endif
