#ifndef _SUCCESSOR_GEN__
#define _SUCCESSOR_GEN__

#include "cpa/interface.hpp"
#include "cpa/state.hpp"
#include "support/exceptions.hpp"
#include "host/types.hpp"

namespace MiniMC {
  namespace CPA {
    /*struct EnumResult {
      EnumResult () : edge(nullptr),proc(0) {}
      EnumResult (MiniMC::Model::Edge* edge,MiniMC::proc_t p) : edge(edge),proc(p) {}
      
      MiniMC::Model::Edge* edge;
      MiniMC::proc_t proc; 
    };*/


    
    
    class TransitionEnumerator {
    public:
      TransitionEnumerator (const MiniMC::CPA::AnalysisState& state) : orig(state)	
      {
	done = !next ();
      }
      

      auto operator* () {
	return MiniMC::CPA::Transition {*iter,proc};
      }
      
      auto operator++ () {
	done = !next();
      }
      
      explicit operator bool () const {return !done;}
      
    private:

      bool next () {
	if (init ) {
	  while(!orig.getCFAState().isActive (proc)){
	    proc++;
	    if (proc >=orig.getCFAState ().getLocationState().nbOfProcesses ())
	      return false;
	  }
	  iter = orig.getCFAState ().getLocationState().getLocation(proc).ebegin ();
	  end = orig.getCFAState ().getLocationState().getLocation(proc).eend ();
	  init = false;
	  return iter!=end;
	}
	
	else {
	  ++iter;
	  if (iter == end) {
	    do {
	      proc++;
	      
	      if (proc >=orig.getCFAState ().getLocationState().nbOfProcesses ())
		return false;
	    }while(!orig.getCFAState().isActive (proc));
	    iter = orig.getCFAState ().getLocationState().getLocation(proc).ebegin ();
	    end = orig.getCFAState ().getLocationState().getLocation(proc).eend ();
	    return iter != end;
	  }
	  return true;
	  
	}
	  
      }

      const MiniMC::CPA::AnalysisState& orig;
      MiniMC::Model::proc_t proc{0};
      bool init{true};
      MiniMC::Model::Location::edge_iterator iter;
      MiniMC::Model::Location::edge_iterator end;
      bool done{false};
    };

    class SuccessorEnumerator {
    public:
      SuccessorEnumerator (const MiniMC::CPA::AnalysisState& s,MiniMC::CPA::AnalysisTransfer& transfer) : ee(s),transfer(transfer),_enumFrom(s) {
	done = next ();
      }
      
      auto operator* () {
	return _next;
      }
      
      auto operator++ () {
	done = next();
      }

      explicit operator bool () const {return !done;}
      
      
      
    private:
      bool next () {
	while (ee) {
	  if (transfer.Transfer (_enumFrom,*ee,_next)) {
	    ++ee;
	    return false;
	  }
	  ++ee;
	}
	return true;
      }
      

      TransitionEnumerator ee;
      MiniMC::CPA::AnalysisState _next;
      MiniMC::CPA::AnalysisTransfer& transfer;
      MiniMC::CPA::AnalysisState _enumFrom;
      
      bool done{false};
    };
    
  } // namespace Algorithms
} // namespace MiniMC

#endif
