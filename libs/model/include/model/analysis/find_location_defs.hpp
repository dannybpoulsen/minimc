#ifndef _MODEL_ANALYSIS_FIND_DEFS
#define _MODEL_ANALYSIS_FIND_DEFS

#include <unordered_map>
#include <unordered_set>
#include <algorithm>


#include "model/variables.hpp"
#include "model/cfg.hpp"
#include "model/instructions.hpp"

namespace MiniMC {
  namespace Model {
    namespace Analysis {
      class LocationDefs {
      public:
	LocationDefs (std::size_t nbVars) : map (nbVars) {} 
	auto& getLocation () const {return location;}
	auto getDefsOfVariables (const MiniMC::Model::Variable_ptr& var) const  {
	  auto id = var->getId ();
	  return std::make_pair (map.at(id).begin(),map.at(id).end());
	}

	auto nbDefsForVariable (const MiniMC::Model::Variable_ptr& var) const {
	  return map.at(var->getId()).size ();
	}

	  
	
	bool insert (const MiniMC::Model::Variable_ptr& var, MiniMC::Model::Instruction* instr) {
	  auto& set = map.at(var->getId ());
	  if (set.count (instr))
	    return false;
	  else {
	    set.insert(instr);
	    return true;
	  }
	}
      private:
	MiniMC::Model::Location_ptr location;
	std::vector<std::unordered_set<MiniMC::Model::Instruction*> > map;
      };


      class CFGDefs {
      public:
	CFGDefs (const MiniMC::Model::CFG_ptr& cfg, std::size_t nbVars) {
	  std::for_each (cfg->getLocations().begin(),cfg->getLocations().end(), [&] (auto& l) {
	      locDefs.insert (std::make_pair(l,LocationDefs(nbVars)));
	      
	    });
	}
	
	
	
	auto& getDefs (const MiniMC::Model::Location_ptr& ptr) {
	  return locDefs.at(ptr);
	}
	
      private:
	std::unordered_map<MiniMC::Model::Location_ptr,LocationDefs> locDefs;
      };

      CFGDefs calculateDefs (MiniMC::Model::Function& f);
      
    }
  }
}

#endif
