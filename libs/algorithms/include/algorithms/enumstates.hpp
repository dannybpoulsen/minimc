#ifndef _PRINTGRAPH__
#define _PRINTGGRAPH__

#include <sstream>
#include "support/feedback.hpp"
#include "support/exceptions.hpp"
#include "support/localisation.hpp"
#include "algorithms/algorithm.hpp"
#include "algorithms/passedwaiting.hpp"
#include "algorithms/successorgen.hpp"
#include "algorithms/reachability.hpp"



namespace MiniMC {
  namespace Algorithms {
    template<class CPA>
    class EnumStates : public MiniMC::Algorithms::Algorithm {
    public:
	  struct Options {
		gsl::not_null<MiniMC::Support::Messager*> messager;
	  };
	  
      EnumStates (const Options& opt) : messager(*opt.messager)  {}
      virtual Result run (const MiniMC::Model::Program& prgm) {
		if (!CPA::PreValidate::validate (prgm,messager)) {
		  return Result::Error;
		}
		messager.message ("Initiating EnumStates");
		std::size_t states = 0;
		
		PWOptions opt;
		CPADFSPassedWaiting<CPA> passed (opt);
		auto progresser = messager.makeProgresser ();
		auto predicate = [] (auto& b) {return false;};
		auto initstate = CPA::Query::makeInitialState (prgm);
		PassedInsert inserter (*progresser,passed);
		try {
		  reachabilitySearch<CPA> (passed,inserter,initstate,predicate);
		}
		catch(MiniMC::Support::VerificationException& exc) {
		  messager.error (exc.what());
		}
		messager.message ("Finished EnumStates");
		messager.message (MiniMC::Support::Localiser ("Total Number of States %1%").format(passed.getPSize())); 
		return Result::Success;
      }

	  static void presetups (MiniMC::Support::Sequencer<MiniMC::Model::Program>& seq,  MiniMC::Support::Messager& mess) {
		CPA::PreValidate::validate (seq,mess);
	  }
	private:
	  MiniMC::Support::Messager& messager;
	  
    };
  }
}

#endif
