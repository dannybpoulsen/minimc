#include "support/smt.hpp"
#include "smt/context.hpp"
#include "support/exceptions.hpp"
#include <vector>

namespace MiniMC {
  namespace Support {
    namespace SMT {

      class SMTFactoryL : public SMTFactory {
      public:
        SMTFactoryL(SMTLib::SMTBackendRegistrar* r) : r(r) {}
        virtual SMTLib::Context_ptr construct() {
          return r->getFunction()();
        }

      private:
        SMTLib::SMTBackendRegistrar* r;
      };

      SMTLib::SMTBackendRegistrar* r = 0;

      SMTFactory_ptr getSMTFactory(const SMTDescr* descr) {
	assert(descr);
	return std::make_shared<SMTFactoryL>(descr->r);
	  
      }

    } // namespace SMT
  }   // namespace Support
} // namespace MiniMC
