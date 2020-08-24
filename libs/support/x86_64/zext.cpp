#include <iostream>
#include "support/types.hpp"
#include "support/casts.hpp"


namespace MiniMC {
  namespace Support {
#ifdef __x86_64__
    template<>
    uint16_t zext<uint8_t,uint16_t> (const uint8_t l) {
      uint16_t res;;
      asm ("movzbw %%al, %%bx "
	   :"=b"(res) /* output */
	   :"a"(l)
	   );
      return res;
    }

    template<>
    uint32_t zext<uint16_t,uint32_t> (const uint16_t l) {
      uint32_t res;;
      asm ("movzwl %%ax, %%ebx"
	   :"=b"(res) /* output */
	   :"a"(l)
	   );
      return res;
    }

    template<>
    uint64_t zext<uint32_t,uint64_t> (const uint32_t l) {
      uint64_t res;;
      asm ("xor %%rbx, %%rbx;"
		   "mov %%eax, %%ebx"
	   :"=b"(res) /* output */
	   :"a"(l)
	   );
      return res;
    }
    
    
    template<>
    uint32_t zext<uint8_t,uint32_t> (const uint8_t l) {
      uint16_t temp = zext<uint8_t,uint16_t> (l);
      return zext<uint16_t,uint32_t> (temp);
    }

    template<>
    uint64_t zext<uint8_t,uint64_t> (const uint8_t l) {
      uint32_t temp = zext<uint8_t,uint32_t> (l);
      return zext<uint32_t,uint64_t> (temp);
    }
    
    
    

    template<>
    uint64_t zext<uint16_t,uint64_t> (const uint16_t l) {
      uint32_t temp = zext<uint16_t,uint32_t> (l);
      return zext<uint32_t,uint64_t> (temp);
    }

    template<>
    uint8_t zext<uint8_t,uint8_t> (const uint8_t l) {
      return l;
    }

    template<>
    uint16_t zext<uint16_t,uint16_t> (const uint16_t l) {
      return l;
    }

    template<>
    uint32_t zext<uint32_t,uint32_t> (const uint32_t l) {
      return l;
    }

    template<>
    uint64_t zext<uint64_t,uint64_t> (const uint64_t l) {
      return l;
    }

    
    template<>
    int16_t zext<int8_t,int16_t> (const int8_t l) {
      int16_t res;;
      asm ("movzbw %%al, %%bx"
	   :"=b"(res) /* output */
	   :"a"(l)
	   );
      return res;
    }

    template<>
    int32_t zext<int16_t,int32_t> (const int16_t l) {
      int32_t res;;
      asm ("movzwl %%ax, %%ebx"
	   :"=b"(res) /* output */
	   :"a"(l)
	   );
      return res;
    }

    template<>
    int64_t zext<int32_t,int64_t> (const int32_t l) {
      int64_t res;;
        asm ("xor %%rbx,%%rbx;"
			 "mov %%eax, %%ebx"
	   :"=b"(res) /* output */
	   :"a"(l)
	   );
      return res;
    }
    
    
    template<>
    int32_t zext<int8_t,int32_t> (const int8_t l) {
      int16_t temp = zext<int8_t,int16_t> (l);
      return zext<int16_t,int32_t> (temp);
    }

    template<>
    int64_t zext<int8_t,int64_t> (const int8_t l) {
      int32_t temp = zext<int8_t,int32_t> (l);
      return zext<int32_t,int64_t> (temp);
    }
    
    
    

    template<>
    int64_t zext<int16_t,int64_t> (const int16_t l) {
      int32_t temp = zext<int16_t,int32_t> (l);
      return zext<int32_t,int64_t> (temp);
    }

    template<>
    int8_t zext<int8_t,int8_t> (const int8_t l) {
      return l;
    }

    template<>
    int16_t zext<int16_t,int16_t> (const int16_t l) {
      return l;
    }

    template<>
    int32_t zext<int32_t,int32_t> (const int32_t l) {
      return l;
    }

    template<>
    int64_t zext<int64_t,int64_t> (const int64_t l) {
      return l;
    }
    
  }
}
#endif
