#ifndef _LOCALISATION__
#define _LOCALISATION__

#include <boost/format.hpp>

namespace MiniMC {
  namespace Support {
    class Localiser {
    public:
      Localiser (const std::string& str) : formatter(str) {}

      template<typename T> 
      Localiser& operator% (const T& t) {
	formatter % t;
	return *this;
      }
      
      const std::string str () {return formatter.str();}
      
    private:
      boost::format formatter;
    };
  }
}

#endif
