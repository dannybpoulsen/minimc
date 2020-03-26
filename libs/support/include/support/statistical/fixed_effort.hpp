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
		struct Options {
		  size_t effort;
		  MiniMC::proba_t alpha;
		}; 
		
		FixedEffort (const Options& opt) : effort (opt.effort),alpha(opt.alpha) {}
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

      class ClopperPearson {
      public:
		struct Options {
		  MiniMC::proba_t width;
		  MiniMC::proba_t alpha;
		}; 
		ClopperPearson (const Options& opt) : width (opt.width), alpha(opt.alpha) {
		  update ();
		}
		auto lProbability () const {return lower;}
		auto hProbability () const {return upper;}
		bool continueSampling () const {
		  return upper-lower > width;
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
		  update ();
		}
		
      private:
		void update () {
		  lower =  binomial_distribution<MiniMC::proba_t>::find_lower_bound_on_p(total,satis,satis < total ? alpha/2 : alpha,binomial_distribution<MiniMC::proba_t>::clopper_pearson_exact_interval);
		  upper =  binomial_distribution<MiniMC::proba_t>::find_upper_bound_on_p(total,satis,satis > 0 ? alpha/2 : alpha,binomial_distribution<MiniMC::proba_t>::clopper_pearson_exact_interval);
		}
		std::size_t total = 0;
		std::size_t satis = 0;
		std::size_t nsatis = 0;
		std::size_t maybe = 0;
		MiniMC::proba_t alpha = 0;
		MiniMC::proba_t width = 1;
		MiniMC::proba_t  lower;
		MiniMC::proba_t  upper;
      };
      
    }
  }
}

#endif
