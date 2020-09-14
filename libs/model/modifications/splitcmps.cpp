#include <vector>
#include <algorithm>
#include <iostream>
#include <unordered_set>

#include "model/cfg.hpp"
#include "model/modifications/splitcmps.hpp"
#include "support/sequencer.hpp"
#include "support/workinglist.hpp"

namespace MiniMC {
  namespace Model {
    namespace Modifications {

      template<MiniMC::Model::InstructionCode i>
      bool evaluateConst (const MiniMC::Model::Instruction& instr,
						  const MiniMC::Model::Value_ptr& val,
						  MiniMC::Model::Value_ptr& cval
						  ){
		std::cerr << "Evaluate Const " << instr << " " <<std::endl;
		cval = nullptr;
		assert(i == instr.getOpcode());
		if constexpr (MiniMC::Model::InstructionData<i>::hasResVar) {
		  std::cerr << "Has Revar " << std::endl;
		  MiniMC::Model::InstHelper<i> helper (instr);
		  if (helper.getResult () == val) {
			std::cerr << "Result is same" << std::endl;
			if constexpr (i == MiniMC::Model::InstructionCode::Assign) {
			  cval = helper.getValue();
			}
			return true;
		  }
		}
		return false;
      }
      
      void killBranchingInFunction (MiniMC::Model::Function& func) {
		MiniMC::Support::WorkingList<MiniMC::Model::Edge_ptr> wlist;
		auto& edges = func.getCFG ()->getEdges ();
		std::copy_if (edges.begin(), edges.end (),wlist.inserter(),[] (auto& e) {return e->template hasAttribute<MiniMC::Model::AttributeType::Guard> ();});
	
	
		auto findConstValueFromLoc = [] (MiniMC::Model::Value_ptr& val, const MiniMC::Model::Location_ptr& loc)->std::shared_ptr<MiniMC::Model::Value>  {
		  MiniMC::Model::Location_ptr location = loc;
				       
		  while (location->nbIncomingEdges () == 1) {
			auto edge = *location->iebegin();
			if (edge->hasAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
			  auto it = edge->getAttribute<MiniMC::Model::AttributeType::Instructions> ().rbegin ();
			  auto end = edge->getAttribute<MiniMC::Model::AttributeType::Instructions> ().rend ();
			  for (; it != end; ++it) {
				MiniMC::Model::Value_ptr cval = nullptr;;
				switch (it->getOpcode ()) {
#define X(OP)															\
				  case MiniMC::Model::InstructionCode::OP:				\
					if (evaluateConst<MiniMC::Model::InstructionCode::OP> (*it,val,cval)) { \
					  if (cval && cval->isConstant()) {					\
						std::cerr << "IS constant " << std::endl;		\
						return cval;}									\
					  else												\
						return nullptr;									\
																		\
					}													\
					break;
				  OPERATIONS
					}
			  }
			  location = edge->getFrom ();
			}
					 
					 
		  }
		  return nullptr;
				       
				       
		};
		std::unordered_set<MiniMC::Model::Location_ptr>  loc_list;
		for (auto& e : wlist) {
		  auto guard = e->template getAttribute<MiniMC::Model::AttributeType::Guard> ();
		  auto val = findConstValueFromLoc(guard.guard,e->getFrom ().get());
	  
		  if (val) {
			loc_list.insert (e->getFrom ());
			assert(val->getType()->getSize() ==1);
			auto iconst = std::static_pointer_cast<MiniMC::Model::IntegerConstant<MiniMC::uint8_t>> (val);
			if (iconst->getValue () && guard.negate) {
			  func.getCFG()->deleteEdge (e);
			}
			if (!iconst->getValue () && !guard.negate ) {
			  func.getCFG()->deleteEdge (e);
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
