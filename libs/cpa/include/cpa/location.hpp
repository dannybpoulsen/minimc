#ifndef _LOCATION__
#define _LOCATION__

#include <ostream>
#include <memory>
#include "model/cfg.hpp"
#include "hash/hashing.hpp"

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
	static State_ptr doJoin (const State_ptr& l, const State_ptr& r) {return r;}
      };
      
      struct CPADef {
	using Query = StateQuery;
	using Transfer = Transferer;
	using Joing = Joiner;
	using Storage = MiniMC::CPA::Storer; 
    };
    }
  }
}


#endif
