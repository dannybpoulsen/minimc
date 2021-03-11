#include "support/smt.hpp"
#include "smt/context.hpp"

namespace MiniMC {
  namespace Support {
	namespace SMT {
	  template<SMTLib::SMTBackend b>
	  class SMTFactoryL : public SMTFactory {
	  public:
		virtual SMTLib::Context_ptr construct () {
		  return SMTLib::makeContext<b> ();
		}
	  };

	  SMTFactory_ptr getSMTFactory () {
		return std::make_shared<SMTFactoryL<SMTLib::SMTBackend::CVC4> > ();
	  }
	  
	}
  }
}
