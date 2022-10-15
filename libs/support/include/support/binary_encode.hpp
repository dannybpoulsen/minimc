#ifndef _BINARY_ENCODE__
#define _BINARY_ENCODE__

#include "host/types.hpp"
#include "support/exceptions.hpp"
#include <memory>
#include <string>
#include <sstream>
#include <iomanip>

namespace MiniMC {
  namespace Support {
    struct DecodeResult {
      std::size_t size;
      std::shared_ptr<MiniMC::BV8[]> buffer;
    };

    class BinaryEncoder {
    public:
      virtual std::string encode(const char* buf, std::size_t) = 0;
      virtual DecodeResult decode(const std::string& str) = 0;
    };

    class Base64Encode {
    public:
      std::string encode(const char* buf, std::size_t);
      DecodeResult decode(const std::string& str);
    };

    class STDEncode : public BinaryEncoder {
    public:
      std::string encode(const char* buf, std::size_t size) override {
	std::stringstream str;
	for (size_t i = 0; i < size; ++i) {
	  str << std::hex << std::setw(2) << std::setfill('0');
	  str << static_cast<int> (buf[i] & 0xFF )<< " ";
	}
	return str.str ();
      }
      
      DecodeResult decode(const std::string&) override {
	throw MiniMC::Support::Exception ("Decode not implemented");
      }
    };
    
  } // namespace Support
} // namespace MiniMC

#endif
