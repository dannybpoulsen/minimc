#ifndef _SUCCESSOR_GEN__
#define _SUCCESSOR_GEN__

#include <gsl/pointers>
#include "cpa/interface.hpp"
#include "support/exceptions.hpp"

namespace MiniMC {
  namespace Algorithms {
    struct Successor {
      MiniMC::CPA::State_ptr state;
      MiniMC::Model::Edge_ptr edge;
      MiniMC::CPA::proc_id proc;
      };
    
    template<class StateQuery,class Transfer>
    class Generator {
      class Iterator {
      public:

	static Iterator makeBegin (MiniMC::CPA::State_ptr& pt) {
	  auto loc = StateQuery::getLocation (pt,0);
	  return Iterator(pt,0,StateQuery::nbOfProcesses (pt),loc->ebegin(),loc->eend());
	}

	static Iterator makeEnd (MiniMC::CPA::State_ptr& pt) {
	  auto proc = StateQuery::nbOfProcesses (pt);
	  auto loc = StateQuery::getLocation (pt,proc-1);
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
	      auto loc = StateQuery::getLocation (curState,proc);
	      iter = loc->ebegin();
	    end  = loc->eend ();
	    }
	    
	    if (proc < last_proc) {
	      succ.proc = proc;
	      succ.edge = iter->get();
	      succ.state = Transfer::doTransfer (curState,succ.edge,succ.proc);
	      if (succ.state) {
		done = true;
	      }
	      else {
		iter++;
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
	if (StateQuery::nbOfProcesses (state) == 0) {
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
