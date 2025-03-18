#ifndef _SUCCESSOR_GEN__
#define _SUCCESSOR_GEN__

#include "minimc/cpa/interface.hpp"
#include "minimc/cpa/state.hpp"
#include "minimc/support/exceptions.hpp"
#include "minimc/model/types.hpp"

#include <generator>
#include <iostream>

namespace MiniMC {
  namespace CPA {

    inline std::generator<MiniMC::CPA::Transition> transitions (const MiniMC::CPA::AnalysisState& state) {
      MiniMC::Model::proc_t procs = state.getLocationState().nbOfProcesses ();
      for (MiniMC::Model::proc_t proc = 0; proc < procs; ++proc) {
	if (state.getLocationState().isActive (proc)) {
	  MiniMC::Model::Location::edge_iterator iter = state.getLocationState().getLocation(proc).ebegin() ;
	  MiniMC::Model::Location::edge_iterator end = state.getLocationState().getLocation(proc).eend();
	  for (;iter != end; ++iter) 
	    co_yield MiniMC::CPA::Transition {*iter,proc};
	}
      
      }

    }

     inline std::generator<MiniMC::CPA::Transition> transitions (const MiniMC::CPA::State& state) {
       MiniMC::Model::proc_t procs = state.getLocationState().nbOfProcesses ();
      for (MiniMC::Model::proc_t proc = 0; proc < procs; ++proc) {
	if (state.getLocationState().isActive (proc)) {
	  MiniMC::Model::Location::edge_iterator iter = state.getLocationState().getLocation(proc).ebegin() ;
	  MiniMC::Model::Location::edge_iterator end = state.getLocationState().getLocation(proc).eend();
	  for (;iter != end; ++iter) 
	    co_yield MiniMC::CPA::Transition {*iter,proc};
	}
      
      }

    }

    inline std::generator<MiniMC::CPA::AnalysisState> successors (const MiniMC::CPA::AnalysisState& state, MiniMC::CPA::AnalysisTransfer& transfer) {
      MiniMC::CPA::AnalysisState newstate;
      for (auto transition : transitions (state)) {
	if (transfer.Transfer (state,transition,newstate))
	  co_yield newstate;
      }
    }

    inline std::generator<MiniMC::CPA::State_ptr> successors (const MiniMC::CPA::State& state, MiniMC::CPA::Transfer& transfer) {
      for (auto transition : transitions (state)) {
	for (auto newstate :  transfer.doTransfer (state,transition))
	  co_yield newstate;
      }
    }

    template<class Ostream>
    inline Ostream& operator<< (Ostream& str, const Transition& t) {
      str << "@" << t.proc << " " << *t.edge;
      return str;
    }
	    
    
        
  } // namespace Algorithms
} // namespace MiniMC

#endif
