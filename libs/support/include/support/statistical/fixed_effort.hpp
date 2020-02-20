#ifndef _FIXED_EFFORT__
#define _FIXED_EFFORT__

#include <iostream>

#include "support/types.hpp"
#include <boost/math/distributions/binomial.hpp>

using namespace boost::math;

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
		FixedEffort (std::size_t e,MiniMC::proba_t p) : effort (e),alpha(p) {}
		auto lProbability () const {return binomial_distribution<MiniMC::proba_t>::find_lower_bound_on_p(total,satis,satis < total ? alpha/2 : alpha,binomial_distribution<MiniMC::proba_t>::clopper_pearson_exact_interval);;}
		auto hProbability () const {return binomial_distribution<MiniMC::proba_t>::find_upper_bound_on_p(total,satis,satis > 0 ? alpha/2 : alpha,binomial_distribution<MiniMC::proba_t>::clopper_pearson_exact_interval);}
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
		MiniMC::proba_t alpha = 0;
      };
    }
  }
}

#endif
