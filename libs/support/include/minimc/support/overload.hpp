#ifndef _OVERLOAD__
#define _OVERLOAD__

#include "minimc/support/exceptions.hpp"

namespace MiniMC {
  namespace Support {

    template<class... Ts>
    struct Overload : Ts... {
      using Ts::operator()...;
    };

    
    
  }
}


#endif
