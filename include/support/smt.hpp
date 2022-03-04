#ifndef _SUPPORT_SMT__
#define _SUPPORT_SMT__

#include "smt/context.hpp"
#include <iostream>
#include <memory>

namespace MiniMC {
  namespace Support {
    namespace SMT {

      
      struct SMTDescr {
        SMTDescr(SMTLib::SMTBackendRegistrar* r = nullptr) : r(r) {}
	SMTDescr (const SMTDescr&) = default;
        std::string name(); 
        std::string descr(); 
	SMTLib::Context_ptr makeContext () const;
      private:
	SMTLib::SMTBackendRegistrar* r;
      };

      
      
      template <class Iterator>
      void getSMTBackends(Iterator it) {
        for (auto& itt : SMTLib::getSMTBackends()) {
          it = itt;
        }
      }
      
    } // namespace SMT
  }   // namespace Support

} // namespace MiniMC
#endif
