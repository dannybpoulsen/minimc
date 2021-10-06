/**
 * @file   storehelp.hpp
 * @date   Mon Apr 20 17:13:13 2020
 * 
 * @brief  
 * 
 * 
 */
#ifndef _SHELP__
#define _SHELP__

#include <type_traits>
#include <string>
#include <boost/endian/conversion.hpp> 
#include "support/types.hpp"

namespace MiniMC {

  /** MiniMC has a compile time switch for selecting whether the memory layout should be little-endian or big-endian. 
   *  saveHelper helps storing values (in the CPUs native format)  in the selected format  into a buffer.    
   * \param T the type this saveHelper stores value for. 
   */
  template<typename T>
  struct saveHelper {
	/** 
	 * 
	 *
	 * @param ptr The buffer to store values into 
	 * @param size size of the buffer
	 *
	 */
    saveHelper (MiniMC::uint8_t* ptr, size_t size) : mem(ptr),size(size) {
      assert(sizeof(T) <= size);
    }
	
	/** 
	 * Store \p t into the buffer associated to this saveHelper
	 *
	
	 * @return *this
	 */
    saveHelper& operator= (const T& t) {
      T buf = t;
      if constexpr (std::is_fundamental<T>::value) {
#ifndef MINIMC_USE_BIG_ENDIAN
		  boost::endian::native_to_little_inplace(buf);
#else
		  boost::endian::native_to_big_inplace(buf);
#endif
		}
      std::copy (reinterpret_cast<const MiniMC::uint8_t*> (&buf),reinterpret_cast<const MiniMC::uint8_t*> (&buf)+sizeof(T),mem);
      return *this;
    }
  private:
    MiniMC::uint8_t* mem;
    std::size_t size;
  };

  /** MiniMC has a compile time switch for selecting whether the memory layout should be little-endian or big-endian. 
   *  loadHelper helps loading values (in the selected format)
   *  from a buffer into the native format of the CPU.   
   * \param T the type this loadHelper loads value for. 
   */
 
  template<typename T>
  struct loadHelper {
	/** 
	 * 
	 *
	 * @param ptr The buffer to load from
	 * @param size the size of buffer
	 *
	 */
    loadHelper (const MiniMC::uint8_t* ptr, size_t size) {
      assert(sizeof(T) <= size);
      std::copy (ptr,ptr+sizeof(T),reinterpret_cast<MiniMC::uint8_t*> (&res));  
      if constexpr (std::is_fundamental<T>::value) {
#ifndef MINIMC_USE_BIG_ENDIAN
		  boost::endian::little_to_native_inplace(res);
#else
		  boost::endian::big_to_native_inplace(res);
#endif
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
