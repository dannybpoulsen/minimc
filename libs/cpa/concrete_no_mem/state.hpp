#ifndef _CSTATE__
#define _CSTATE__

#include <memory>

#include "hash/hashing.hpp"
#include "cpa/interface.hpp"
#include "stack.hpp"
#include "support/types.hpp"
#include "heap.hpp"

namespace MiniMC {
  namespace CPA {
    namespace ConcreteNoMem {

      template<class Register>
      inline std::ostream& outputAs (std::ostream& os, Register& r, MiniMC::Model::Type_ptr& t) {
	{
	  switch (t->getTypeID ()) {
	  case MiniMC::Model::TypeID::Integer: {
	    switch (t->getSize ()) {
	    case 1:
	      return os << (int) r.template get<MiniMC::uint8_t> (); 
	    case 2:
	      return os <<  r.template get<MiniMC::uint16_t> ();
	    case 4:
	      return os <<  r.template get<MiniMC::uint32_t> ();
	    case 8:
	      return os <<  r.template get<MiniMC::uint16_t> ();
	    }
	  }
	  case MiniMC::Model::TypeID::Pointer: {
	    return os << r.template get<MiniMC::pointer_t> ();
	  }
	  case MiniMC::Model::TypeID::Bool:
	    return os <<  (r.template get<MiniMC::uint8_t> () ? "T" : "F");
	  default:
	    return os << "??";
	  }
	}
      }
      
      
      class State : public MiniMC::CPA::State
      {
      public:
	State (
	       MiniMC::pointer_t globalstack,
	       std::vector<MiniMC::pointer_t>& s,
	       Heap& h
	       ) :
	  globalStack(globalstack),
	  stacks(s),
	  heap(h)
	{
	}
	State (const State& ) = default;

	virtual MiniMC::Hash::hash_t hash (MiniMC::Hash::seed_t seed = 0) const {
	  auto s = MiniMC::Hash::Hash(&globalStack,1,seed);
	  s = MiniMC::Hash::Hash(stacks.data(),stacks.size(),s);
	  return heap.hash (s);
	}

	virtual std::ostream& output (std::ostream& os) const  override {
	  for (auto& s : stacks) {
	    Stack stack (heap.copy_out (s));
	    MiniMC::Model::VariableStackDescr* ptr = stack.getData().descr;
	    for (auto& h : ptr->getVariables ()) {
	      auto reg = stack.load (h);
	      auto type = h->getType ();
	      os << h->getName () << " " << *type <<" :" ;
	      outputAs (os, reg, type);
	      os << std::endl;
	    }
	  }
	  return os;
	}
	
	struct StackDetails {
	  StackDetails (pointer_t& s,
			pointer_t& gs,
			Heap& heap,
			size_t proc
			) : stack(heap.copy_out(s)),
			    gstack(heap.copy_out(gs)),
			    heap(heap),
			    proc(proc),
			    stack_ptr(s),
			    gstack_ptr(gs)
	  {}

	  
	  
	  void push_frame (const MiniMC::Model::VariableStackDescr_ptr& ptr) {
	    assert(!stack.isModified());
	    auto nptr = createStack (ptr,heap);
	    auto cc = heap.copy_out (nptr);
	    stack =  cc;
	    auto data = stack.getData ();
	    data.prev = stack_ptr;
	    stack.setData (data);
	    stack_ptr = nptr;
	  }
	  
	  void pop_frame () {
	    assert(!stack.isModified());
	    auto data = stack.getData ();
	    if (!MiniMC::Support::is_null (data.allocs))
	      heap.free_obj (data.allocs);
	    auto nptr = data.prev;
	    assert(!MiniMC::Support::is_null (nptr));
	    heap.free_obj (stack_ptr);
	    auto buf = heap.copy_out (nptr);
	    stack = buf;
	    stack_ptr = nptr;
	  }

	  void commit () {
	    if (stack.isModified ()) {
	      auto buffer = stack.releaseBuffer ();
	      heap.writeRaw (stack_ptr,buffer);
	    }
	    
	    if (gstack.isModified ()) {
	      auto buffer = gstack.releaseBuffer ();
	      heap.writeRaw (gstack_ptr,buffer);
	    }
	  }
	  
	  Stack stack;
	  Stack gstack;
	  Heap& heap;
	  size_t proc;
	  pointer_t& stack_ptr;
	  pointer_t& gstack_ptr;
	};
	
	virtual std::shared_ptr<MiniMC::CPA::State> copy () const {return std::make_shared<State> (*this);}
	virtual std::shared_ptr<MiniMC::CPA::ConcreteNoMem::State> lcopy () const {return std::make_shared<State> (*this);}
	StackDetails getStackDetails (size_t p) {
	  return StackDetails (stacks[p],
			       globalStack,
			       heap,
			       p
			       );
	}

	const StackDetails getStackDetails (size_t p) const {
	  return StackDetails (const_cast<pointer_t&>(stacks[p]),
			       const_cast<pointer_t&> (globalStack),
			       const_cast<Heap&> (heap),
			       p
			       );
	}

 	std::size_t nbProcs () const {return stacks.size();}

	

	
	
      private:
	pointer_t globalStack;
	std::vector<MiniMC::pointer_t> stacks;
	Heap heap;
      };
    }
  }
}

#endif


