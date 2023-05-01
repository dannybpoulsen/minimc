#ifndef _SUCCESSOR_GEN__
#define _SUCCESSOR_GEN__

#include "cpa/interface.hpp"
#include "support/exceptions.hpp"
#include "host/types.hpp"

namespace MiniMC {
  namespace Algorithms {
    struct EnumResult {
      EnumResult () : edge(nullptr),proc(0) {}
      EnumResult (MiniMC::Model::Edge* edge,MiniMC::proc_t p) : edge(edge),proc(p) {}
      
      MiniMC::Model::Edge* edge;
      MiniMC::proc_t proc; 
    };


    
    
    class EdgeEnumerator {
    public:
      EdgeEnumerator (const MiniMC::CPA::AnalysisState& state) : orig(state),
								 iter(orig.getCFAState ().getLocationState().getLocation(0).ebegin ()),
								 end(orig.getCFAState ().getLocationState().getLocation(0).eend ())
	
      {
	done = !next ();
      }
      

      auto operator* () {
	return EnumResult {*iter,proc};
      }

      auto operator++ () {
	done = !next();
      }
      
      explicit operator bool () const {return !done;}
      
    private:

      bool next () {
	if (init ) {
	  iter = orig.getCFAState ().getLocationState().getLocation(proc).ebegin ();
	  end = orig.getCFAState ().getLocationState().getLocation(proc).eend ();
	  init = false;
	  return iter!=end;
	}
	
	else {
	  ++iter;
	  if (iter == end) {
	    proc++;
	    if (proc >=orig.getCFAState ().getLocationState().nbOfProcesses ())
	      return false;
	    iter = orig.getCFAState ().getLocationState().getLocation(proc).ebegin ();
	    end = orig.getCFAState ().getLocationState().getLocation(proc).eend ();
	    return iter != end;
	  }
	  return true;
	  
	}
	  
      }

      const MiniMC::CPA::AnalysisState& orig;
      proc_t proc{0};
      bool init{true};
      MiniMC::Model::Location::edge_iterator iter;
      MiniMC::Model::Location::edge_iterator end;
      bool done{false};
    };
    
  } // namespace Algorithms
} // namespace MiniMC

#endif
