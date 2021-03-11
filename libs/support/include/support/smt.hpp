#ifndef _SUPPORT_SMT__
#define _SUPPORT_SMT__

#include <memory>

#include "smt/context.hpp"

namespace MiniMC {
  namespace Support {
	namespace SMT {
	  class SMTFactory {
	  public:
		virtual SMTLib::Context_ptr construct () = 0;
	  };
	  
	  using SMTFactory_ptr = std::shared_ptr<SMTFactory>;
	  
	  SMTFactory_ptr getSMTFactory ();
	  
	}
  }

}
#endif
