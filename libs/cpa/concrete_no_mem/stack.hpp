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
	Stack (const Stack& s) : buffer(new MiniMC::uint8_t[s.size]),
				 size(s.size)
	{
	  
	  if (s.buffer)
	    assert(s.buffer);
	    std::copy(s.buffer.get(),s.buffer.get()+size,buffer.get());
	  }

	Stack() : buffer(nullptr), size(0) {}
	
	InRegister load (const MiniMC::Model::Variable_ptr& ptr) {
	  return InRegister (buffer.get()+ptr->getPlace(),ptr->getType()->getSize());
	}

	template <class RegClass>
	void save (RegClass& reg,const MiniMC::Model::Variable_ptr& ptr) {
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
