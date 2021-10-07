#include <stdio.h>
#include <unistd.h>
#include <iostream>
namespace MiniMC {
  namespace Linux {
    class ColorModifier {
    public:
      enum class Code {
		 FG_RED      = 31,
		 FG_GREEN    = 32,
		 FG_BLUE     = 34,
		 FG_DEFAULT  = 39,
		 BG_RED      = 41,
		 BG_GREEN    = 42,
		 BG_BLUE     = 44,
		 BG_DEFAULT  = 49
      };
      
      ColorModifier(Code pCode) : code(pCode) {}
      void operator= (Code c) {code = c;}
      std::ostream& apply (std::ostream& os) const {
		if ( (&os == &std::cout && isatty( STDOUT_FILENO ))
			 || (&os == &std::cerr && isatty( STDERR_FILENO ))
			 || (&os == &std::clog && isatty( STDERR_FILENO )) ) {
		  return os << "\033[" << static_cast<int>(code) << "m";
		}
		else 
		  return os;
      }
    private:
      Code code;
    };
	
    
    
  }
}

inline std::ostream& operator<< (std::ostream& os, const MiniMC::Linux::ColorModifier& c) {
  return c.apply(os);
}
    
