#ifndef _BINARY_ENCODE__
#define _BINARY_ENCODE__

#include "minimc/support/exceptions.hpp"
#include "minimc/host/types.hpp"
#include <memory>
#include <string>
#include <sstream>
#include <iomanip>

namespace MiniMC {
  namespace Support {
    struct DecodeResult {
      std::size_t size;
      std::unique_ptr<MiniMC::BV8[]> buffer;
    };

    class BinaryEncoder {
    public:
      virtual std::string encode(std::span<const MiniMC::BV8> span) = 0;
      virtual std::string decode(const std::string& str) = 0;
    };

    

    class STDEncode : public BinaryEncoder {
    public:
      std::string encode(std::span<const MiniMC::BV8> span ) override {
	std::stringstream str;
	for (auto& d : span) {
	  str << std::hex << std::setw(2) << std::setfill('0');
	  str << static_cast<int> (d & 0xFF )<< " ";
	}
	return str.str ();
      }
      
      std::string decode(const std::string&) override;
    };
    
  } // namespace Support
} // namespace MiniMC

#endif
