#ifndef _PRINTGRAPH__
#define _PRINTGGRAPH__

#include <sstream>
#include "support/feedback.hpp"
#include "support/exceptions.hpp"
#include "support/localisation.hpp"
#include "cpa/interface.hpp"
#include "algorithms/algorithm.hpp"
#include "algorithms/passedwaiting.hpp"
#include "algorithms/successorgen.hpp"
#include "algorithms/reachability.hpp"



namespace MiniMC {
  namespace Algorithms {
    class EnumStates : public MiniMC::Algorithms::Algorithm {
    public:
      struct Options {
	MiniMC::CPA::CPA_ptr cpa;
      };
	  
      EnumStates (const Options& opt) : messager(MiniMC::Support::getMessager ()),cpa(opt.cpa)  {}
      virtual Result run (const MiniMC::Model::Program& prgm) {
	if (!cpa->makeValidate ()->validate (prgm,messager)) {
	  return Result::Error;
	}
	messager.message ("Initiating EnumStates");
	std::size_t states = 0;
	
	PWOptions opt;
	opt.storer = cpa->makeStore();
	opt.joiner = cpa->makeJoin ();
	DFSWaiting passed (opt);
	auto progresser = messager.makeProgresser ();
	auto predicate = [] (auto& b) {return false;};
	auto query = cpa->makeQuery ();
	auto transfer = cpa->makeTransfer ();
	auto initstate = query->makeInitialState (prgm);
	PassedInsert inserter (*progresser,passed);
	try {
	  reachabilitySearch (passed,inserter,initstate,predicate,query,transfer);
	}
	catch(MiniMC::Support::VerificationException& exc) {
	  messager.error (exc.what());
	}
	messager.message ("Finished EnumStates");
	messager.message (MiniMC::Support::Localiser ("Total Number of States %1%").format(passed.getPSize())); 
	return Result::Success;
      }
      
    private:
      MiniMC::Support::Messager& messager;
      MiniMC::CPA::CPA_ptr cpa;
    };
  }
}

#endif
