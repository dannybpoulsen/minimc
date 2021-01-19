#ifndef _VALUEMAP__
#define _VALUEMAP__

#include <ostream>
#include <functional>
#include <memory>
#include <gsl/pointers>

#include "model/variables.hpp"

namespace MiniMC {
  namespace Util {
	template<class T>
	struct GetIndex {
	  std::size_t operator() (const T& t) {return 0;}
	};	

	template<class F, class T,class Index = GetIndex<F>>
	class FixedVector {
	  FixedVector (std::size_t size) : mem(new T[size]),size(size){}
	  FixedVector (const FixedVector&) = default;
	  const T& at (const F& f) const  { assert(Index{}(f) < size); return mem.get()[Index{}(f)];}
	  T& operator[] (const F& f) { assert(Index{}(f) < size); return mem.get()[Index{}(f)];}
	  T& operator[] (const F& f) const  { assert(Index{}(f) < size); return mem.get()[Index{}(f)];}
	  
				
	private:
	  std::shared_ptr<T[]> mem;
	  std::size_t size;
	};
  }
}


#endif
