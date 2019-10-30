#ifndef _STACK__
#define _STACK__

#include "model/variables.hpp"
#include "support/types.hpp"

#include "register.hpp"

namespace MiniMC {
  namespace CPA {
    namespace ConcreteNoMem {
      class Stack {
      public:
	Stack (std::unique_ptr<MiniMC::uint8_t[]>& buffer)   {}
		InRegister load (MiniMC::Model::Variable_ptr& ptr) {
	  return InRegister (buffer.get()+ptr->getPlace(),ptr->getType()->getSize());
	}
		
		void save (OutRegister& reg,MiniMC::Model::Variable_ptr& ptr) {
		  assert(reg.getSize() == ptr->getType()->getSize());
		  std::copy (reinterpret_cast<const MiniMC::uint8_t*> (reg.getMem()),reinterpret_cast<const MiniMC::uint8_t*> (reg.getMem())+reg.getSize(),buffer.get()+ptr->getPlace());
	}
		
      private:
		std::unique_ptr<MiniMC::uint8_t[]> buffer; 
      };
    }
  }
}

#endif 
