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
#include "algorithms/reachability.hpp"

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
		auto progresser = messager.makeProgresser ();
		CPADFSPassedWaiting<CPA> passed;
		auto initstate = CPA::Query::makeInitialState (prgm);
		MiniMC::CPA::State_ptr foundState = nullptr;
		try {
		  auto predicate = [](const MiniMC::Algorithms::Successor& succ) {
							 auto nstate = succ.state;
							 auto loc = CPA::Query::getLocation (nstate,succ.proc);
							 if (loc->template is<MiniMC::Model::Location::Attributes::AssertViolated> ()) {
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
