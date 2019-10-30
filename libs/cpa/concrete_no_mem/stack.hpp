#ifndef _STACK__
#define _STACK__

#include "model/variables.hpp"
#include "support/types.hp"

#include "register.hpp"

namespace MiniMC {
  namespace CPA {
    namespace ConcreteNoMem {
      class Stack {
      public:
	Stack (std::unique_ptr<MiniMC::uint8_t[]>& buffer)   {}
	InRegister load (MiniMC::Variable_ptr& ptr) {
	  return InRegister (buffer.get()+ptr->getPlace(),ptr->getType()->getSize());
	}
	
	void save (OutRegister& reg,MiniMC::Variable_ptr& ptr) {
	  assert(reg.getSize() == ptr->getType()->getSize());
	  std::copy (reg.getMem(),reg.getMem()+reg.getSize(),buffer.get()+ptr->getPlace());
	}
	
      private:
	std::unique_ptr<MiniMC::uint8_t[]> buffer; 
      };
    }
  }
}

#endif 
