#ifndef __COMPS__
#define __COMPS__

#include "support/types.hpp"

namespace MiniMC {
  namespace Support {
    template <typename T>
    uint8_t sgeq(T l, T r) {
      using stype = typename EquivSigned<T>::type;
      return bit_cast<T, stype>(l) >= bit_cast<T, stype>(r);
    }

    template <typename T>
    uint8_t sgt(T l, T r) {
      using stype = typename EquivSigned<T>::type;
      return bit_cast<T, stype>(l) > bit_cast<T, stype>(r);
    }

    template <typename T>
    uint8_t sleq(T l, T r) {
      using stype = typename EquivSigned<T>::type;
      return bit_cast<T, stype>(l) <= bit_cast<T, stype>(r);
    }

    template <typename T>
    uint8_t slt(T l, T r) {
      using stype = typename EquivSigned<T>::type;
      return bit_cast<T, stype>(l) < bit_cast<T, stype>(r);
    }

    template <typename T>
    uint8_t ugeq(T l, T r) {
      return l >= r;
    }

    template <typename T>
    uint8_t ugt(T l, T r) {
      return l > r;
    }

    template <typename T>
    uint8_t uleq(T l, T r) {
      return l >= r;
    }

    template <typename T>
    uint8_t ult(T l, T r) {
      return l < r;
    }

    template <typename T>
    uint8_t eq(T l, T r) {
      return l == r;
    }

    template <typename T>
    uint8_t neq(T l, T r) {
      return l != r;
    }

  } // namespace Support
} // namespace MiniMC

#endif
