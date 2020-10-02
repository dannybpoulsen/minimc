#include <vector>
#include <algorithm>
#include <iostream>
#include <unordered_set>

#include "model/cfg.hpp"
#include "model/modifications/splitcmps.hpp"
#include "support/sequencer.hpp"
#include "support/workinglist.hpp"
#include "model/analysis/find_location_defs.hpp"


namespace MiniMC {
  namespace Model {
    namespace Modifications {

      template<MiniMC::Model::InstructionCode i>
      bool evaluateConst (const MiniMC::Model::Instruction& instr,
			  const MiniMC::Model::Value_ptr& val,
			  MiniMC::Model::Value_ptr& cval
			  ){
	cval = nullptr;
	assert(i == instr.getOpcode());
	if constexpr (MiniMC::Model::InstructionData<i>::hasResVar) {
	  MiniMC::Model::InstHelper<i> helper (instr);
	  if (helper.getResult () == val) {
	    if constexpr (i == MiniMC::Model::InstructionCode::Assign) {
	      cval = helper.getValue();
	      return true;
	    }
	  }
	}
	return false;
      }

      bool evalConst (const MiniMC::Model::Instruction& instr,
		      const MiniMC::Model::Value_ptr& val,
		      MiniMC::Model::Value_ptr& cval
		      ){
	switch (instr.getOpcode ()) {
#define X(OP)								\
	  case MiniMC::Model::InstructionCode::OP:			\
	    return evaluateConst<MiniMC::Model::InstructionCode::OP> (instr,val,cval); \
	      								\
	    break;
	  OPERATIONS
	    }
	return false;
      }
      void killBranchingInFunction (MiniMC::Model::Function& func) {
	auto cfgdefs = MiniMC::Model::Analysis::calculateDefs (func);
	MiniMC::Support::WorkingList<MiniMC::Model::Edge_ptr> wlist;
	auto& edges = func.getCFG ()->getEdges ();
	std::copy_if (edges.begin(), edges.end (),wlist.inserter(),[] (auto& e) {return e->template hasAttribute<MiniMC::Model::AttributeType::Guard> () && !e->template hasAttribute<MiniMC::Model::AttributeType::Instructions> () ;});
	
	
	std::unordered_set<MiniMC::Model::Location_ptr>  loc_list;
	for (auto& e : wlist) { 
	  auto guard = e->template getAttribute<MiniMC::Model::AttributeType::Guard> ();
	  assert(!e->template hasAttribute<MiniMC::Model::AttributeType::Instructions> ());
	  //Grab definitions of predecessor location (as that must be the definitions used when moving along this edge) 
	  auto& locdefs = cfgdefs.getDefs (e->getFrom ());
	  auto guard_var = std::static_pointer_cast<MiniMC::Model::Variable> (guard.guard);
	  if (locdefs.nbDefsForVariable (guard_var) == 1) {
	    auto instr_tup = locdefs.getDefsOfVariables (guard_var);
	    auto instr = *instr_tup.first;
	    MiniMC::Model::Value_ptr constant;
	    if (evalConst (*instr,guard.guard,constant)) {
	      auto iconst = std::static_pointer_cast<MiniMC::Model::IntegerConstant<MiniMC::uint8_t>> (constant);
	      if (iconst->getValue () && guard.negate) {
		func.getCFG()->deleteEdge (e);
	      }
	      if (!iconst->getValue () && !guard.negate ) {
		func.getCFG()->deleteEdge (e);
	      }
	      loc_list.insert (e->getFrom ());
	    }
	    
	  }
	}
		  
		
	for (auto& loc : loc_list) {
	  if (loc->nbOutgoingEdges () == 1) {
	    //At this point the only edge left is always satisfied... Just delete it.
	    assert(loc->nbIncomingEdges () == 1);
	    auto in = loc->iebegin();
	    auto out = loc->ebegin();
	    in->setTo (out->getTo ());
	    func.getCFG()->deleteLocation (loc);
		    
	  }
	}
		
		
		
      }
	  
      bool KillUnneededBranching::run (MiniMC::Model::Program&  prgm) {
	for (auto& F : prgm.getFunctions ()) {
	  killBranchingInFunction (*F);
	}
	return true;
      }
      
      
    }
  }
}
