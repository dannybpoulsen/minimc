#include "algorithms/reachability/reachability.hpp"
#include "cpa/interface.hpp"
#include "algorithms/algorithms.hpp"
#include "algorithms/successorgen.hpp"
#include "storage/storage.hpp"
#include <algorithm>

namespace MiniMC {
  namespace Algorithms {
    namespace Reachability {
      StateStatus DefaultFilter (const MiniMC::CPA::AnalysisState& state) {
	for (auto& dstate : state.dataStates ()) {
	  auto solver = dstate.getConcretizer ();
	  switch (solver->isFeasible ()) {
	  case MiniMC::CPA::Solver::Feasibility::Feasible:
	  case MiniMC::CPA::Solver::Feasibility::Unknown:
	    break;
	  default:
	    return StateStatus::Discard;
	  }
	}

	return StateStatus::Keep;
      }
      

      Verdict Reachability::search (const MiniMC::CPA::AnalysisState& state, GoalFunction goal,FilterFunction filter) {
	std::list<MiniMC::CPA::AnalysisState> waiting;
	MiniMC::Storage::HashStorage storage;
	
        auto insert = [&storage,&waiting,filter](auto& state) {  
	  if (filter(state) == StateStatus::Keep) {
	    auto ins = storage.insert (state);
	    if (ins) {
	      waiting.push_back (state);
	    }
	  }
	};

	insert(state);
	while (waiting.size ()) {
	  auto searchee = std::move(waiting.back ());
	  waiting.pop_back ();

	  if (goal(searchee)) {
	    found = std::move(searchee);
	    return Verdict::Found;
	  }

	  if (searchee.getCFAState ().getLocationState().nbOfProcesses ()) {
	  
	    MiniMC::CPA::AnalysisState newstate;
	    MiniMC::Algorithms::EdgeEnumerator enumerator{searchee};
	    //MiniMC::Algorithms::EnumResult res;
	    for (; enumerator; ++enumerator) {
	      MiniMC::Algorithms::EnumResult res = *enumerator;
	      if (transfer.Transfer (searchee,*res.edge,res.proc,newstate)) {
		insert(newstate);
	      }
	    }
	  }
	  
	  
	  progress_indicator = Progress{.passed = storage.size (), .waiting = waiting.size ()};
	  progress_indicator.notif_listeners ();
	}
	
	return Verdict::NotFound;
	
      }
      
    }
  }
}


