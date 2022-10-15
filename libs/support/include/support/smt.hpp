#ifndef _SUPPORT_SMT__
#define _SUPPORT_SMT__

#include "smt/context.hpp"
#include "host/types.hpp"
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

      template<class Iterator>
      void extractBytes (Iterator it, Iterator end, MiniMC::BV8* dest) {
	std::size_t bitscounted = 0;
	for (; it != end; ++it,++bitscounted ) {
	  if (*it) {
	    std::size_t bytenumber = bitscounted / 8;
	    std::size_t bit = bitscounted % 8;
	    
	    *(dest+bytenumber) |= (1 << bit);
	  }
	  
	}
      }
      
    } // namespace SMT
  }   // namespace Support

} // namespace MiniMC
#endif
