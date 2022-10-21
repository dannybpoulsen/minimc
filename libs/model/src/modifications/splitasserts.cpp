#include "model/modifications/splitasserts.hpp"

namespace MiniMC {
  namespace Model {
    namespace Modifications {
      bool SplitAsserts::runFunction(const MiniMC::Model::Function_ptr& F) {
	auto source_loc = std::make_shared<MiniMC::Model::SourceInfo>();
	MiniMC::Model::LocationInfoCreator locc(F->getSymbol(),&F->getRegisterDescr ());
	auto& cfg = F->getCFA();
	MiniMC::Support::WorkingList<MiniMC::Model::Edge_ptr> wlist;
	auto inserter = wlist.inserter();
	std::for_each(cfg.getEdges().begin(),
                        cfg.getEdges().end(),
		      [&](const MiniMC::Model::Edge_ptr& e) { inserter = e; });
	auto eloc = cfg.makeLocation(locc.make("AssertViolation", static_cast<AttrType>(MiniMC::Model::Attributes::AssertViolated), *source_loc));
	eloc->getInfo().set<MiniMC::Model::Attributes::AssertViolated>();
	
	for (auto E : wlist) {
	  if (E->getInstructions ()) {
	    auto& instrs = E->getInstructions ();
	    if (instrs.last().getOpcode() == MiniMC::Model::InstructionCode::Assert) {
	      auto val = instrs.last().getOps<MiniMC::Model::InstructionCode::Assert> ().expr;
	      instrs.erase((instrs.rbegin() + 1).base());
	      
	      auto nloc = cfg.makeLocation(locc.make("Assert", 0, *source_loc));
	      auto ttloc = E->getTo();
	      E->setTo(nloc);
	      auto ff_edge = cfg.makeEdge(nloc, eloc);
	      ff_edge->getInstructions () = MiniMC::Model::InstructionStream({MiniMC::Model::createInstruction<MiniMC::Model::InstructionCode::NegAssume> ({
			.expr = val})},false);
		  
	      auto tt_edge = cfg.makeEdge(nloc, ttloc);
	      tt_edge->getInstructions () = MiniMC::Model::InstructionStream({MiniMC::Model::createInstruction<MiniMC::Model::InstructionCode::Assume> ({
			.expr = val})},false);
	    }
	  }
	}
	return true;
      }
      
    }
  }
}
