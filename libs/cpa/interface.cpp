#include <ostream>
#include "model/cfg.hpp"
#include "hash/hashing.hpp"
#include "cpa/interface.hpp"


namespace MiniMC {
  namespace CPA {
	   std::ostream& operator<< (std::ostream& os, const State& state) {
		 proc_id procs = state.nbOfProcesses ();
		 auto concretizer = state.getConcretizer ();
		 for (proc_id i = 0; i < procs; ++i) {
		   os << "Process." << i << "@" << state.getLocation(i)->getInfo() .getName()<<  "\n";
		   auto stackDescr = state.getLocation(i)->getCFG()->getFunction ()->getVariableStackDescr ();
		   for (auto v : stackDescr->getVariables ()) {
			 os << v->getName () << " : ";
			 concretizer->evaluate_str (i,v,os);
			 os << "\n";
		   }
		 }
		 return os;
	   }
  }
}


