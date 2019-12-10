#ifndef _PRINTGRAPH__
#define _PRINTGGRAPH__

#include <sstream>
#include "support/feedback.hpp"
#include "support/exceptions.hpp"
#include "support/localisation.hpp"
#include "algorithms/algorithm.hpp"
#include "algorithms/passedwaiting.hpp"
#include "algorithms/successorgen.hpp"


namespace MiniMC {
  namespace Algorithms {
    template<class CPA>
    class EnumStates : public MiniMC::Algorithms::Algorithm {
    public:
      EnumStates (MiniMC::Support::Messager& m) : MiniMC::Algorithms::Algorithm (m)  {}
      virtual Result run (const MiniMC::Model::Program& prgm) {
	auto& messager = getMessager ();
	messager.message ("Initiating EnumStates");
	std::size_t states = 0;
	  
	CPADFSPassedWaiting<CPA> passed;
	auto initstate = CPA::Query::makeInitialState (prgm);
	try {
	  passed.insert(initstate);
	  auto progresser = messager.makeProgresser ();
	  while (passed.hasWaiting()) {
	    states++;
	    auto cur = passed.pull ();
	    MiniMC::Algorithms::Generator<typename CPA::Query,typename CPA::Transfer> generator (cur);
	    auto it = generator.begin();
	    auto end = generator.end();
	    for (;it != end; ++it) {
	      if (!it->hasErrors ()) {
		auto nstate = it->state;
		assert(nstate);
		passed.insert(nstate);
	      }
	    }
	  }
	}
	catch(MiniMC::Support::VerificationException& exc) {
	  messager.error (exc.what());
	}
	messager.message ("Finished EnumStates");
	messager.message ((MiniMC::Support::Localiser ("Total Number of States %1%") % states).str()); 
	return Result::Success;
      }
    };
  }
}

#endif
