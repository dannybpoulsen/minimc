/**
 * @file   random.hpp
 * @author Danny Bøgsted Poulsen <caramon@homemachine>
 * @date   Mon Apr 20 15:12:40 2020
 * 
 * @brief  
 * 
 * 
 */
#ifndef _RANDOM__
#define _RANDOM__

#include <limits>
#include <memory>
#include <random>
#include <type_traits>
#include <vector>

#include "host/types.hpp"

namespace MiniMC {
  namespace Support {

    /** RandomNumber generator "wrapper" with  a thread_local \p Engine for generating numbers.
	 * \tparam Engine The random number generator. 
	 */
    template <class Engine = std::mt19937_64>
    class RandomNumber {
    public:
      template <typename T, std::enable_if_t<std::is_integral<T>::value, bool> = 0>
      T uniform() {
        return this->uniform<T>(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
      }

      template <typename T, std::enable_if_t<std::is_integral<T>::value, bool> = 0>
      T uniform(T min, T max) {
        return std::uniform_int_distribution<T>{min, max}(getEngine());
      }

      template <typename T, std::enable_if_t<std::is_floating_point<T>::value, bool> = 0>
      T uniform() {
        return this->uniform<T>(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
      }

      template <typename T, std::enable_if_t<std::is_floating_point<T>::value, bool> = 0>
      T uniform(T min, T max) {
        return std::uniform_real_distribution<T>{min, max}(getEngine);
      }

      template <typename T, std::enable_if_t<std::is_floating_point<T>::value, bool> = 0>
      T exponential(T rate) {
        return std::exponential_distribution<T>{rate}(getEngine());
      }

      template <typename T, std::enable_if_t<std::is_floating_point<T>::value, bool> = 0>
      T shiftedExponential(T rate, T shift) {
        return shift + std::exponential_distribution<T>{rate}(getEngine());
      }

      template <typename T, typename Container = std::vector<T>>
      const T& selectUniform(const Container& c) {
        assert(c.size());
        return c.at(this->uniform<std::size_t>(0, c.size() - 1));
      }

    private:
      auto& getEngine() {
        static thread_local Engine engine(std::random_device{}());
        return engine;
      }
    };

  } // namespace Support
} // namespace MiniMC

#endif
