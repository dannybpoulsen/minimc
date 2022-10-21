#ifndef _ALGORITHMS__
#define _ALGORITHMS__

#include <list>
#include <functional>

namespace MiniMC {
  namespace Algorithms {
    template<class T>
    struct Observable {
      using observer = std::function<void(const T&)>;
      void operator= (T&& t) {
	val = t; 
      }

      void notif_listeners () const {
	for ( auto& o : observers)
	  o (val);
      }

      
      const T& get () const {return val;}
      
      
      void listen (observer obs) {observers.push_back (obs);}
    private:
      T val;
      std::list<observer> observers;
    };

  }
}


#endif
