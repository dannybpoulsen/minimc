#ifndef _REGISTER__
#define _REGISTER__

#include <memory>
#include <cassert>
#include "support/types.hpp"

namespace MiniMC {
  namespace CPA {
    namespace ConcreteNoMem {
      class InRegister {
      public:
	InRegister (const void* m, size_t s) : mem (m), size(s) {}
	const void* getMem () const {return mem;}
	auto getSize () const {return size;}
	template<class T>
	const T& get () const {
	  assert(sizeof(T) == size);
	  return *reinterpret_cast<const T*> (mem); 
	}
	
      private:
	const void* mem;
	size_t size;
      };

      class ConstRegister : public  InRegister {
      public:
	ConstRegister (std::unique_ptr<uint8_t[]>& mem, size_t s) : InRegister(mem.get(),s),
								   mem(std::move(mem)) {}
	
      private:
	std::unique_ptr<uint8_t[]> mem;
      };
      
      using OutRegister = ConstRegister;
	  
      
    }
  }
}

#endif 
