#ifndef _CONCRETE_VM__
#define  _CONCRETE_VM__

#include "vm/vm.hpp"

namespace  MiniMC {
  namespace VM {
    namespace Concrete {
      MiniMC::VM::ValueLookup_ptr makeLookup (std::size_t );
      MiniMC::VM::PathControl_ptr makePathControl ( );
      MiniMC::VM::Memory_ptr makeMemory ( );
    }
  }
}

#endif
