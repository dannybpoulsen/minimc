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

      SMTFactory_ptr getSMTFactory() {
        if (r) {
          return std::make_shared<SMTFactoryL>(r);
        } else {
          throw MiniMC::Support::ConfigurationException("No  SMT Backend Selected");
        }
      }

      void setSMTSolver(const SMTDescr* descr) {
        r = (descr)->r;
      }

    } // namespace SMT
  }   // namespace Support
} // namespace MiniMC
