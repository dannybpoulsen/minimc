#ifndef _REACHABILITY__
#define _REACHABILITY__

#include <functional>

#include "support/feedback.hpp"
#include "support/localisation.hpp"
#include "cpa/state.hpp"
#include "algorithms/successorgen.hpp"

namespace MiniMC {
  namespace Algorithms {

	struct DummyInsert {
	  void operator= (const MiniMC::CPA::State_ptr& ptr) {
	  }
	};

	template<class Passed>
	struct PassedInsert {
	  PassedInsert (MiniMC::Support::Progresser& p,Passed& pass) : waitmess("Waiting: %1%, Passed: %2%"), progresser(p),passed(pass) {}
	  void operator= (const MiniMC::CPA::State_ptr& ptr) {
		progresser.progressMessage (waitmess.format(passed.getWSize(),passed.getPSize()));
	  }
	  MiniMC::Support::Localiser waitmess; ;
	  MiniMC::Support::Progresser& progresser;
	  Passed& passed;
	};
	
	using Predicate = std::function<bool(const MiniMC::Algorithms::Successor&)>;
	template<class CPA, class Passed,class Inserter = DummyInsert>
	MiniMC::CPA::State_ptr reachabilitySearch (Passed& passed, Inserter& insert, const MiniMC::CPA::State_ptr&  searchFrom, Predicate predicate) {
	  passed.insert(searchFrom);
	  while (passed.hasWaiting()) {
		auto cur = passed.pull ();
		insert = cur;
		MiniMC::Algorithms::Generator<typename CPA::Query,typename CPA::Transfer> generator (cur);
		auto it = generator.begin();
		auto end = generator.end();
		for (;it != end; ++it) {
		  if (!it->hasErrors ()) {
			if (predicate (*it)){
			  return it->state;
			}
			passed.insert(it->state);
		  }
		}
	  }
	  return nullptr;
	}
  }
}
  

#endif 
