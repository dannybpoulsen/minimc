#ifndef _SUCCESSOR_GEN__
#define _SUCCESSOR_GEN__

#include <gsl/pointers>
#include "cpa/interface.hpp"
#include "support/exceptions.hpp"
#include "support/types.hpp"

namespace MiniMC {
  namespace Algorithms {
    enum class ErrorFlags {
      AssertViolated = 1,
    };
	
	
    struct Successor {
      MiniMC::CPA::State_ptr state;
      MiniMC::Model::Edge_ptr edge;
      MiniMC::CPA::proc_id proc;      
      MiniMC::uint8_t eflags;
      bool hasErrors () const {return eflags;}
    };

	/** 
	 * Generator of successor states. 
	 * This class takes a State as input, and exposes an iterator interface to generate all possible successors.
	 * \tparam StateQuery class used for Querying the state for location and successors
	 * \tparam Transfer Transfer relation for executing an Edge in a state
	 */
    template<class StateQuery,class Transfer>
    class Generator {
      class Iterator {
      public:

		static Iterator makeBegin (MiniMC::CPA::State_ptr& pt) {
		  auto loc = pt->getLocation (0);
		  return Iterator(pt,0,pt->nbOfProcesses (),loc->ebegin(),loc->eend());
		}
		
		static Iterator makeEnd (MiniMC::CPA::State_ptr& pt) {
		  auto proc = pt->nbOfProcesses ();
		  auto loc = pt->getLocation (proc-1);
		  return Iterator(pt,proc-1,proc,loc->eend(),loc->eend());
		}
		
		Successor& operator* () {return succ;}
		Successor* operator-> () {return &succ;}
		
		Iterator& operator++ () {
		  ++iter;
		  update();
		  return *this;
		}
		
		bool operator== (const Iterator& oth) const {
		  return oth.proc == proc &&
			oth.iter == iter &&
			oth.end == end;
		}
		
		bool operator!= (const Iterator& oth) const  {
		  return !(*this == oth);
		}
		
      private:
		Iterator (MiniMC::CPA::State_ptr& st, MiniMC::CPA::proc_id proc, MiniMC::CPA::proc_id  lproc, MiniMC::Model::Location::edge_iterator beg, MiniMC::Model::Location::edge_iterator end) :
		  curState(st),
		  proc(proc),
		  last_proc(lproc),
		  iter(beg),
		  end(end) {
		  update();
		}
		
		void update () {
		  bool done  = false;
		  while (!done) {
			while (iter == end && proc < last_proc) {
			  proc++;
			  if (proc >= last_proc)
				break;
			  assert(proc <= last_proc);
			  auto loc = curState->getLocation (proc);//
			  iter = loc->ebegin();
			  end  = loc->eend ();
			}
			
			if (proc < last_proc) {
			  succ.proc = proc;
			  succ.edge = (*iter);
			  succ.eflags = 0;
			  try {
				succ.state = Transfer::doTransfer (curState,succ.edge,succ.proc);
			  }
			  catch (MiniMC::Support::AssertViolated& e) {
				succ.state = nullptr;
				succ.eflags = static_cast<MiniMC::uint8_t> (ErrorFlags::AssertViolated); 
			  }
	      
			  if (succ.state || succ.eflags) {
				done = true;
			  }
			  else {
				++iter;
			  }
			}
			else {
			  done  = true;
			}
		  }
	  
		}
	
		MiniMC::CPA::State_ptr curState; 
		MiniMC::CPA::proc_id proc;
		MiniMC::CPA::proc_id last_proc;
		MiniMC::Model::Location::edge_iterator iter;
		MiniMC::Model::Location::edge_iterator end;
		Successor succ;
      };
    public:
      using iterator = Iterator;
      Generator (MiniMC::CPA::State_ptr& state) :state(state) {
      }
	  
      auto begin() {
		if (state->nbOfProcesses () == 0) {
		  throw MiniMC::Support::Exception ("No Processes to generate sucessors for");
		}
		return Iterator::makeBegin(state);}

	  auto end() {return Iterator::makeEnd(state);}
	  
    private:
      MiniMC::CPA::State_ptr state;
    

    };
  
      
  

  }
}

#endif
