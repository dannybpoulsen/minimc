#ifndef _REPLACEMEM__
#define _REPLACEMEM__

#include "model/cfg.hpp"
#include "support/sequencer.hpp"


namespace MiniMC {
  namespace Model {
    namespace Modifications {
      struct RemoveMemNondet : public MiniMC::Support::Sink<MiniMC::Model::Program> {
	virtual bool run (MiniMC::Model::Program&  prgm) {
	  for (auto& F : prgm.getFunctions ()) {
	    for (auto& E : F->getCFG()->getEdges ()) {
	      if (E->hasAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
		for (auto& I : E->getAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
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
	  return true;
	}
		
      };

	  

      struct ExpandNondet : public MiniMC::Support::Sink<MiniMC::Model::Program> {
	virtual bool run (MiniMC::Model::Program&  prgm) {
	  for (auto& F : prgm.getFunctions ()) {
	    std::vector<MiniMC::Model::Edge> todel;
	    auto cfg =  F->getCFG();
	    do {
	      todel.clear();
	      for (auto& E : F->getCFG()->getEdges ()) {
		if (E->hasAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
		  for (auto& I : E->getAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
					
		    if (I.getOpcode () == MiniMC::Model::InstructionCode::NonDet) {
		      auto from = E->getFrom();
		      auto to = E->getTo ();
		      MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::NonDet> nondet (I);	  
					  
		    }
		  }
		}
	      }
			  
	    }while (todel.size());
	  }
	  return true;
	}
      };
    }
  }
}

#endif
