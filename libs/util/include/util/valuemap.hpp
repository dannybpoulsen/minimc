#ifndef _VALUEMAP__
#define _VALUEMAP__

#include <ostream>
#include <functional>
#include <memory>
#include <gsl/pointers>

#include "hash/hashing.hpp"
#include "model/variables.hpp"

namespace MiniMC {
  namespace Util {
	template<class T>
	struct GetIndex {
	  std::size_t operator() (const T& t) {return 0;}
	};	

	template<class F, class T,class Index = GetIndex<F>>
	class FixedVector {
	public:
	  FixedVector (std::size_t size) : mem(new T[size]),size(size){}
	  FixedVector (const FixedVector& o) : mem(new T[o.size]),size(o.size) {
		std::copy (o.mem.get(),o.mem.get()+o.size,mem.get());
	}
	  
	  const T& at (const F& f) const  { assert(Index{}(f) < size); return mem.get()[Index{}(f)];}
	  T& operator[] (const F& f) { assert(Index{}(f) < size); return mem.get()[Index{}(f)];}
	  const T& operator[] (const F& f) const  { assert(Index{}(f) < size); return mem.get()[Index{}(f)];}
	  
	  std::ostream& output (std::ostream& os) const {
		for (size_t i = 0; i < size; i++) {
		  os << i << " : " << mem[i] << mem[i].getSize()<< std::endl;
		}
		return os;
	  }
	  
	  MiniMC::Hash::hash_t hash (MiniMC::Hash::seed_t s) const {
		MiniMC::Hash::seed_t seed = s;
		for (size_t i = 0; i < size; i++) {
		  MiniMC::Hash::hash_combine (seed,mem[i]);
		}
		return seed;
	  }
	  
	  
	private:
	  std::unique_ptr<T[]> mem;
	  std::size_t size;
	};

	template<class F, class T,class Index>
	inline std::ostream& operator<< (std::ostream& os, const FixedVector<F,T,Index>& vec) {
	  return vec.output (os);
	}
	
  }
}

#endif
