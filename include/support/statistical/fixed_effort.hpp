/**
 * @file   fixed_effort.hpp
 * @author Danny Bøgsted Poulsen <caramon@homemachine>
 * @date   Mon Apr 20 15:00:25 2020
 * 
 * @brief  
 * 
 * 
 */
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

      /** 
	   *
	   * Class for calculating a confidence interval for a probability using a fixed amount of samples, and confidence level.
	   *
	   */
      class FixedEffort {
      public:
        /** Options for the algorithm*/
        struct Options {
          size_t effort;         /**< THe number of samles*/
          MiniMC::proba_t alpha; /** The confidence level*/
        };

        FixedEffort(const Options& opt) : effort(opt.effort), alpha(opt.alpha) {}

        /** 
		 *
		 * @return  Lower bound on the confidence interval
		 */
        auto lProbability() const {
          return binomial_distribution<MiniMC::proba_t>::find_lower_bound_on_p(total, satis, satis < total ? alpha / 2 : alpha, binomial_distribution<MiniMC::proba_t>::clopper_pearson_exact_interval);
          ;
        }

        /** 
		 *
		 * @return  Lower bound on the confidence interval
		 */
        auto hProbability() const { return binomial_distribution<MiniMC::proba_t>::find_upper_bound_on_p(total, satis, satis > 0 ? alpha / 2 : alpha, binomial_distribution<MiniMC::proba_t>::clopper_pearson_exact_interval); }

        /** 
		 *
		 * @return  true if total number of samples is less than the effort needed. 
		 */
        bool continueSampling() const {
          return total < effort;
        }

        /** 
		 * Add a new sample and update the confidence bounds. 
		 * \param res The result of the sample
		 */
        void sample(Result res) {
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

      /** 
	   *
	   * Sequential probability estimation algorithm based on Clopper-Pearsons interval for Binomial distributions.
	   *
	   */
      class ClopperPearson {
      public:
        /** Options for the algorithm */
        struct Options {
          MiniMC::proba_t width; /**< The confidence interval width*/
          MiniMC::proba_t alpha; /**< The confidence */
        };
        ClopperPearson(const Options& opt) : width(opt.width), alpha(opt.alpha) {
          update();
        }

        /** 
		 *
		 * @return  Lower bound on the confidence interval
		 */
        auto lProbability() const { return lower; }

        /** 
		 *
		 * @return  upper bound on the confidence interval
		 */
        auto hProbability() const { return upper; }

        /** 
		 * 
		 *
		 * @return true if hProbability - lProbaility > width otherwise false 
		 */
        bool continueSampling() const {
          return upper - lower > width;
        }

        /** 
		 * Add a new sample and update the confidence bounds. 
		 * \param res The result of the sample
		 */
        void sample(Result res) {
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
          update();
        }

      private:
        void update() {
          lower = binomial_distribution<MiniMC::proba_t>::find_lower_bound_on_p(total, satis, satis < total ? alpha / 2 : alpha, binomial_distribution<MiniMC::proba_t>::clopper_pearson_exact_interval);
          upper = binomial_distribution<MiniMC::proba_t>::find_upper_bound_on_p(total, satis, satis > 0 ? alpha / 2 : alpha, binomial_distribution<MiniMC::proba_t>::clopper_pearson_exact_interval);
        }

        std::size_t total = 0;
        std::size_t satis = 0;
        std::size_t nsatis = 0;
        std::size_t maybe = 0;
	MiniMC::proba_t width = 1;
        MiniMC::proba_t alpha = 0;
        MiniMC::proba_t lower;
        MiniMC::proba_t upper;
      };

    } // namespace Statistical
  }   // namespace Support
} // namespace MiniMC

#endif
