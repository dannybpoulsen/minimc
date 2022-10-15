#ifndef _HASH__
#define _HASH__

#include <cstdint>
#include <functional>
#include <vector>

uint64_t hash_impl(const void* addr, std::size_t len, uint64_t seed);

namespace MiniMC {
  namespace Hash {
    using hash_t = uint64_t;
    using seed_t = hash_t;

    template <class T>
    hash_t Hash(const T* data, std::size_t size, seed_t seed) {
      return hash_impl(data, size * sizeof(T), seed);
    }

    
    struct Hasher {

      operator hash_t () {
	return Hash (hashes.data(),hashes.size (),0);
      }
      
      template<class T>
      auto& operator<< (const T& obj) {
	std::hash<T> hasher;
	std::back_inserter (hashes) = hasher(obj);
	return *this;
      }
    private:
      std::vector<hash_t> hashes;
    };
    

  } // namespace Hash
} // namespace MiniMC

template <typename T>
concept hasHash = requires (T a) {
  a.hash ();
};

namespace std {
  template<class T> requires hasHash<T> 
  struct hash<T> {
    auto operator() (const T& t) {return t.hash ();}
  };
}

#endif
