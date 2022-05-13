#include <sstream>

#include "base64.h"
#include "support/binary_encode.hpp"

namespace MiniMC {
  namespace Support {
    std::string Base64Encode::encode(const char* buf, std::size_t len) {
      return base64_encode(reinterpret_cast<unsigned char const*>(buf), len, false);
    }

    DecodeResult Base64Encode::decode(const std::string& str) {
      std::string dec = base64_decode(str);
      DecodeResult res;
      res.size = dec.length();
      res.buffer.reset(new MiniMC::BV8[dec.length()]);
      std::copy(dec.c_str(), dec.c_str() + dec.length(), res.buffer.get());
      return res;
    }

  } // namespace Support
} // namespace MiniMC
