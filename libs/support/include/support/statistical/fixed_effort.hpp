#ifndef _FIXED_EFFORT__
#define _FIXED_EFFORT__

#include <iostream>

#include "support/types.hpp"

namespace MiniMC {
  namespace Support {
    namespace Statistical {
      enum class Result {
		  Satis,
		  NSatis,
		  Maybe
      };
      
      class FixedEffort {
      public:
	FixedEffort (std::size_t e ) : effort (e) {}
	auto lProbability () const {return static_cast<MiniMC::proba_t> (satis) / total;}
	auto hProbability () const {return static_cast<MiniMC::proba_t> (satis+maybe) / total;}
	bool continueSampling () const {
	  return total < effort;
	}

	void sample (Result res) {
	  switch (res) {
	  case Result::Satis:
	    satis++;
	    break;
	  case Result::NSatis:
	    nsatis++;
	    break;
	  case Result::Maybe:
	    maybe++;
	    break;
	  }
	  total++;
	}
	
      private:
	const std::size_t effort;
	std::size_t total = 0;
	std::size_t satis = 0;
	std::size_t nsatis = 0;
	std::size_t maybe = 0;
	  
      };
    }
  }
}

#endif
