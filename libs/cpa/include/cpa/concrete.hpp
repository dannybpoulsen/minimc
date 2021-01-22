
#ifndef _CPA_CONCRETE_INTERFACE__
#define _CPA_CONCRETE_INTERFACE__

#include <ostream>
#include <memory>
#include <unordered_map>
#include "cpa/state.hpp"
#include "cpa/interface.hpp"
#include "support/sequencer.hpp"
#include "support/feedback.hpp"
#include "model/cfg.hpp"
#include "hash/hashing.hpp"

namespace MiniMC {
  namespace CPA {
	namespace Concrete {
	  struct StateQuery {
		
		static MiniMC::CPA::State_ptr makeInitialState (const MiniMC::Model::Program&);
		
		static size_t nbOfProcesses (const MiniMC::CPA::State_ptr& ) {return 0;}

		static MiniMC::Model::Location_ptr getLocation (const MiniMC::CPA::State_ptr&, proc_id id) {return nullptr;}
	  };
	  
	  struct Transferer {
		
		static MiniMC::CPA::State_ptr doTransfer (const MiniMC::CPA::State_ptr& s, const MiniMC::Model::Edge_ptr& e,proc_id id) {return nullptr;}
	  };

	
	  struct Joiner {  
		static MiniMC::CPA::State_ptr doJoin (const MiniMC::CPA::State_ptr& l, const MiniMC::CPA::State_ptr& r) {return nullptr;}
		static bool covers (const MiniMC::CPA::State_ptr& l, const MiniMC::CPA::State_ptr& r) {
		  return false;
		}
		
		static void coverCopy (const MiniMC::CPA::State_ptr& from, MiniMC::CPA::State_ptr& to) {}
		
	  };
	  
	  struct PrevalidateSetup {
		static void validate (MiniMC::Support::Sequencer<MiniMC::Model::Program>& seq, MiniMC::Support::Messager& mess) {}
	  };
	  
	
    
	  struct CPADef {
		using Query = StateQuery; 
		using Transfer = Transferer;
		using Join = Joiner; 
		using Storage = MiniMC::CPA::Storer<Join>; 
		using PreValidate = PrevalidateSetup;  
	  };
	}
  }
}

#endif

