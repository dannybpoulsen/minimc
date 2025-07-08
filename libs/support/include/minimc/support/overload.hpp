#ifndef _OVERLOAD__
#define _OVERLOAD__

#include "minimc/support/exceptions.hpp"
#include "minimc/support/feedback.hpp"
#include <cxxabi.h>
#include <sstream>
namespace MiniMC {
  namespace Support {

    template<class A>
    concept COutputtable = requires(std::ostream& os, const A& a) {
      {a.output(os)};
    };
    
    template<class A, class... Args>
    std::string descriptive_string (A a, Args... args) {
      int     status;
      char   *realname;
      std::string res;
      
      const std::type_info  &ti = typeid(A);
      realname = abi::__cxa_demangle(ti.name(), NULL, NULL, &status);
      res = realname;
      std::free(realname);
      if constexpr (COutputtable<A>) {
	std::stringstream str;
	a.output(str);
	res = res + str.str();
      }
	
      if constexpr (sizeof...(Args) > 0) {
	return res + " " +descriptive_string<Args...> (args...);
      }
      else
	return res;
    }
    
    template<class Res>
    struct Error {
      template<class... Args>
      Res operator() (Args... args) const {throw MiniMC::Support::Exception (descriptive_string<Args...> (args...));}
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
