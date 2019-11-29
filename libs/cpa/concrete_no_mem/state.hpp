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
      struct RetPoints {
	template<class T>
	std::shared_ptr<T> pop () {
	  if(locs.size()) {
	    auto res = locs.back();
	    locs.pop_back();
	    return reinterpret_cast<T*> (res)->shared_from_this();
	  }
	  return nullptr;
	}

	template<class T>
	void push (const std::shared_ptr<T>& ptr) {
	  locs.push_back(ptr.get());
	}

	virtual MiniMC::Hash::hash_t hash (MiniMC::Hash::seed_t seed = 0) const {
	  if (!locs.size()) {
	    return seed;
	  }
	  else {
	    return MiniMC::Hash::Hash (reinterpret_cast<const uint64_t*> (locs.data()),locs.size(),seed);
	  }
	}
	
	std::vector<void*> locs;
      };
      
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
	  for (size_t i = 0; i < stacks.size(); ++i) {
	    rets.emplace_back();
	  }
	}
	State (const State& ) = default;
	virtual std::ostream& output (std::ostream& os) const {return os << hash(0);}
	virtual MiniMC::Hash::hash_t hash (MiniMC::Hash::seed_t seed = 0) const {
	  auto s = MiniMC::Hash::Hash(&globalStack,1,seed);
	  s = MiniMC::Hash::Hash(stacks.data(),stacks.size(),s);
	  for (auto& r : rets)
	    s = r.hash(s);
	  return heap.hash (s);
	}

	struct StackDetails {
	  StackDetails (pointer_t& s,
			pointer_t& gs,
			RetPoints& ret,
			Heap& heap,
			size_t proc
			) : stack(heap.copy_out(s)),
			    gstack(heap.copy_out(gs)),
			    ret(ret),
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
	  
	  void pop_frame (const MiniMC::Model::VariableStackDescr_ptr& ptr, const MiniMC::Model::VariableStackDescr_ptr& nstack) {
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
	  RetPoints& ret;
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
			       rets[p],
			       heap,
			       p
			       );
	}


 	std::size_t nbProcs () const {return stacks.size();}

	

	
	
      private:
	pointer_t globalStack;
	std::vector<MiniMC::pointer_t> stacks;
	std::vector<RetPoints> rets;
	Heap heap;
      };
    }
  }
}

#endif


