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
			if (std::static_pointer_cast<Constant> (op)->isNonCompileConstant ()) {
			  throw MiniMC::Support::Exception ("Can't copy non-compile constants");}
			inserter = op;
		  }
		  else   {
			inserter = val.at(op.get());
		  }
		}										   
		  );
		  
		assert(vals.size() == inst.getNbOps ());
		insert = MiniMC::Model::Instruction (inst.getOpcode (),vals);
	  }
	  
	  
	  inline void  copyEdgeAnd (const MiniMC::Model::Edge_ptr& edge,
						 const ReplaceMap<MiniMC::Model::Location>& locs,
						 MiniMC::Model::CFG_ptr& cfg
						 ) { 
		
		auto to = (locs.count (edge->getTo ().get().get())) ? locs.at (edge->getTo ().get().get()) : edge->getTo ().get();
		auto from = (locs.count (edge->getFrom ().get().get())) ? locs.at (edge->getFrom ().get().get()) : edge->getFrom ().get ();
		
		auto nedge = cfg->makeEdge (from,to);
		if (edge->hasAttribute<MiniMC::Model::AttributeType::Guard> ()) {
		  auto& guard = edge->getAttribute<MiniMC::Model::AttributeType::Guard> ();
		  nedge->setAttribute<MiniMC::Model::AttributeType::Guard> (guard);
		}

		if (edge->hasAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
		  auto& orig = edge->getAttribute<MiniMC::Model::AttributeType::Instructions> ();
		  MiniMC::Model::InstructionStream nstr;
		  nstr.isPhi = orig.isPhi;
		  auto insert = nstr.back_inserter  ();
		  std::for_each (orig.begin(),orig.end(),[&](const MiniMC::Model::Instruction& inst) {
			insert = inst;
		  });
		  
		  nedge->setAttribute<MiniMC::Model::AttributeType::Instructions> (nstr);
		}

	
		
	  }

	  template<class LocInsert,class LocInserter>
	  void copyLocation (MiniMC::Model::CFG_ptr to, const MiniMC::Model::Location_ptr& loc , LocInsert inserter, LocInserter linserter,const std::string pref  ="") {
		auto nloc = to->makeLocation (loc->getInfo());
		inserter = std::make_pair (loc.get(),nloc);
		linserter = nloc.get();
	  }

	  template<class Inserter>
	  void  copyEdgeAndReplace (const MiniMC::Model::Edge_ptr& edge,
								const ReplaceMap<MiniMC::Model::Value>& val,
								const ReplaceMap<MiniMC::Model::Location>& locs,
								MiniMC::Model::CFG_ptr& cfg,
								Inserter insertTo) {
		auto to = (locs.count (edge->getTo ().get().get())) ? locs.at (edge->getTo ().get().get()) : edge->getTo ().get();
		auto from = (locs.count (edge->getFrom ().get().get())) ? locs.at (edge->getFrom ().get().get()) : edge->getFrom ().get ();
		
		auto nedge = cfg->makeEdge (from,to);
		if (edge->hasAttribute<MiniMC::Model::AttributeType::Guard> ()) {
		  auto& guard = edge->getAttribute<MiniMC::Model::AttributeType::Guard> ();
		  
		  nedge->setAttribute<MiniMC::Model::AttributeType::Guard> ( MiniMC::Model::Guard (val.at (guard.guard.get()),guard.negate ));
		}
		
		if (edge->hasAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
		  auto& orig = edge->getAttribute<MiniMC::Model::AttributeType::Instructions> ();
		  MiniMC::Model::InstructionStream nstr;
		  nstr.isPhi = orig.isPhi;
		  auto insert = nstr.back_inserter  ();
		  std::for_each (orig.begin(),orig.end(),[&](const MiniMC::Model::Instruction& inst) {
			std::vector<MiniMC::Model::Value_ptr> vals;
			for (auto it = inst.begin(); it!=inst.end (); ++it) {
			  vals.push_back ((*it)->isConstant () ? *it : val.at (it->get()) ); 
			}
			insert = MiniMC::Model::Instruction (inst.getOpcode (), vals);
		  });
		  
		  nedge->setAttribute<MiniMC::Model::AttributeType::Instructions> (nstr);
		}

		insertTo = nedge;

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
		  auto nloc = to->makeLocation (loc->getInfo());
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
