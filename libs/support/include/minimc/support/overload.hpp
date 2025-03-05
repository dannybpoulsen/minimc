#ifndef _OVERLOAD__
#define _OVERLOAD__

#include "minimc/support/exceptions.hpp"
#include <cxxabi.h>

namespace MiniMC {
  namespace Support {
    
    
    template<class A, class... Args>
    std::string descriptive_string () {
      int     status;
      char   *realname;
      std::string res;
      
      const std::type_info  &ti = typeid(A);
      realname = abi::__cxa_demangle(ti.name(), NULL, NULL, &status);
      res = realname;
      std::free(realname);
      if constexpr (sizeof...(Args) > 0) {
	return res + " " +descriptive_string<Args...> ();
      }
      else
	return res;
    }
    
    template<class Res>
    struct Error {
      template<class... Args>
      Res operator() (Args... ) const {throw MiniMC::Support::Exception (descriptive_string<Args...> ());}
    };


    struct Ignore {
      template<class... Args>
      void operator() (Args... ) const {}
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
