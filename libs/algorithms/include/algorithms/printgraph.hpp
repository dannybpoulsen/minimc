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
		gsl::not_null<MiniMC::Support::Messager*> messager;
      };
      using CPA = MiniMC::CPA::ARG::CPADef<ACPA>;
      PrintCPA (const Options& opt) : messager(*opt.messager.get())  {}
      virtual Result run (const MiniMC::Model::Program& prgm) {
		messager.message ("Initiating PrintCPA");
		MiniMC::Support::Graph_ptr graph = MiniMC::Support::CreateGraph<MiniMC::Support::GraphType::DOT> ("CPA");
		CPADFSPassedWaiting<CPA> passed;
		
		try {
		  auto progresser = messager.makeProgresser ();
		  auto predicate = [] (auto& b) {return false;};
		  auto initstate = CPA::Query::makeInitialState (prgm);
		  PassedInsert inserter (*progresser,passed);
		  reachabilitySearch<CPA> (passed,inserter,initstate,predicate);
		  
		  //State space is now generated - create the graph
		  auto it = passed.stored_begin();
		  auto end = passed.stored_end();
		  MiniMC::CPA::ARG::generateARGGraph (graph,it,end);

		}
	
		catch(MiniMC::Support::VerificationException& exc) {
		  messager.error (exc.what());
		}
		messager.message ("Finished PrintCPA");
		messager.message ("Writing Graph");
		graph->write ("CPA");
		messager.message ("Wrote Graph");
	
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
