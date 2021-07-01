#ifndef _SUPPORT_SMT__
#define _SUPPORT_SMT__

#include <memory>
#include <iostream>
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

	  
	  
	  struct SMTDescr {
		SMTDescr (SMTLib::SMTBackendRegistrar* r) : r(r) {}
		virtual const std::string& name () {return r->getName ();}
		virtual const std::string& descr () {return r->getDescritpion ();}
		SMTLib::SMTBackendRegistrar* r;
	  };

	  template<class Iterator>
	  void getSMTBackends (Iterator it) {
		for (auto& itt : SMTLib::getSMTBackends ()) {
		  it = itt;
		}
	  }
	  
	  void setSMTSolver (const SMTDescr* );
	  
	}
  }

}
#endif
