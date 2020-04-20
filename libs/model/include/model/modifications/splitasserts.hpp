#ifndef _SPLITASSERTS__
#define _SPLITASSERT__

#include <algorithm>

#include "model/cfg.hpp"
#include "support/sequencer.hpp"
#include "support/workinglist.hpp"


namespace MiniMC {
  namespace Model {
    namespace Modifications {
	  /**
	   *
	   * For each \ref MiniMC::Model::InstructionCode::Assert
	   * instruction, a special Location with \ref
	   * MiniMC::Location::Attributes::AssertViolated set is inserted.
	   * The edge containing the Instruction is split, such that the
	   * one edge goes to the current successor Location while one goes newly
	   * constructed Location.  
	   * In this way exploration algorithms do not need to special
	   * handling for \ref  MiniMC::Model::InstructionCode::Assert
	   * instructions, but can instead for locations with \ref
	   * MiniMC::Location::Attributes::AssertViolated set.
	   */
	  struct SplitAsserts : public MiniMC::Support::Sink<MiniMC::Model::Program> {
		virtual bool run (MiniMC::Model::Program&  prgm) {
		  for (auto& F : prgm.getFunctions ()) {
			auto cfg = F->getCFG ();
			auto it = cfg->getEdges ().begin();
			auto end = cfg->getEdges ().end ();
			MiniMC::Support::WorkingList<MiniMC::Model::Edge_ptr> wlist;
			auto inserter =wlist.inserter ();
			std::for_each (cfg->getEdges().begin(),
						   cfg->getEdges().end (),
						   [&](const MiniMC::Model::Edge_ptr& e) {inserter = e;}
						   );
	    
			for (auto E : wlist) {
			  if (E->hasAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
				auto& instrs = E->getAttribute<MiniMC::Model::AttributeType::Instructions> ();
				if (instrs.last().getOpcode () == MiniMC::Model::InstructionCode::Assert) {
				  E->getFrom()->unset<MiniMC::Model::Location::Attributes::CallPlace> ();
				  assert(!E->getFrom ()->is<MiniMC::Model::Location::Attributes::CallPlace> ());
				  auto val = MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::Assert> (instrs.last ()).getAssert ();
				  instrs.erase ((instrs.rbegin()+1).base());
		   
				  auto eloc = cfg->makeLocation ("AssertViolation");
				  eloc->set<MiniMC::Model::Location::Attributes::AssertViolated> ();
				  auto nloc = cfg->makeLocation ("");
				  auto ttloc = E->getTo ();
				  E->setTo (nloc);
				  auto ff_edge = cfg->makeEdge (nloc,eloc,prgm.shared_from_this());
				  ff_edge->setAttribute<MiniMC::Model::AttributeType::Guard> (MiniMC::Model::Guard (val,true));
				  auto tt_edge = cfg->makeEdge (nloc,ttloc,prgm.shared_from_this());
				  tt_edge->setAttribute<MiniMC::Model::AttributeType::Guard> (MiniMC::Model::Guard (val,false));
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
