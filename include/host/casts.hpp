#ifndef _CASTS__
#define _CASTS__

#include "support/exceptions.hpp"
#include "host/types.hpp"

namespace MiniMC {
  namespace Host {
    template <class T, class P>
    P zext(const T t);

    template <class T, class P>
    P sext(const T t);

    template <class T, class P>
    P trunc(const T& t) {
      static_assert(sizeof(P) <= sizeof(T));
      using uns_from = typename HostType<sizeof(T)*8>::Unsigned;
      using uns_to = typename HostType<sizeof(P)*8>::Unsigned;

      uns_from casted = bit_cast<T, uns_from>(t);
      uns_to res = static_cast<uns_to>(casted);

      return bit_cast<uns_to, P>(res);
    }

  } // namespace Support
} // namespace MiniMC

#endif
