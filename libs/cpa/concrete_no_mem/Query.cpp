#include "cpa/concrete_no_mem.hpp"
#include "stack.hpp"
#include "state.hpp"


namespace MiniMC {
  namespace CPA {
    namespace ConcreteNoMem {
      MiniMC::CPA::State_ptr StateQuery::makeInitialState (const MiniMC::Model::Program&) {
	
      }
      
      static size_t nbOfProcesses (const State_ptr& s) {
	auto& state = static_cast<MiniMC::CPA::ConcreteNoMem::State&> (*s);
	return  state.nbProcs  ();
      }
      static MiniMC::Model::Location_ptr getLocation (const State_ptr&, proc_id);
      
      
    }
  }
}
