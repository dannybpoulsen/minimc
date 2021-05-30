#ifndef _PRINTGRAPH__
#define _PRINTGGRAPH__

#include <set>
#include <gsl/pointers>
#include "support/feedback.hpp"
#include "support/exceptions.hpp"
#include "support/graph.hpp"
#include "support/localisation.hpp"
#include "algorithms/algorithm.hpp"
#include "algorithms/passedwaiting.hpp"
#include "algorithms/successorgen.hpp"
#include "algorithms/reachability.hpp"
#include "cpa/arg.hpp"


namespace MiniMC {
  namespace Algorithms {
    template<class ACPA>
    class PrintCPA : public MiniMC::Algorithms::Algorithm {
    public:
      struct Options {
		bool filterSatis = false;
		bool delayTillConverge = true;
	  };

	  struct AnalysisResult {
		MiniMC::Support::Graph_ptr graph = nullptr;
	  };
	  
      using CPA = MiniMC::CPA::ARG::CPADef<ACPA>;
      PrintCPA (const Options& opt) : messager(MiniMC::Support::getMessager ())  {
		if (opt.filterSatis)
		  pwopt.filter = [](const MiniMC::CPA::State_ptr& s) {return s->getConcretizer()->isFeasible () == MiniMC::CPA::Concretizer::Feasibility::Feasible;};
		if (!opt.delayTillConverge)
		  pwopt.delay = [](const MiniMC::CPA::State_ptr& s) {return false;};
	  }
      virtual Result run (const MiniMC::Model::Program& prgm) {
		if (!CPA::PreValidate::validate (prgm,messager)) {
		  return Result::Error;
		}
		messager.message ("Running PrintCPA");
		aresult.graph = MiniMC::Support::CreateGraph<MiniMC::Support::GraphType::DOT> ("CPA");
		
		
		CPADFSPassedWaiting<CPA> passed (pwopt);
		
		try {
		  {
			auto progresser = messager.makeProgresser ();
			auto predicate = [] (auto& b) {return false;};
			auto initstate = CPA::Query::makeInitialState (prgm);
			PassedInsert inserter (*progresser,passed);
			reachabilitySearch<CPA> (passed,inserter,initstate,predicate);
		  }
		  //State space is now generated - create the graph
		  auto it = passed.stored_begin();
		  auto end = passed.stored_end();
		  MiniMC::CPA::ARG::generateARGGraph (aresult.graph,it,end);
		  
		}
		
		catch(MiniMC::Support::VerificationException& exc) {
		  messager.error (exc.what());
		}
		messager.message ("Finished PrintCPA");
		return Result::Success;
      }
      
      static void presetups (MiniMC::Support::Sequencer<MiniMC::Model::Program>& seq,  MiniMC::Support::Messager& mess) {
		CPA::PreValidate::validate (seq,mess);
      }
	  
	  const AnalysisResult& getAnalysisResult () const {return aresult;}
	  
    private:
      MiniMC::Support::Messager& messager;
	  PWOptions pwopt;
	  AnalysisResult aresult;
	};
    
   
    
  }
}

#endif
