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

	template<class AllocState>
	InRegister load (const MiniMC::Model::Variable_ptr& ptr, const AllocState& state) {
	  return InRegister (buffer.get()+state.memIndexConv(ptr),ptr->getType()->getSize());
	}

	template <class RegClass,class AllocState>
	void save (RegClass& reg,const MiniMC::Model::Variable_ptr& ptr,const AllocState& s) {
	  assert(reg.getSize() == ptr->getType()->getSize());
	  std::copy (reinterpret_cast<const MiniMC::uint8_t*> (reg.getMem()),reinterpret_cast<const MiniMC::uint8_t*> (reg.getMem())+reg.getSize(),buffer.get()+s.memIndexConv(ptr));
	}

	std::size_t getSize () const {return size;}
	const MiniMC::uint8_t* getBuffer () const {return buffer.get();}
	void extend (size_t s) {
	  std::unique_ptr<MiniMC::uint8_t[]> nbuf (new MiniMC::uint8_t[size+s]);
	  
	  std::copy(buffer.get(),buffer.get()+size,nbuf.get());
	  std::fill(nbuf.get()+size,nbuf.get()+size+s,0);
	  std::swap(nbuf,buffer);
	  size+= s;
	}

	
	virtual MiniMC::Hash::hash_t hash (MiniMC::Hash::seed_t seed = 0) const {
	  return MiniMC::Hash::Hash (getBuffer(),getSize(),seed);
	}

	
	
      private:
	std::unique_ptr<MiniMC::uint8_t[]> buffer;
	std::size_t size;     
      };
      
      class AllocState {
      public:
	std::size_t memIndexConv (const MiniMC::Model::Variable_ptr& ptr) const  {
	  return ptr->getPlace()+head;
	}
	
	virtual bool push_frame (std::size_t s, Stack& st) {
	  if (next+s < st.getSize())
	    st.extend(s);
	  head = next;
	  next = next+s;
	  return true;
	  
	}

	
	virtual void pop_frame (std::size_t s, const MiniMC::Model::VariableStackDescr_ptr& nstack) {
	  next = next-s;
	  head = next-nstack->getTotalSize();
	}
	
      virtual MiniMC::Hash::hash_t hash (MiniMC::Hash::seed_t seed = 0) const {
	std::size_t data[2] = {head,next}; 
	return MiniMC::Hash::Hash (data,2,seed);
      }
	
      private:
	std::size_t head = 0;
	std::size_t next = 0;
      };
      
    }
  }
}

#endif 
