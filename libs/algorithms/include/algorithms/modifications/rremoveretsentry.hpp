#ifndef _removeretsentry__
#define _removeretsentry__

#include "model/cfg.hpp"
#include "support/sequencer.hpp"

namespace MiniMC {
  namespace Algorithms {
    namespace Modifications {
      struct RemoveRetEntryPoints : public MiniMC::Support::Sink<MiniMC::Model::Program> {
	virtual void run (MiniMC::Model::Program&  prgm) {
	  for (auto& F : prgm.getEntryPoints ()) {
	    for (auto& E : F->getCFG()->getEdges ()) {
	      if (E->hasAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
		for (auto& I : E->getAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
		  if (I.getOpcode () == MiniMC::Model::InstructionCode::Ret ||
		      I.getOpcode () == MiniMC::Model::InstructionCode::RetVoid) {
		    MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::Skip> skip; 
		    I.replace (skip.BuildInstruction ());
		  }
		}
	      }
	    }
	  }
	}

      };
      
    }
  }
}


#endif 
