
#include <unordered_set>

#include "support/queue_stack.hpp"
#include "model/analysis/find_location_defs.hpp"
#include "model/utils.hpp"

namespace MiniMC {
  namespace Model {
    namespace Analysis {
      CFGDefs_ptr calculateDefs (MiniMC::Model::Function& f) {
		auto& cfg = f.getCFG ();
		auto& vStack = f.getVariableStackDescr ();
		CFGDefs_ptr defs = std::make_shared<CFGDefs> (cfg,vStack->getTotalVariables ());
		MiniMC::Support::Queue<MiniMC::Model::Location> waiting;
		for (auto loc : cfg->getLocations ())
		  waiting.insert (loc);

		while (!waiting.empty ()) {
		  auto curLoc = waiting.pull ();
		  auto& ldefs = defs->getDefs (curLoc);
		  auto addEdge = [&] (const MiniMC::Model::Edge_ptr& edge ) {
			
			bool changed = false;
			std::unordered_set<MiniMC::Model::Variable_ptr> varsDefByEdge;
			auto& edgeDefs = defs->getDefs(edge->getTo ());
			assert(edge->getFrom ().get() == curLoc);
	    
			if (edge->template hasAttribute<MiniMC::Model::AttributeType::Instructions> () ) {
			  auto& instrs = edge->template getAttribute<MiniMC::Model::AttributeType::Instructions> ();
			  
			  for (auto iit = instrs.rbegin(); iit != instrs.rend();++iit) {
				auto valueDefined = valueDefinedBy (*iit);
		
				if (valueDefined) {
				  MiniMC::Model::Variable_ptr varDefined = std::static_pointer_cast<MiniMC::Model::Variable> (valueDefined);
				  if (varsDefByEdge.count (varDefined))
					continue;
				  varsDefByEdge.insert(varDefined);;
				  changed |= edgeDefs.insert (varDefined,&*iit);
				}
			  }
			}
			
			for (auto& v : vStack->getVariables ()) {
			  if (varsDefByEdge.count (v))
				continue;
			  auto variableDefinitions = ldefs.getDefsOfVariables (v);
			  auto it = variableDefinitions.first;
			  auto end = variableDefinitions.second;
			  for (; it != end; ++it) {
		
				changed |= edgeDefs.insert(v, *it);
			  }
			}
	    
			
			if (changed) {
			  
			  waiting.insert (edge->getTo ());  
			}
		  };
		  
		  for (auto eit = curLoc->ebegin(); eit != curLoc->eend (); ++eit) {
			addEdge (*eit);
		  }
	  
		}
		return defs;
	
      }
    }
  }
}
