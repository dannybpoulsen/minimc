#ifndef _STACK__
#define _STACK__

#include "hash/hashing.hpp"
#include "model/variables.hpp"
#include "support/types.hpp"
#include "support/pointer.hpp"
#include "register.hpp"


namespace MiniMC {
  namespace CPA {
    namespace ConcreteNoMem {

      struct __attribute__((packed)) StackData {
	pointer_t prev;
	pointer_t allocs;
	offset_t allocSize;
	MiniMC::Model::Variable* ret;
	MiniMC::Model::VariableStackDescr* descr;
      };
      
      template<class Heap>
      pointer_t createStack (const MiniMC::Model::VariableStackDescr_ptr& s, Heap& h) {
	StackData data;
	data.prev = MiniMC::Support::null_pointer();
	data.allocs = MiniMC::Support::null_pointer();
	data.allocSize = 0;
	data.ret = nullptr;
	data.descr = s.get ();
	return h.make_obj_initialiser (s->getTotalSize()+sizeof(StackData),data);
	
      }
      
      class Stack {
      public:
	Stack (std::unique_ptr<MiniMC::uint8_t[]> buffer) : buffer(std::move(buffer))   {
	  update ();
	}
	
	Stack (Stack&& s) : buffer(std::move(s.buffer)),content(s.content),data(s.data) {
	  
	}

	void operator= (std::unique_ptr<MiniMC::uint8_t[]>& buffer) {
	  std::swap (buffer,this->buffer);
	  update();
	}
	
	InRegister load (const MiniMC::Model::Variable_ptr& ptr) const {
	  return InRegister (content+ptr->getPlace(),ptr->getType()->getSize());
	}
	
	template <class RegClass>
	void save (RegClass& reg,const MiniMC::Model::Variable_ptr& ptr) {
	  assert(reg.getSize() == ptr->getType()->getSize());
	  std::copy (reinterpret_cast<const MiniMC::uint8_t*> (reg.getMem()),reinterpret_cast<const MiniMC::uint8_t*> (reg.getMem())+reg.getSize(),content+ptr->getPlace());
	  modified = true;
	}

	bool isModified () const {return modified;}
	
	const MiniMC::uint8_t* getBuffer () const {return buffer.get();}
	std::unique_ptr<MiniMC::uint8_t[]> releaseBuffer () {return std::move(buffer);}
	
	void setData (StackData t) {
	  *data = t;
	  modified = true;
	}
	
	auto getData () const {
	  return *data;
	}
	
      private:
	void update () {
	  data = reinterpret_cast<StackData*> (this->buffer.get());
	  content = this->buffer.get()+sizeof(StackData);
	}
	std::unique_ptr<MiniMC::uint8_t[]> buffer;
	MiniMC::uint8_t* content;
	StackData* data;
	bool modified = false;
      };      
    }
  }
}

#endif 
