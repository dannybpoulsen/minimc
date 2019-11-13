#ifndef _CSTATE__
#define _CSTATE__

#include <memory>

#include "hash/hashing.hpp"
#include "cpa/interface.hpp"
#include "stack.hpp"
#include "support/types.hpp"

namespace MiniMC {
  namespace CPA {
    namespace ConcreteNoMem {
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
	{}
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
	  return s; 
	}

	struct StackDetails {
	  StackDetails (Stack& s,AllocState& st,
			Stack& gs,AllocState& gst) : stack(s),alloc(st),
						      gstack(gs),galloc(gst) {}
	  Stack& stack;
	  AllocState& alloc;
	  Stack& gstack;
	  AllocState& galloc;
	  
	};
	
	virtual std::shared_ptr<MiniMC::CPA::State> copy () const {return std::make_shared<State> (*this);}
	virtual std::shared_ptr<MiniMC::CPA::ConcreteNoMem::State> lcopy () const {return std::make_shared<State> (*this);}
	/*const Stack& getStack (size_t p) const  {return stacks[p];}
	Stack& getStack (size_t p)  {return stacks[p];}
	AllocState& getAlloc (size_t p)  {return allocs[p];}
	const Stack& getGStack (size_t p) const  {return globalStack;}
	Stack& getGStack (size_t p)  {return globalStack;}
	AllocState& getGAlloc (size_t p)  {return globalAlloc;}
	*/
	StackDetails getStackDetails (size_t p) {
	  return StackDetails (stacks[p],
			       allocs[p],
			       globalStack,
			       globalAlloc);
	}


 	std::size_t nbProcs () const {return stacks.size();}

	void push_frame (MiniMC::Model::VariableStackDescr_ptr& ptr,size_t t) {
	  allocs[t].push_frame (ptr->getTotalSize(),stacks[t]);
	}

	void pop_frame (MiniMC::Model::VariableStackDescr_ptr& ptr,size_t t) {
	  allocs[t].pop_frame (ptr->getTotalSize(),stacks[t]);
	}

	
	
      private:
	Stack globalStack;
	AllocState globalAlloc;
	std::vector<Stack> stacks;
	std::vector<AllocState> allocs;
      };
    }
  }
}

#endif


