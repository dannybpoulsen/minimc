#ifndef _OVERLOAD__
#define _OVERLOAD__


namespace MiniMC {
  namespace Support {
    template<class... Ts> struct Overload : Ts... { using Ts::operator()...; };
    
  }
}


#endif
