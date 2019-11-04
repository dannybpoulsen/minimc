#ifndef _STACK__
#define _STACK__

#include "hash/hashing.hpp"
#include "model/variables.hpp"
#include "support/types.hpp"

#include "register.hpp"

namespace MiniMC {
  namespace CPA {
    namespace ConcreteNoMem {
      class Stack {
      public:
		Stack (std::unique_ptr<MiniMC::uint8_t[]>& buffer,std::size_t& s) : buffer(std::move(buffer)),size(s)   {}
	Stack (const Stack& s)   {
	  buffer.reset (new MiniMC::uint8_t[s.getSize ()]);
	  std::copy(s.buffer.get(),s.buffer.get()+size,buffer.get());
	  size = s.size;
	}

	Stack() : buffer(nullptr), size(0) {}
	
	InRegister load (MiniMC::Model::Variable_ptr& ptr) {
	  return InRegister (buffer.get()+ptr->getPlace(),ptr->getType()->getSize());
	}
	
	void save (OutRegister& reg,MiniMC::Model::Variable_ptr& ptr) {
	  assert(reg.getSize() == ptr->getType()->getSize());
	  std::copy (reinterpret_cast<const MiniMC::uint8_t*> (reg.getMem()),reinterpret_cast<const MiniMC::uint8_t*> (reg.getMem())+reg.getSize(),buffer.get()+ptr->getPlace());
	}

	std::size_t getSize () const {return size;}
	const MiniMC::uint8_t* getBuffer () const {return buffer.get();}

	
	virtual MiniMC::Hash::hash_t hash (MiniMC::Hash::seed_t seed = 0) const {
	  return MiniMC::Hash::Hash (getBuffer(),getSize(),seed);
	}
	
      private:
	std::unique_ptr<MiniMC::uint8_t[]> buffer;
	std::size_t size;
      };
    }
  }
}

#endif 
