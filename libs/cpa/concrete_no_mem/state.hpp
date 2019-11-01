#ifndef _STATE__
#define _STATE__

#include <memory>

#include "hash/hashing.hpp"
#include "cpa/interface.hpp"
#include "stack.hpp"
#uinclude "support/types.hpp"

namespace MiniMC {
  namespace CPA {
    namespace ConcreteNoMem {
      class State : public MiniMC::CPA::State,
		    public Stack
      {
	State (std::unique_ptr<MiniMC::uint8_t[]>& buffer,std::size_t s) : stack(buffer,s) {}
	virtual std::ostream& output (std::ostream& os) const {return os << "S";}
	virtual MiniMC::Hash::hash_t hash (MiniMC::Hash::seed_t seed = 0) const {return MiniMC::Hash::Hash (getBufer(),getSize());}
	virtual std::shared_ptr<State> copy () const {return std::make_shared<State> (*this);}
	Stack& getStack () {return *this;}
      };
    }
  }
}

#endif


