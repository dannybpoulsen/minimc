#include "smt/smt.hpp"
#include "smt/context.hpp"
#include "support/exceptions.hpp"
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

      std::string SMTDescr::name () { return ( r != nullptr) ? r->getName() : "None"; }
      std::string SMTDescr::descr () { return (r != nullptr) ? r->getDescritpion() : "No SMT Solver selected"; }
      
      
    } // namespace SMT
  }   // namespace Support
} // namespace MiniMC
