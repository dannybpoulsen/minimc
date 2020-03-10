#ifndef _SHELP__
#define _SHELP__

#include <type_traits>
#include <string>
#include <boost/endian/conversion.hpp> 
#include "support/types.hpp"

namespace MiniMC {

  template<typename T>
  struct saveHelper {
    saveHelper (MiniMC::uint8_t* ptr, size_t size) : mem(ptr),size(size) {
      assert(sizeof(T) <= size);
    }
    
    saveHelper& operator= (const T& t) {
      T buf = t;
      if constexpr (std::is_fundamental<T>::value) {
	boost::endian::native_to_little_inplace(buf);
      }
      std::copy (reinterpret_cast<const MiniMC::uint8_t*> (&buf),reinterpret_cast<const MiniMC::uint8_t*> (&buf)+sizeof(T),mem);
      return *this;
    }
  private:
    MiniMC::uint8_t* mem;
    std::size_t size;
  };

  template<typename T>
  struct loadHelper {
    loadHelper (const MiniMC::uint8_t* ptr, size_t size) {
      assert(sizeof(T) <= size);
      std::copy (ptr,ptr+sizeof(T),reinterpret_cast<MiniMC::uint8_t*> (&res));  
      if constexpr (std::is_fundamental<T>::value) {
	  boost::endian::little_to_native_inplace(res);
	}
    }
    operator T () {
      return res;
    }
    
  private:
    T res;
  };

  
 
  
  
}


#endif
