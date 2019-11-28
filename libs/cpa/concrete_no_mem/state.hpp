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
      
      class State : public MiniMC::CPA::State,
		    public Stack
      {
      public:
	State (
	       Stack& global,
	       AllocState& galloc,
	       std::vector<Stack>& s,
	       std::vector<AllocState>& allocs
	       ) :
	  globalStack(global),
	  globalAlloc(galloc),
	  stacks(s),
	  allocs(allocs)
	{
	  for (size_t i = 0; i < allocs.size(); ++i) {
	    rets.emplace_back();
	  }
	}
	State (const State& ) = default;
	virtual std::ostream& output (std::ostream& os) const {return os << hash(0);}
	virtual MiniMC::Hash::hash_t hash (MiniMC::Hash::seed_t seed = 0) const {
	  MiniMC::Hash::seed_t s = seed;
	  s = globalStack.hash (s);
	  s= globalAlloc.hash (s);
	  for (auto& i : stacks)
	    s = i.hash (s);
	  for (auto& i : allocs)
	    s = i.hash (s);
	  for (auto& i : rets)
	    s = i.hash (s);
	  return heap.hash(s); 
	}

	struct StackDetails {
	  StackDetails (Stack& s,AllocState& st,
			Stack& gs,AllocState& gst,
			RetPoints& ret,
			size_t proc
			) : stack(s),alloc(st),
			    gstack(gs),galloc(gst),
			    ret(ret),
			    proc(proc)
	  {}

	  void push_frame (const MiniMC::Model::VariableStackDescr_ptr& ptr) {
	    alloc.push_frame (ptr->getTotalSize(),stack);
	  }
	  
	  void pop_frame (const MiniMC::Model::VariableStackDescr_ptr& ptr, const MiniMC::Model::VariableStackDescr_ptr& nstack) {
	    alloc.pop_frame (ptr->getTotalSize(),nstack);
	  }
	  
	  Stack& stack;
	  AllocState& alloc;
	  Stack& gstack;
	  AllocState& galloc;
	  RetPoints& ret;
	  size_t proc;
	};
	
	virtual std::shared_ptr<MiniMC::CPA::State> copy () const {return std::make_shared<State> (*this);}
	virtual std::shared_ptr<MiniMC::CPA::ConcreteNoMem::State> lcopy () const {return std::make_shared<State> (*this);}
	StackDetails getStackDetails (size_t p) {
	  return StackDetails (stacks[p],
			       allocs[p],
			       globalStack,
			       globalAlloc,
			       rets[p],
			       p
			       );
	}


 	std::size_t nbProcs () const {return stacks.size();}

	

	
	
      private:
	Stack globalStack;
	AllocState globalAlloc;
	std::vector<Stack> stacks;
	std::vector<AllocState> allocs;
	std::vector<RetPoints> rets;
	Heap heap;
      };
    }
  }
}

#endif


