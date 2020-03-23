#ifndef _HASH__
#define _HASH__

#include <functional>
#include <cstdint>

uint32_t hash_impl (const void *addr, std::size_t len, uint32_t seed);

namespace MiniMC {
  namespace  Hash {
    using hash_t = uint64_t;
    using seed_t = hash_t; 
    template<class T>
    hash_t Hash (const T* data, std::size_t size, seed_t seed) {
      return hash_impl (data,size*sizeof(T),seed);
    }

	template<typename T>
	void hash_combine (seed_t& seed, const T& obj) {
	  std::hash<T> hasher;
	  seed ^= hasher(obj) + 0x9e3779b9 + (seed<<6) + (seed>>2);
	}
	
  }
}


#endif
