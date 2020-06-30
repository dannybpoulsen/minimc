/**
 * @file   location.hpp
 * @date   Mon Apr 20 18:20:00 2020
 * 
 *  This file contains the definition of a Location tracking CPA
 * 
 * 
 */
#ifndef _LOCATION__
#define _LOCATION__

#include "model/cfg.hpp"
#include "cpa/interface.hpp"

namespace MiniMC {
  namespace CPA {
    namespace Location {
      struct StateQuery {
		static State_ptr makeInitialState (const MiniMC::Model::Program&);
		static size_t nbOfProcesses (const State_ptr& );
		static MiniMC::Model::Location_ptr getLocation (const State_ptr&, proc_id);
	};
      
      struct Transferer {
		static State_ptr doTransfer (const State_ptr& s, const MiniMC::Model::Edge_ptr&,proc_id);
      };
      
      struct Joiner {  
	/** 
	 * The Location tracking CPA can only join if the two states has equal hash value.  
	 *
	 * @return 
	 */
	static State_ptr doJoin (const State_ptr& l, const State_ptr& r) {
	  if (std::hash<MiniMC::CPA::State>{} (*l) == std::hash<MiniMC::CPA::State>{} (*r))
	    return l;
	  return nullptr;
	}
	
	/** 
	 *  \p l covers \p r if their hash values are the same 
	 */
	static bool covers (const State_ptr& l, const State_ptr& r) {
	  return std::hash<MiniMC::CPA::State>{} (*l) == std::hash<MiniMC::CPA::State>{} (*r);
	}
	
	static void coverCopy (const State_ptr& from, State_ptr& to) {
	}
		
      };
	  
      struct CPADef {
	using Query = StateQuery;
	using Transfer = Transferer;
	using Join = Joiner;
	using Storage = MiniMC::CPA::Storer<Join>; 
	using PreValidate = MiniMC::CPA::PrevalidateSetup;
      };
    }
  }
}


#endif
