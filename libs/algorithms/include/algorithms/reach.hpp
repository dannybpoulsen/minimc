#ifndef _PRINTGRAPH__
#define _PRINTGGRAPH__

#include <sstream>
#include "support/feedback.hpp"
#include "support/exceptions.hpp"
#include "support/localisation.hpp"
#include "cpa/location.hpp"
#include "cpa/concrete.hpp"
#include "cpa/compound.hpp"
#include "algorithms/algorithm.hpp"
#include "algorithms/passedwaiting.hpp"
#include "algorithms/successorgen.hpp"
#include "algorithms/reachability.hpp"

namespace MiniMC {
  namespace Algorithms {
	class ExplicitReachability : public MiniMC::Algorithms::Algorithm {
    public:
	  enum class ReachabilityResult {
		AssertViolated,
		NoViolation,
		Inconclusive
	  };
	  struct AnalysisResult {
		ReachabilityResult result;
	  };
	  
	  struct Options {
		
	  };
      ExplicitReachability (const Options& opt) : messager(MiniMC::Support::getMessager ())  {}
      using CPA = MiniMC::CPA::Compounds::CPADef<0,
												 MiniMC::CPA::Location::CPADef,
												 MiniMC::CPA::Concrete::CPADef
												 >;
	  
      virtual Result run (const MiniMC::Model::Program& prgm) {
		if (!CPA::PreValidate::validate (prgm,messager)) {
		  return Result::Error;
		}

		messager.message ("Initiating Reachability");
		PWOptions opt;
		CPADFSPassedWaiting<CPA> passed (opt);
		auto initstate = CPA::Query::makeInitialState (prgm);
		MiniMC::CPA::State_ptr foundState = nullptr;
		try {
		  auto progresser = messager.makeProgresser ();
		  auto predicate = [](const MiniMC::Algorithms::Successor& succ) {
			auto nstate = succ.state;
			auto loc = nstate->getLocation (succ.proc);
			if (loc->getInfo().template is<MiniMC::Model::Attributes::AssertViolated> ()) {
			  return true;
			}
			return false;
		  };
		  
		  
		  MiniMC::Algorithms::PassedInsert insert (*progresser,passed);
		  foundState = MiniMC::Algorithms::reachabilitySearch<CPA> (passed,insert,initstate,predicate);
		  
		  
		}
		catch(MiniMC::Support::VerificationException& exc) {
		  messager.error (exc.what());
		}
		messager.message ("Finished Reachability");
		if (foundState) {
		  result.result = ReachabilityResult::AssertViolated;
		  messager.message ("AssertViolated");
		  return Result::Success;
		}
		else {
		  result.result = ReachabilityResult::NoViolation;
		  messager.message ("No Problem found");
		  return Result::Success;
		}
      }

	  const auto& getAnalysisResult () const {return result;}
	  
	private:
	  MiniMC::Support::Messager& messager;
	  AnalysisResult result {.result = ReachabilityResult::Inconclusive};
    };
  }
}

#endif
