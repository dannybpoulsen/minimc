
#ifndef _OUTPUT__
#define _OUTPUT__
#include <iosfwd>

namespace MiniMC {
  namespace Model {
    class Program;
    class CFA;
    void writeCFA (std::ostream&, const MiniMC::Model::CFA&);
    void writeProgram (std::ostream&, const MiniMC::Model::Program&);
  }
}



#endif 
