#include <iostream>
#include "support/types.hpp"
#include "support/casts.hpp"


namespace MiniMC {
  namespace Support {
#ifdef __x86_64__
    template<>
    uint16_t sext<uint8_t,uint16_t> (const uint8_t l) {
      uint16_t res;;
      asm ("cbw"
	   :"=a"(res) /* output */
	   :"a"(l)
	   );
      return res;
    }

    template<>
    uint32_t sext<uint16_t,uint32_t> (const uint16_t l) {
      uint32_t res;;
      asm ("cwde"
	   :"=a"(res) /* output */
	   :"a"(l)
	   );
      return res;
    }

    template<>
    uint64_t sext<uint32_t,uint64_t> (const uint32_t l) {
      uint64_t res;;
      asm ("cdqe"
	   :"=a"(res) /* output */
	   :"a"(l)
	   );
      return res;
    }
    
    
    template<>
    uint32_t sext<uint8_t,uint32_t> (const uint8_t l) {
      uint16_t temp = sext<uint8_t,uint16_t> (l);
      return sext<uint16_t,uint32_t> (temp);
    }

    template<>
    uint64_t sext<uint8_t,uint64_t> (const uint8_t l) {
      uint32_t temp = sext<uint8_t,uint32_t> (l);
      return sext<uint32_t,uint64_t> (temp);
    }
    
    
    

    template<>
    uint64_t sext<uint16_t,uint64_t> (const uint16_t l) {
      uint32_t temp = sext<uint16_t,uint32_t> (l);
      return sext<uint32_t,uint64_t> (temp);
    }

    template<>
    uint8_t sext<uint8_t,uint8_t> (const uint8_t l) {
      return l;
    }

    template<>
    uint16_t sext<uint16_t,uint16_t> (const uint16_t l) {
      return l;
    }

    template<>
    uint32_t sext<uint32_t,uint32_t> (const uint32_t l) {
      return l;
    }

    template<>
    uint64_t sext<uint64_t,uint64_t> (const uint64_t l) {
      return l;
    }

    
    template<>
    int16_t sext<int8_t,int16_t> (const int8_t l) {
      int16_t res;;
      asm ("cbw"
	   :"=a"(res) /* output */
	   :"a"(l)
	   );
      return res;
    }

    template<>
    int32_t sext<int16_t,int32_t> (const int16_t l) {
      int32_t res;;
      asm ("cwde"
	   :"=a"(res) /* output */
	   :"a"(l)
	   );
      return res;
    }

    template<>
    int64_t sext<int32_t,int64_t> (const int32_t l) {
      int64_t res;;
      asm ("cdqe"
	   :"=a"(res) /* output */
	   :"a"(l)
	   );
      return res;
    }
    
    
    template<>
    int32_t sext<int8_t,int32_t> (const int8_t l) {
      int16_t temp = sext<int8_t,int16_t> (l);
      return sext<int16_t,int32_t> (temp);
    }

    template<>
    int64_t sext<int8_t,int64_t> (const int8_t l) {
      int32_t temp = sext<int8_t,int32_t> (l);
      return sext<int32_t,int64_t> (temp);
    }
    
    
    

    template<>
    int64_t sext<int16_t,int64_t> (const int16_t l) {
      int32_t temp = sext<int16_t,int32_t> (l);
      return sext<int32_t,int64_t> (temp);
    }

    template<>
    int8_t sext<int8_t,int8_t> (const int8_t l) {
      return l;
    }

    template<>
    int16_t sext<int16_t,int16_t> (const int16_t l) {
      return l;
    }

    template<>
    int32_t sext<int32_t,int32_t> (const int32_t l) {
      return l;
    }

    template<>
    int64_t sext<int64_t,int64_t> (const int64_t l) {
      return l;
    }
    
  }
}
#endif
