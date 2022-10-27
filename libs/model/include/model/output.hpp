
#ifndef _OUTPUT__
#define _OUTPUT__
#include <iosfwd>

namespace MiniMC {
  namespace Model {
    class Program;
    void writeProgram (std::ostream&, const MiniMC::Model::Program&);
  }
}



#endif 
