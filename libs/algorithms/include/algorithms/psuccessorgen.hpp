#ifndef _PSUCCESSOR_GEN__
#define _PSUCCESSOR_GEN__

#include <gsl/pointers>
#include "cpa/interface.hpp"
#include "support/random.hpp"
#include "support/exceptions.hpp"
#include "support/types.hpp"

namespace MiniMC {
  namespace Algorithms {
    namespace Proba {
	  struct Successor {
		MiniMC::CPA::State_ptr state;
		MiniMC::Model::Edge_ptr edge;
		MiniMC::CPA::proc_id proc;      
	  };
    
	  template<class StateQuery,class Transfer>
	  class Generator {
		class Iterator {
		public:
		  Successor& operator* () {return succ;}
		  Successor* operator-> () {return &succ;}
	
		  Iterator& operator++ () {
			update();
			return *this;
		  }
	
		  bool operator== (const Iterator& oth) const {
			return false;
		  }

		  bool operator!= (const Iterator& oth) const  {
			return true;
		  }
	
		public:
		  Iterator (MiniMC::CPA::State_ptr& st) :
		    curState(st){
		    update();
		  }
		  
		  void update () {
			auto proc = StateQuery::nbOfProcesses (curState);
			std::vector<MiniMC::proc_t> procs;
			auto insert = std::back_inserter (procs); 
			for (size_t i = 0; i < proc; ++i) {
			  if (StateQuery::getLocation (curState,i)->hasOutgoingEdge())
				insert = i;
			}
			if (procs.size()) {
			  MiniMC::Support::RandomNumber random;
			  succ.proc = random.selectUniform<MiniMC::proc_t> (procs);
			  auto it = StateQuery::getLocation(curState,succ.proc)->ebegin();
			  auto end = StateQuery::getLocation(curState,succ.proc)->eend();
			  for (; it != end; ++it) {
			    auto edge = *it;
			    succ.state = Transfer::doTransfer (curState,edge,succ.proc);
			    if (succ.state) {
			      succ.edge = edge;
			      return;
			    }
			  }
			}
			succ.edge = nullptr;
			succ.state = nullptr;
		  }
	
		  MiniMC::CPA::State_ptr curState; 
		  Successor succ;
		};
	  public:
		using iterator = Iterator;
		Generator (MiniMC::CPA::State_ptr& state) :state(state) {
		}
		auto begin() {
		  return Iterator (state);}
		auto end() {return Iterator (state);}
		 
	  private:
		MiniMC::CPA::State_ptr state;
    

	  };
  
      
  

	}
  }
}
#endif
