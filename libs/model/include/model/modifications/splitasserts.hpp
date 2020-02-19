#ifndef _SPLITASSERTS__
#define _SPLITASSERT__

#include "model/cfg.hpp"
#include "support/sequencer.hpp"


namespace MiniMC {
  namespace Model {
    namespace Modifications {
      struct SplitAsserts : public MiniMC::Support::Sink<MiniMC::Model::Program> {
	

		
		
	virtual bool run (MiniMC::Model::Program&  prgm) {
	  for (auto& F : prgm.getFunctions ()) {
	    auto cfg = F->getCFG ();
	    auto it = cfg->getEdges ().begin();
	    auto end = cfg->getEdges ().end ();
	    for (; it!=end; ++it) {
	      auto& E = *it;
	      if (E->hasAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
		auto& instrs = E->getAttribute<MiniMC::Model::AttributeType::Instructions> ();
		if (instrs.last().getOpcode () == MiniMC::Model::InstructionCode::Assert) {
		  auto val = MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::Assert> (instrs.last ()).getAssert ();
		  instrs.erase ((instrs.rbegin()+1).base());
		   
		  auto eloc = cfg->makeLocation ("AssertViolation");
		  eloc->set<MiniMC::Model::Location::Attributes::Error> ();
		  auto nloc = cfg->makeLocation ("");
		  auto ttloc = E->getTo ();
		  E->setTo (nloc);
		  auto ff_edge = cfg->makeEdge (nloc,eloc,prgm.shared_from_this());
		  ff_edge->setAttribute<MiniMC::Model::AttributeType::Guard> (MiniMC::Model::Guard (val,true));
		  auto tt_edge = cfg->makeEdge (nloc,ttloc,prgm.shared_from_this());
		  tt_edge->setAttribute<MiniMC::Model::AttributeType::Guard> (MiniMC::Model::Guard (val,false));
		  it = cfg->getEdges().begin();
		  end = cfg->getEdges ().end();
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
