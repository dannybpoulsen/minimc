#ifndef _SUCCESSOR_GEN__
#define _SUCCESSOR_GEN__

#include "minimc/cpa/interface.hpp"
#include "minimc/cpa/state.hpp"
#include "minimc/model/source.hpp"
#include "minimc/support/exceptions.hpp"
#include "minimc/model/types.hpp"

#include <generator>
#include <iostream>
#include <ranges>

namespace MiniMC {
  namespace CPA {

    inline std::generator<MiniMC::CPA::Transition> transitions (const MiniMC::CPA::State& state,MiniMC::Model::proc_t id) {
      if (state.getLocationState().isActive (id)) {
	MiniMC::Model::Location::edge_iterator iter = state.getLocationState().getLocation(id).ebegin() ;
	MiniMC::Model::Location::edge_iterator end = state.getLocationState().getLocation(id).eend();
	for (;iter != end; ++iter) 
	  co_yield MiniMC::CPA::Transition {*iter,id};
      }
	

    }
    
     inline std::generator<MiniMC::CPA::Transition> transitions (const MiniMC::CPA::State& state) {
       MiniMC::Model::proc_t procs = state.getLocationState().nbOfProcesses ();
       for (MiniMC::Model::proc_t proc = 0; proc < procs; ++proc) {
	 co_yield std::ranges::elements_of (transitions(state,proc));
       }

     }

    inline std::generator<MiniMC::CPA::State_ptr> comm_successors (const MiniMC::CPA::State& state, MiniMC::CPA::Transfer& transfer,MiniMC::Model::proc_t id) {
      std::vector<MiniMC::CPA::State_ptr> states;
      states.push_back(state.copy());
      while(states.size()) {
	auto stat = states.back();
	states.pop_back();
	for (auto transition : transitions (*stat,id)) {
	  for (auto newstate :  transfer.doTransfer (*stat,transition)) {
	    if (!newstate->getLocationState().getLocation(id).getInfo().getFlags().isSet (MiniMC::Model::Attributes::Committed)) {
	      co_yield newstate;
	      
	    }
	    else
	      states.push_back(newstate);
	  }
	}
      }
    }
      
    
    inline std::generator<MiniMC::CPA::State_ptr> norm_successors (const MiniMC::CPA::State& state, MiniMC::CPA::Transfer& transfer,MiniMC::Model::proc_t id) {
      for (auto transition : transitions (state,id)) {
	for (auto newstate :  transfer.doTransfer (state,transition)) { 
	  co_yield newstate;
	}
      }
    }

    inline std::generator<MiniMC::CPA::State_ptr> successors (const MiniMC::CPA::State& state, MiniMC::CPA::Transfer& transfer) {
      MiniMC::Model::proc_t procs = state.getLocationState().nbOfProcesses ();
      for (MiniMC::Model::proc_t proc = 0; proc < procs; ++proc) {
	if (!state.getLocationState().getLocation(proc).getInfo().getFlags().isSet (MiniMC::Model::Attributes::Committed))
	  co_yield std::ranges::elements_of (norm_successors(state,transfer,proc));
	else
	  co_yield std::ranges::elements_of (comm_successors(state,transfer,proc));
	
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
