#ifndef _PRINTGRAPH__
#define _PRINTGGRAPH__

#include <sstream>
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
		auto insert = [&](auto& state) -> std::unique_ptr<MiniMC::Support::Node> {
																				  std::stringstream str;
																				  str << std::hash<MiniMC::CPA::State>{} (*state);
																				  auto node = graph->getNode (str.str());
																				  std::stringstream labelstr;
																				  labelstr<< *state;
																				  node->setLabel (labelstr.str());
																				  return node;
		};
		try {
		  auto progresser = messager.makeProgresser ();
		  auto predicate = [] (auto& b) {return false;};
		  auto initstate = CPA::Query::makeInitialState (prgm);
		  PassedInsert inserter (*progresser,passed);
		  reachabilitySearch<CPA> (passed,inserter,initstate,predicate);
		  
		  //State space is now generated - create the graph
		  auto it = passed.stored_begin();
		  auto end = passed.stored_end();
		  std::set<MiniMC::CPA::State_ptr> visited;
		  MiniMC::Algorithms::Stack working;
		  auto addState = [&] (std::weak_ptr<MiniMC::CPA::State> winp) {
							auto inp = winp.lock();
							if (inp) {
							  if (visited.count(inp) == 0) {
								working.insert(inp);
								visited.insert(inp);
							  }
							}
						  };
		  std::for_each (it,end,addState);
		  while (!working.empty ()) {
			auto current = working.pull ();
			auto curnode = insert ( current );

			auto argcur = std::static_pointer_cast<MiniMC::CPA::ARG::State> (current);
			for (auto& wpar : *argcur) {
			  auto  state = wpar.from.lock ();
			  if (state) {
				auto pnode = insert(state);
				std::stringstream edgetext;
				edgetext << wpar.who << ": " << *wpar.edge;
				pnode->connect(*curnode,edgetext.str());
				addState (state);
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
      }


	  
    private:
      MiniMC::Support::Messager& messager;
    };
    
   
    
  }
}

#endif
