#ifndef _SUPPORT_SMT__
#define _SUPPORT_SMT__

#include "smt/context.hpp"
#include "minimc/host/types.hpp"
#include <iostream>
#include <memory>
#include <expected>


namespace MiniMC {
  namespace Support {
    namespace SMT {

      
      struct SMTDescr {
        SMTDescr(SMTLib::SMTBackendRegistrar* r = nullptr) : r(r) {}
	SMTDescr (const SMTDescr&) = default;
        std::string name() const; 
        std::string descr() const; 
	SMTLib::Context_ptr makeContext () const;
      private:
	SMTLib::SMTBackendRegistrar* r;
      };
      enum class Error {
	NoSolver
      };
      
      class SMTSolverRepository {
      public:
	static SMTSolverRepository& get ();
	
	std::expected<SMTDescr,Error> getBackend (const std::string& i ) {
	  for (auto& r : descr) {
	    if (r.name () == i) {
	      return r;
	    }
	  }
	  return std::unexpected (Error::NoSolver);
	}
	
	auto& getSMTBackends () const {
	  return descr;
	}
      private:
	SMTSolverRepository ();
	std::vector<SMTDescr> descr;
      };

      
      
      template<class Iterator,class EIterator>
      void extractBytes (Iterator it, Iterator end, EIterator dest) {
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
