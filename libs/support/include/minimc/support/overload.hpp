#ifndef _OVERLOAD__
#define _OVERLOAD__

#include "minimc/support/exceptions.hpp"

namespace MiniMC {
  namespace Support {

    template<class Res>
    struct Error {
      template<class... Args>
      Res operator() (Args... ) const {throw MiniMC::Support::Exception ("hh");}
    };
    
    template<class... Ts>
    struct Overload : Ts... {
      using Ts::operator()...;
    };


    template<class... Ts>
    Overload (Ts... ) -> Overload<Ts...>;
    
    
  }
}


#endif
