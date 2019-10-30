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

      
      class OutRegister {
      public:
		OutRegister (std::unique_ptr<MiniMC::uint8_t[]>& m, std::size_t s) : mem(std::move(m)), size(s) {}
		const void* getMem () const {return mem.get();}
		auto getSize () const {return size;}
		template<class T>
		const T& get () const {
		  assert(sizeof(T) == size);
		  return *reinterpret_cast<const T*> (mem.get()); 
		}
		
	  private:
		std::unique_ptr<uint8_t[]> mem;
		std::size_t size;
      };
	  
      
    }
  }
}

#endif 
