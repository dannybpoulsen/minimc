#include "minimc/model/modifications/splitasserts.hpp"
#include "minimc/model/variables.hpp"
#include "minimc/support/workinglist.hpp"
#include "minimc/support/overload.hpp"

namespace MiniMC {
  namespace Model {
    namespace Modifications {
      bool SplitAsserts::runFunction(const MiniMC::Model::Function_ptr& F) {
	auto frame = F->getFrame ();
	//MiniMC::Model::LocationInfoCreator locc(F->getRegisterDescr ());
	auto& cfg = F->getCFA();
	MiniMC::Support::WorkingList<MiniMC::Model::Edge_ptr> wlist;
	auto inserter = wlist.inserter();
	std::for_each(cfg.getEdges().begin(),
                        cfg.getEdges().end(),
		      [&](const MiniMC::Model::Edge_ptr& e) { inserter = e; });
	auto info = MiniMC::Model::LocationInfo{{MiniMC::Model::Attributes::AssertViolated},F->getFrame()};
	auto eloc = cfg.makeLocation(frame.makeFresh ("Assert"),info);
	eloc->getInfo().getFlags () |= MiniMC::Model::Attributes::AssertViolated;
	
	for (auto E : wlist) {
	  if (E->getInstructions ()) {
	    auto instrs = E->getInstructions ();
	    instrs.last().visit (MiniMC::Support::Overload {
		[&instrs,&cfg,&frame,&E,&eloc,&F](const MiniMC::Model::TInstruction<MiniMC::Model::VMInstructionCode::Assert>& instr) {
		  auto val = instr.getOps ().expr;
		  instrs.erase((instrs.rbegin() + 1).base());
		  MiniMC::Model::LocationInfo info{ {},F->getFrame()};
		  auto nloc = cfg.makeLocation(frame.makeFresh (), info);
		  auto ttloc = E->getTo();
		  
		  cfg.makeEdge (E->getFrom (),nloc,std::move(instrs)); 
		  cfg.deleteEdge (E.get());
		  auto ff_edge = cfg.makeEdge(nloc, eloc, MiniMC::Model::InstructionStream({MiniMC::Model::Instruction::make<MiniMC::Model::VMInstructionCode::Assume> (std::make_shared<MiniMC::Model::LogNotExpr>(val))}));;
		  
		    
		  auto tt_edge = cfg.makeEdge(nloc, ttloc,MiniMC::Model::InstructionStream({MiniMC::Model::Instruction::make<MiniMC::Model::VMInstructionCode::Assume> (val)}));
		  
		},
		[] (auto& ) {}
		});
	    
	    }
	  }
	return true;
      }
      
    }
  }
}

