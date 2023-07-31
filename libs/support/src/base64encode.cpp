#include <sstream>
#include <iostream>

#include "support/binary_encode.hpp"

namespace MiniMC {
  namespace Support {
    

    std::string STDEncode::decode (const std::string& str) {
      std::stringstream stream(str);
      std::string val;
      std::stringstream out;
      while (std::getline (stream,val,' ')) {
	out <<  static_cast<char> (std::stoi (val,nullptr,16));
      }
      
      return out.str ();
    }

  } // namespace Support
} // namespace MiniMC
