#include "minimc/model/modifications/expandnondet.hpp"
#include "minimc/model/instructions.hpp"
#include "minimc/support/workinglist.hpp"
#include "minimc/support/feedback.hpp"
#include "minimc/support/localisation.hpp"
#include "minimc/support/overload.hpp"
#include "minimc/model/nondet_generator.hpp"


#include <limits>
#include <algorithm>

namespace MiniMC {
  namespace Model {
    namespace Modifications {
      

      
      void expandEdge (MiniMC::Model::CFA& cfa, const MiniMC::Model::Edge* edge) {
	if (edge->getInstructions ()) {
	  // Only bother if we have instructions to deal with
	  auto& instr = edge->getInstructions();
	  MiniMC::Model::InstructionStream nstr;
	  auto prev =  edge->getFrom ();
	  auto goal =  edge->getTo ();
	  
	  for (auto& i : instr) {
	    i.visit ( MiniMC::Support::Overload {
		[&edge, &cfa,&prev,&nstr](const MiniMC::Model::TInstruction<MiniMC::Model::VMInstructionCode::NonDet>& instr) {
		  auto nloc = cfa.makeLocation (prev->getSymbol (),prev->getInfo ());
		  auto nnondet = cfa.makeLocation (prev->getSymbol (),prev->getInfo ());
		  
		  cfa.makeEdge (prev,nloc,std::move(nstr),edge->isPhi ());
		  nstr.clear ();
		  auto assign = instr.getOps ().res;
		  
		  MiniMC::Model::NonDetGenerator gen;
		  for (auto t : gen.generate(*assign->getType())) {
		    nstr.add<VMInstructionCode::Assign> (assign,t);
		    cfa.makeEdge (nloc,nnondet,std::move(nstr));
		    nstr.clear ();
		  }
		  
		  prev = nnondet;
		  
		  
		},
		  [&i,&nstr](auto& ) {
		    nstr.add (i);
		  }
		  
		  }
	      );
	  }
	  cfa.makeEdge (prev,goal,std::move(nstr));
	  cfa.deleteEdge ( edge);
	}
      }
	
      void expandNonDetCFAEdges (MiniMC::Model::CFA& cfa) {
	MiniMC::Support::WorkingList<const MiniMC::Model::Edge*> wlist;
	std::for_each (cfa.getEdges().begin (),cfa.getEdges().end (),[&wlist](auto& e) {wlist.inserter () = e.get ();});
	std::for_each (wlist.begin(), wlist.end (),[&cfa](auto& e) {
	  expandEdge (cfa,e);
	}
	  );
      }

      MiniMC::Model::Program NonDetExpander::operator()  (MiniMC::Model::Program&& prgm) {
	messager << MiniMC::Support::TInfo {"Unfolding non-determinstic values"};
	for (auto& function : prgm.getFunctions ()) {
	  expandNonDetCFAEdges (function->getCFA ());
	}
	return prgm;
      }
      
    }
  }
}
  
