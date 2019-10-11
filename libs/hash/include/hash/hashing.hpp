#ifndef _HASH__
#define _HASH__

#include <cstdint>

uint32_t hash_impl (const void *addr, std::size_t len, uint32_t seed);

namespace MiniMC {
  namespace  Hash {
    using hash_t = uint32_t;
    using seed_t = hash_t; 
    template<class T>
    hash_t Hash (const T* data, std::size_t size,seed_t seed) {
      return hash_impl (data,size*sizeof(T),seed);
    }
  }
}


#endif
