#ifndef _pathheap__
#define _pathheap__

#include <memory>
#include "support/feedback.hpp"
#include "util/ssamap.hpp"
#include "cpa/interface.hpp"
#include "smt/context.hpp"
#include "smt/solver.hpp"


namespace MiniMC {
  namespace CPA {
    namespace PathFormula {
      struct HeapEntry {
	SMTLib::Term_ptr content;
	std::size_t size; //Only fixed size allocations supported for now
      };
      
      class Heap {
      public:
	void free (SMTLib::Term_ptr pointer) {
	  //Ignore for now
	}

	void  write (SMTLib::Term_ptr content,SMTLib::Term_ptr pointer,std::size_t bytes, SMTLib::TermBuilder& ) {
	  //Ignore for now
	}

	SMTLib::Term_ptr  read (SMTLib::Term_ptr pointer,std::size_t bytes,SMTLib::TermBuilder& builder) const  {
	  auto sort = builder.makeBVSort (bytes*8);
	  return builder.makeVar (sort, "Read");    
	}

	SMTLib::Term_ptr allocate (MiniMC::uint64_t size,SMTLib::TermBuilder& builder) {
	  auto sort = builder.makeBVSort (64);
	  return builder.makeVar (sort, "Allocation");
	}
	
      private:
	std::vector<HeapEntry> entries;
      };
      
    }
  }
}


#endif
