#ifndef _BINARY_ENCODE__
#define _BINARY_ENCODE__

#include <memory>
#include <string>
#include "support/types.hpp"

namespace MiniMC {
  namespace Support {
    struct DecodeResult {
      std::size_t size;
      std::shared_ptr<MiniMC::uint8_t[]> buffer;
    };
    
    class BinaryEncoder {
    public:
      virtual std::string encode (const char* buf, std::size_t) = 0;
      virtual DecodeResult decode (const std::string& str) = 0;
      
    };

    class Base64Encode {
    public:
      std::string encode (const char* buf, std::size_t);
      DecodeResult decode (const std::string& str);
      
    };
  }
}

#endif
