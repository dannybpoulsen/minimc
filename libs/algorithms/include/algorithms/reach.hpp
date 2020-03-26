#ifndef _PRINTGRAPH__
#define _PRINTGGRAPH__

#include <sstream>
#include "support/feedback.hpp"
#include "support/exceptions.hpp"
#include "support/localisation.hpp"
#include "cpa/location.hpp"
#include "cpa/concrete_no_mem.hpp"
#include "cpa/compound.hpp"
#include "algorithms/algorithm.hpp"
#include "algorithms/passedwaiting.hpp"
#include "algorithms/successorgen.hpp"


namespace MiniMC {
  namespace Algorithms {
    class ExplicitReachability : public MiniMC::Algorithms::Algorithm {
    public:
	  struct Options {
		gsl::not_null<MiniMC::Support::Messager*> messager;
	  };
      ExplicitReachability (const Options& opt) : messager(*opt.messager)  {}
      using CPA = MiniMC::CPA::Compounds::CPADef<0,
						 MiniMC::CPA::Location::CPADef,
						 MiniMC::CPA::ConcreteNoMem::CPADef
												 >;
      
      virtual Result run (const MiniMC::Model::Program& prgm) {
		messager.message ("Initiating Reachability");
		std::size_t states = 0;
		
	CPADFSPassedWaiting<CPA> passed;
	auto initstate = CPA::Query::makeInitialState (prgm);
	MiniMC::Support::Localiser waitmess ("Waiting: %1%, Passed: %2%");
	MiniMC::CPA::State_ptr foundState = nullptr;
	try {
	  passed.insert(initstate);
	  auto progresser = messager.makeProgresser ();
	  while (passed.hasWaiting()) {
	    progresser->progressMessage (waitmess.format(passed.getWSize(),passed.getPSize()));
	    auto cur = passed.pull ();
	    MiniMC::Algorithms::Generator<typename CPA::Query,typename CPA::Transfer> generator (cur);
	    auto it = generator.begin();
	    auto end = generator.end();
	    for (;it != end; ++it) {
	      if (!it->hasErrors ()) {
		auto nstate = it->state;
		assert(nstate);
		passed.insert(nstate);
		auto loc = CPA::Query::getLocation (nstate,it->proc);
		if (loc->template is<MiniMC::Model::Location::Attributes::AssertViolated> ()) {
		  foundState = nstate;
		}
	      }
	    }
	  }
	}
	catch(MiniMC::Support::VerificationException& exc) {
	  messager.error (exc.what());
	}
	messager.message ("Finished Reachability");
	if (foundState) {
	  messager.message ("AssertViolated");
	  return Result::Success;
	}
	else {
	  messager.message ("No Problem found");
	  return Result::Success;
	}
      }
      
      static void presetups (MiniMC::Support::Sequencer<MiniMC::Model::Program>& seq,  MiniMC::Support::Messager& mess) {
		CPA::PreValidate::validate (seq,mess);
		CPA::PreValidate::setup (seq,mess);
      }

	private:
	  MiniMC::Support::Messager& messager;
	  
    };
  }
}

#endif
