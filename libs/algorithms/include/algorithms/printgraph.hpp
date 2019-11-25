#ifndef _PRINTGRAPH__
#define _PRINTGGRAPH__

#include <sstream>
#include "support/feedback.hpp"
#include "support/exceptions.hpp"
#include "support/graph.hpp"
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
	CPADFSPassedWaiting<CPA> passed;
	auto initstate = CPA::Query::makeInitialState (prgm);
	
	passed.insert(initstate);
	while (passed.hasWaiting()) {
	  auto cur = passed.pull ();
	  std::stringstream str;
	  str << cur->hash ();
	  auto curnode = graph->getNode (str.str());
	  MiniMC::Algorithms::Generator<typename CPA::Query,typename CPA::Transfer> generator (cur);
	  auto it = generator.begin();
	  auto end = generator.end();
	  for (;it != end; ++it) {
	    auto nstate = it->state;
	    std::stringstream nstr;
	    nstr << nstate->hash ();
	    auto ncurnode = graph->getNode (nstr.str());
	    std::stringstream labelstr;
	    labelstr<< *nstate;
	    ncurnode->setLabel (labelstr.str());
	    std::stringstream edgestr;
	    edgestr<< it->proc <<":" << *it->edge;
	    
	    curnode->connect (*ncurnode,edgestr.str());
	    
	    passed.insert(nstate);
	  }
	}

	graph->write ("CPA");
	
	messager.message ("Finished PrintCPA");
	return Result::Success;
      }
    };
  }
}

#endif
