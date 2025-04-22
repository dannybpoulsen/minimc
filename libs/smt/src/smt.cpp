#include "minimc/smt/smt.hpp"
#include "smt/context.hpp"
#include "minimc/support/exceptions.hpp"
#include <vector>

namespace MiniMC {
  namespace Support {
    namespace SMT {

      SMTLib::Context_ptr SMTDescr::makeContext() const {
	if (r)
	  return r->getFunction()();
	else
	  throw MiniMC::Support::ConfigurationException ("No SMT Solver selected");
      }

      std::string SMTDescr::name () const { return ( r != nullptr) ? r->getName() : "None"; }
      std::string SMTDescr::descr () const { return (r != nullptr) ? r->getDescritpion() : "No SMT Solver selected"; }

            
      template <class Iterator>
      void getBackends(Iterator it) {
        for (auto& itt : SMTLib::getSMTBackends()) {
          it = itt;
        }
      }

      SMTSolverRepository::SMTSolverRepository () {
	getBackends (std::back_inserter(descr));
      }

      SMTSolverRepository& SMTSolverRepository::get () {
	static SMTSolverRepository rep;
	return rep;
      }
	
	 
      
    } // namespace SMT
  }   // namespace Support
} // namespace MiniMC
