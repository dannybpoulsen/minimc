#include "algorithms/reachability/reachability.hpp"
#include "cpa/interface.hpp"
#include "algorithms/algorithms.hpp"
#include "algorithms/successorgen.hpp"
#include "storage/storage.hpp"
#include <algorithm>

namespace MiniMC {
  namespace Algorithms {
    namespace Reachability {

      
      StateStatus DefaultFilter (const MiniMC::CPA::State_ptr& state) {
	auto solver = state->getConcretizer ();
	switch (solver->isFeasible ()) {
	case MiniMC::CPA::Solver::Feasibility::Feasible:
	case MiniMC::CPA::Solver::Feasibility::Unknown:
	  return StateStatus::Keep;
	default:
	  return StateStatus::Discard;
	}
      }
      

      Verdict Reachability::search (const MiniMC::CPA::State_ptr& state, GoalFunction goal,FilterFunction filter) {
	std::list<MiniMC::CPA::State_ptr> waiting;
	MiniMC::Algorithms::Generator generator{transfer};
	MiniMC::Storage::HashStorage storage;
	
        auto insert = [&storage,&waiting,filter](auto& state) {  
	  if (filter(state) == StateStatus::Keep) {
	    auto ins = storage.insert (state);
	    if (ins) {
	      waiting.push_back (std::move(ins));
	    }
	  }
	};

	insert(state);
	while (waiting.size ()) {
	  auto searchee = std::move(waiting.back ());
	  waiting.pop_back ();

	  if (goal(searchee)) {
	    return Verdict::Found;
	  }
	  auto successors = generator.generate (searchee);
	  auto end = successors.second;
	  std::for_each (successors.first,end,[insert](auto& succ){insert(succ.state);});
	  progress_indicator = Progress{.passed = storage.size (), .waiting = waiting.size ()};
	  progress_indicator.notif_listeners ();
	}

	return Verdict::NotFound;
	
      }
      
    }
  }
}


