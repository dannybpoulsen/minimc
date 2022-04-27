#include "model/modifications/splitasserts.hpp"

namespace MiniMC {
  namespace Model {
    namespace Modifications {
      bool SplitAsserts::runFunction(const MiniMC::Model::Function_ptr& F) {
	auto source_loc = std::make_shared<MiniMC::Model::SourceInfo>();
	MiniMC::Model::LocationInfoCreator locc(F->getName());
	auto& cfg = F->getCFG();
	MiniMC::Support::WorkingList<MiniMC::Model::Edge_ptr> wlist;
	auto inserter = wlist.inserter();
	std::for_each(cfg.getEdges().begin(),
                        cfg.getEdges().end(),
		      [&](const MiniMC::Model::Edge_ptr& e) { inserter = e; });
	auto eloc = cfg.makeLocation(locc.make("AssertViolation", static_cast<AttrType>(MiniMC::Model::Attributes::AssertViolated), *source_loc));
	eloc->getInfo().set<MiniMC::Model::Attributes::AssertViolated>();
	
	for (auto E : wlist) {
	  if (E->hasAttribute<MiniMC::Model::AttributeType::Instructions>()) {
	    auto& instrs = E->getAttribute<MiniMC::Model::AttributeType::Instructions>();
	    if (instrs.last().getOpcode() == MiniMC::Model::InstructionCode::Assert) {
	      E->getFrom()->getInfo().unset<MiniMC::Model::Attributes::CallPlace>();
	      assert(!E->getFrom()->getInfo().is<MiniMC::Model::Attributes::CallPlace>());
	      auto val = instrs.last().getOps<MiniMC::Model::InstructionCode::Assert> ().expr;
	      instrs.erase((instrs.rbegin() + 1).base());
	      
	      auto nloc = cfg.makeLocation(locc.make("Assert", 0, *source_loc));
	      auto ttloc = E->getTo();
                E->setTo(nloc);
                auto ff_edge = cfg.makeEdge(nloc, eloc);
                ff_edge->setAttribute<MiniMC::Model::AttributeType::Guard>(MiniMC::Model::Guard(val, true));
                auto tt_edge = cfg.makeEdge(nloc, ttloc);
                tt_edge->setAttribute<MiniMC::Model::AttributeType::Guard>(MiniMC::Model::Guard(val, false));
	    }
	  }
	}
	return true;
      }
      
    }
  }
}
