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
    class PrintCPA : public MiniMC::Algorithms::Algorithm {
    public:
      struct Options {
	bool filterSatis = false;
	bool delayTillConverge = true;
	MiniMC::CPA::CPA_ptr cpa;
      };

      struct AnalysisResult {
	MiniMC::Support::Graph_ptr graph = nullptr;
      };
	  
      PrintCPA (const Options& opt) : messager(MiniMC::Support::getMessager ())  {
	if (opt.filterSatis)
	  pwopt.filter = [](const MiniMC::CPA::State_ptr& s) {return s->getConcretizer()->isFeasible () == MiniMC::CPA::Concretizer::Feasibility::Feasible;};
	if (!opt.delayTillConverge)
	  pwopt.delay = [](const MiniMC::CPA::State_ptr& s) {return false;};
	cpa = std::make_shared<MiniMC::CPA::ARG::CPA> (opt.cpa);
	pwopt.storer = cpa->makeStore ();
	pwopt.joiner = cpa->makeJoin ();
	
      }
      virtual Result run (const MiniMC::Model::Program& prgm) {
	if (!cpa->makeValidate()->validate (prgm,messager)) {
	  return Result::Error;
	}
	messager.message ("Running PrintCPA");
	aresult.graph = MiniMC::Support::CreateGraph<MiniMC::Support::GraphType::DOT> ("CPA");
		
		
	DFSWaiting passed (pwopt);
		
	try {
	  {
	    auto progresser = messager.makeProgresser ();
	    auto predicate = [] (auto& b) {return false;};
	    auto query = cpa->makeQuery ();
	    auto transfer = cpa->makeTransfer ();
	    auto initstate = query->makeInitialState (prgm);
	
	    PassedInsert inserter (*progresser,passed);
	    reachabilitySearch (passed,inserter,initstate,predicate,query,transfer);
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
      
  
      const AnalysisResult& getAnalysisResult () const {return aresult;}
	  
    private:
      MiniMC::Support::Messager& messager;
      PWOptions pwopt;
      AnalysisResult aresult;
      MiniMC::CPA::CPA_ptr cpa;
      
    };
    
   
    
  }
}

#endif
