#ifndef _PRINTGRAPH__
#define _PRINTGGRAPH__

#include <sstream>
#include "support/feedback.hpp"
#include "support/exceptions.hpp"
#include "support/graph.hpp"
#include "support/localisation.hpp"
#include "algorithms/algorithm.hpp"
#include "algorithms/passedwaiting.hpp"
#include "algorithms/successorgen.hpp"


namespace MiniMC {
  namespace Algorithms {
    template<class CPA>
    class PrintCPA : public MiniMC::Algorithms::Algorithm {
    public:
      PrintCPA (MiniMC::Support::Messager& m) : MiniMC::Algorithms::Algorithm (m)  {}
      virtual Result run (const MiniMC::Model::Program& prgm) {
		auto& messager = getMessager ();
		messager.message ("Initiating PrintCPA");
		MiniMC::Support::Graph_ptr graph = MiniMC::Support::CreateGraph<MiniMC::Support::GraphType::DOT> ("CPA");
		auto error = graph->getNode ("Error");
		CPADFSPassedWaitingAll<CPA> passed;
		auto initstate = CPA::Query::makeInitialState (prgm);
		MiniMC::Support::Localiser waitmess ("Waiting: %1%, Passed: %2%");
		try {
		  passed.insert(initstate);
		  auto progresser = messager.makeProgresser ();
		  while (passed.hasWaiting()) {
			progresser->progressMessage (waitmess.format(passed.getWSize(),passed.getPSize()));
			auto cur = passed.pull ();
			std::stringstream str;
			str << cur->hash ();
			auto curnode = graph->getNode (str.str());
			if (cur->isPotentialLoop ())
			  curnode->color ();
			MiniMC::Algorithms::Generator<typename CPA::Query,typename CPA::Transfer> generator (cur);
			auto it = generator.begin();
			auto end = generator.end();
			for (;it != end; ++it) {
			  std::stringstream edgestr;
			  edgestr<< it->proc <<":" << *it->edge;
			  if (it->hasErrors ()) {
				curnode->connect (*error,edgestr.str());
			  }
			  else {
				auto nstate = it->state;
				std::stringstream nstr;
				nstr << nstate->hash ();
				auto ncurnode = graph->getNode (nstr.str());
				std::stringstream labelstr;
				labelstr<< *nstate;
				ncurnode->setLabel (labelstr.str());
		
				curnode->connect (*ncurnode,edgestr.str());
		
				passed.insert(nstate);
			  }
			}
		  }
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
		CPA::PreValidate::setup (seq,mess);
	  }
	  
    };
    
   
    
  }
}

#endif
