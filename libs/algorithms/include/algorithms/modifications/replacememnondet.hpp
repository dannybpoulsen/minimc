#ifndef _REPLACEMEM__
#define _REPLACEMEM__

#include "model/cfg.hpp"
#include "support/sequencer.hpp"


namespace MiniMC {
  namespace Algorithms {
    namespace Modifications {
      struct RemoveMemNondet : public MiniMC::Support::Sink<MiniMC::Model::Program> {
		virtual void run (MiniMC::Model::Program&  prgm) {
		  for (auto& F : prgm.getFunctions ()) {
			for (auto& E : F->getCFG()->getEdges ()) {
			  for (auto& I : E->getInstructions ()) {
				if (I.getOpcode () == MiniMC::Model::InstructionCode::Load ) {
				  MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::NonDet> nondet;
				  MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::Load> load (I);	  
				  nondet.setResult (load.getResult ());
				  I.replace (nondet.BuildInstruction ());
				}
			  }
			}
		  }
		}
		
      };

	  

	  struct ExpandNondet : public MiniMC::Support::Sink<MiniMC::Model::Program> {
		virtual void run (MiniMC::Model::Program&  prgm) {
		  for (auto& F : prgm.getFunctions ()) {
			std::vector<MiniMC::Model::Edge> todel;
			auto cfg =  F->getCFG();
			do {
			  todel.clear();
			  for (auto& E : F->getCFG()->getEdges ()) {
				std::vector<MiniMC::Model::Instruction>& instr = E->getInstructions ();
				for (auto& I : instr) {
				  
				  if (I.getOpcode () == MiniMC::Model::InstructionCode::NonDet) {
					auto from = E->getFrom();
					auto to = E->getTo ();
					MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::NonDet> nondet (I);	  
					
				  }
				}
			  }
			  
			}while (todel.size());
		  }
		  
		}
 
		
      
		
	  };
	}
  }
}

#endif