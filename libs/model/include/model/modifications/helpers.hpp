#ifndef _HELPERS__
#define _HELPERS__

#include <unordered_map>
#include <algorithm>
#include "model/cfg.hpp"
#include "support/sequencer.hpp"
#include "support/exceptions.hpp"

namespace MiniMC {
  namespace Model {
    namespace Modifications {
	  template<class T>
	  using ReplaceMap = std::unordered_map<T*,std::shared_ptr<T>>;

	  template<class Inserter>
	  void  copyInstructionAndReplace (const MiniMC::Model::Instruction& inst, const ReplaceMap<MiniMC::Model::Value>& val, Inserter insert) {
		std::vector<MiniMC::Model::Value_ptr> vals;
		auto inserter = std::back_inserter (vals);
		std::for_each (inst.begin(),inst.end(),[&](const MiniMC::Model::Value_ptr& op) {
												 if (op->isConstant()) {
												   inserter = op;
												 }
												 else if (!op->isNonCompileConstant ())  {
												   inserter = val.at(op.get());
												 }
												 else {
												   throw MiniMC::Support::Exception ("Can't copy non-compile constants");
												 }
											   }											   
		  );
		  
		assert(vals.size() == inst.getNbOps ());
		insert = MiniMC::Model::Instruction (inst.getOpcode (),vals);
	  }
	  
	  template<class Inserter>
	  void  copyEdgeAndReplace (const MiniMC::Model::Edge_ptr& edge,
								const ReplaceMap<MiniMC::Model::Value>& val,
								const ReplaceMap<MiniMC::Model::Location>& locs,
								MiniMC::Model::CFG_ptr& cfg,
								Inserter insertTo) { 
		
		auto nedge = cfg->makeEdge (locs.at(edge->getFrom ().get().get()),locs.at(edge->getTo ().get().get()),edge->getProgram());
		if (edge->hasAttribute<MiniMC::Model::AttributeType::Guard> ()) {
		  auto& guard = edge->getAttribute<MiniMC::Model::AttributeType::Guard> ();
		  nedge->setAttribute<MiniMC::Model::AttributeType::Guard> (MiniMC::Model::Guard (val.at(guard.guard.get()),guard.negate));
		}

		if (edge->hasAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
		  auto& orig = edge->getAttribute<MiniMC::Model::AttributeType::Instructions> ();
		  MiniMC::Model::InstructionStream nstr;
		  nstr.isPhi = orig.isPhi;
		  auto insert = nstr.back_inserter  ();
		  std::for_each (orig.begin(),orig.end(),[&](const MiniMC::Model::Instruction& inst) {
												   copyInstructionAndReplace (inst,val,insert);
												 });
		  
		  nedge->setAttribute<MiniMC::Model::AttributeType::Instructions> (nstr);
		}

		insertTo =  nedge;
		
	  }

	  
	  template<class LocInsert,class EdgeInsert>
	  void copyCFG (const MiniMC::Model::CFG_ptr& from,
					ReplaceMap<MiniMC::Model::Value>& val,
					MiniMC::Model::CFG_ptr to,
					const std::string pref,
					ReplaceMap<MiniMC::Model::Location>& locmap,	
					LocInsert lInsert,
					EdgeInsert eInsert) {
		for (auto& loc : from->getLocations ()) {
		  auto nloc = to->makeLocation (pref+":"+loc->getName());
		  nloc->setAttributesFlags (loc->getAttributesFlags ());
		  locmap.insert (std::pair (loc.get(),nloc));
		  lInsert = loc;
		}

		for (auto& e : from->getEdges ()) {
		  copyEdgeAndReplace<EdgeInsert> (e,val,locmap,to,eInsert);
		  
		}
		
	  }

	  
	 
	}
  }
}


#endif
