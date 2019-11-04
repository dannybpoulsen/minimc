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
	State (std::vector<Stack>& s)  {}
		State (const State& ) = default;
	virtual std::ostream& output (std::ostream& os) const {return os << "S";}
	virtual MiniMC::Hash::hash_t hash (MiniMC::Hash::seed_t seed = 0) const {
	  MiniMC::Hash::seed_t s = seed;
	  for (auto& i : stacks)
	    s = i.hash (s);
	  return s; 
	}
		virtual std::shared_ptr<MiniMC::CPA::State> copy () const {return std::make_shared<State> (*this);}
		virtual std::shared_ptr<MiniMC::CPA::ConcreteNoMem::State> lcopy () const {return std::make_shared<State> (*this);}
		const Stack& getStack (size_t p) const  {return stacks[p];}
		std::size_t nbProcs () const {return stacks.size();}
      private:
		std::vector<Stack> stacks;
      };
    }
  }
}

#endif


