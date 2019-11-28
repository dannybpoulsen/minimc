#ifndef _EXCEPTION__
#define _EXCEPTION__

#include <stdexcept>


namespace MiniMC {
  namespace Support {
    class Exception : public std::runtime_error {
    public:
      Exception (const std::string& mess) : std::runtime_error(mess) {}
    };

    class BufferOverflow : Exception {
    public:
      BufferOverflow () : Exception ("BUffer Overflow") {}
    };

    class InvalidPointer : Exception {
    public:
      InvalidPointer () : Exception ("Invalid Pointer") {}
    };

    
    
  }
}

#endif 
