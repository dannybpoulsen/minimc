#include "model/modifications/splitasserts.hpp"

namespace MiniMC {
  namespace Model {
    namespace Modifications {
      bool SplitAsserts::runFunction(const MiniMC::Model::Function_ptr& F) {
	auto frame = F->getFrame ();
	auto source_loc = std::make_shared<MiniMC::Model::SourceInfo>();
	MiniMC::Model::LocationInfoCreator locc(F->getRegisterDescr ());
	auto& cfg = F->getCFA();
	MiniMC::Support::WorkingList<MiniMC::Model::Edge_ptr> wlist;
	auto inserter = wlist.inserter();
	std::for_each(cfg.getEdges().begin(),
                        cfg.getEdges().end(),
		      [&](const MiniMC::Model::Edge_ptr& e) { inserter = e; });
	auto info = locc.make({MiniMC::Model::Attributes::AssertViolated}, *source_loc);
	auto eloc = cfg.makeLocation(frame.makeFresh (),info);
	eloc->getInfo().getFlags () |= MiniMC::Model::Attributes::AssertViolated;
	
	for (auto E : wlist) {
	  if (E->getInstructions ()) {
	    auto instrs = E->getInstructions ();
	    if (instrs.last().getOpcode() == MiniMC::Model::InstructionCode::Assert) {
	      auto val = instrs.last().getOps<MiniMC::Model::InstructionCode::Assert> ().expr;
	      instrs.erase((instrs.rbegin() + 1).base());
	      auto info = locc.make( {}, *source_loc);
	      auto nloc = cfg.makeLocation(frame.makeFresh (), info);
	      auto ttloc = E->getTo();
	      
	      cfg.makeEdge (E->getFrom (),nloc,std::move(instrs)); 
	      cfg.deleteEdge (E.get());
	      auto ff_edge = cfg.makeEdge(nloc, eloc, MiniMC::Model::InstructionStream({MiniMC::Model::Instruction::make<MiniMC::Model::InstructionCode::NegAssume> (val)}));
	     
	      
	      auto tt_edge = cfg.makeEdge(nloc, ttloc,MiniMC::Model::InstructionStream({MiniMC::Model::Instruction::make<MiniMC::Model::InstructionCode::Assume> (val)}));
	    }
	  }
	}
	return true;
      }
      
    }
  }
}

