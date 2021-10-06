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
      struct StateQuery : public MiniMC::CPA::StateQuery {
	State_ptr makeInitialState (const MiniMC::Model::Program&);
	size_t nbOfProcesses (const State_ptr& ) override;
	MiniMC::Model::Location_ptr getLocation (const State_ptr&, proc_id);
      };
      
      struct Transferer : public MiniMC::CPA::Transferer  {
	State_ptr doTransfer (const State_ptr& s, const MiniMC::Model::Edge_ptr&,proc_id) override;
      };
      
      struct Joiner : public MiniMC::CPA::Joiner {  
	/** 
	 * The Location tracking CPA can only join if the two states has equal hash value.  
	 *
	 * @return 
	 */
	State_ptr doJoin (const State_ptr& l, const State_ptr& r) override {
	  if (std::hash<MiniMC::CPA::State>{} (*l) == std::hash<MiniMC::CPA::State>{} (*r))
	    return l;
	  return nullptr;
	}
	
	/** 
		 *  \p l covers \p r if their hash values are the same 
		 */
	bool covers (const State_ptr& l, const State_ptr& r) override {
	  return std::hash<MiniMC::CPA::State>{} (*l) == std::hash<MiniMC::CPA::State>{} (*r);
	}
	
		
		
      };

      using CPA = CPADef<
	StateQuery,
	Transferer,
	Joiner,
	MiniMC::CPA::Storer,
	MiniMC::CPA::PrevalidateSetup>;
      
    }

    namespace SingleLocation {
      struct StateQuery : public MiniMC::CPA::StateQuery {
	State_ptr makeInitialState (const MiniMC::Model::Program&);
	size_t nbOfProcesses (const State_ptr& ) {return 1;}
	MiniMC::Model::Location_ptr getLocation (const State_ptr&, proc_id);
      };
      
      struct Transferer {
	State_ptr doTransfer (const State_ptr& s, const MiniMC::Model::Edge_ptr&,proc_id);
      };
      
      struct Joiner : public MiniMC::CPA::Joiner {  
	/** 
	 * The Location tracking CPA can only join if the two states has equal hash value.  
	 *
	 * @return 
	 */
	State_ptr doJoin (const State_ptr& l, const State_ptr& r);
	
	/** 
	 *  \p l covers \p r if their hash values are the same 
	 */
	bool covers (const State_ptr& l, const State_ptr& r) {
	  return std::hash<MiniMC::CPA::State>{} (*l) == std::hash<MiniMC::CPA::State>{} (*r);
	}
	
	
		
      };

      using CPA = CPADef<
	StateQuery,
	Transferer,
	Joiner,
	MiniMC::CPA::Storer,
	MiniMC::CPA::PrevalidateSetup>;
      
    }
	
  }
}


#endif
