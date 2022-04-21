#include "algorithms/reachability/reachability.hpp"
#include "cpa/interface.hpp"
#include "algorithms/algorithms.hpp"
#include "algorithms/successorgen.hpp"

#include <algorithm>

namespace MiniMC {
  namespace Algorithms {
    namespace Reachability {
      Verdict Reachability::search (const MiniMC::CPA::State_ptr& state, GoalFunction goal) {
	MiniMC::CPA::Storer storage {joiner};
	std::list<MiniMC::CPA::State_ptr> waiting;
	MiniMC::Algorithms::Generator generator{transfer};

	auto insert = [&storage,&waiting](auto& state) {
	  if (!storage.isCoveredByStore (state)) {
	    auto joinPair = storage.joinState (state);
	    waiting.push_front (joinPair.joined);
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
	}

	return Verdict::NotFound;
	
      }
	
      
    }
  }
}
