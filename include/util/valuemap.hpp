#ifndef _VALUEMAP__
#define _VALUEMAP__

#include <functional>
#include <memory>
#include <ostream>

#include "hash/hashing.hpp"

namespace MiniMC {
  namespace Util {
    template <class T>
    struct GetIndex {
      std::size_t operator()(const T&);
    };

    template <class F, class T, class Index = GetIndex<F>>
    class FixedVector {
    public:
      FixedVector(std::size_t size) : mem(new T[size]), size(size) {}
      FixedVector(const FixedVector& o) : mem(new T[o.size]), size(o.size) {
        std::copy(o.mem.get(), o.mem.get() + o.size, mem.get());
      }

      const T& at(const F& f) const {
        assert(Index{}(f) < size);
        return mem.get()[Index{}(f)];
      }
      
      /*T& operator[](const F& f) {
        assert(Index{}(f) < size);
        return mem.get()[Index{}(f)];
	}*/

      void set (const F& f, T&& t) {
	assert(Index{}(f) < size);
	mem.get()[Index{}(f)] = std::move(t);	
      }
      
      const T& operator[](const F& f) const {
	assert(Index{}(f) < size);
        return mem.get()[Index{}(f)];
      }
      
      std::ostream& output(std::ostream& os) const {
        for (size_t i = 0; i < size; i++) {
          os << i << " : " << mem[i] << "-" << mem[i].getSize() << std::endl;
        }
        return os;
      }

      MiniMC::Hash::hash_t hash() const {
	MiniMC::Hash::Hasher hash;
	for (size_t i = 0; i < size; i++) {
	  constexpr bool has_get = requires(const T& t) {
	    t.get();
	  }; 
	  if constexpr (has_get || std::is_pointer_v<T>) {
	    hash << *mem[i];
	  }
	  else {	    
	    hash << mem[i];
	  }
	}
	return hash;
      }
      
      
      auto begin () const {
	return mem.get ();
      }

      auto end () const  {
	return mem.get ()+size;
      }
      
      
      auto getSize () const {return size;}
      
    private:
      std::unique_ptr<T[]> mem;
      std::size_t size;
    };

    template <class F, class T, class Index>
    inline std::ostream& operator<<(std::ostream& os, const FixedVector<F, T, Index>& vec) {
      return vec.output(os);
    }

  } // namespace Util
} // namespace MiniMC

#endif
