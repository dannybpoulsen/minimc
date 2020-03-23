#ifndef _STATE__
#define _STATE__

#include <ostream>
#include <memory>
#include "hash/hashing.hpp"

namespace MiniMC {
  namespace CPA {
    
	using proc_id = std::size_t;
    class State {
    public:
      ~State () {}
      virtual std::ostream& output (std::ostream& os) const {return os << "_";}
      virtual MiniMC::Hash::hash_t hash (MiniMC::Hash::seed_t seed = 0) const {return 0;}
      virtual std::shared_ptr<State> copy () const {return std::make_shared<State> ();}
      virtual bool need2Store () const {return false;}
    };
    
    using State_ptr = std::shared_ptr<State>;
    
    inline std::ostream& operator<< (std::ostream& os, const State& state) {
      return state.output(os);
    }
  }
}


namespace std {
  template<>
  struct hash<MiniMC::CPA::State> {
	std::size_t operator()(MiniMC::CPA::State const& s) const noexcept {
	  return s.hash ();
	}
  };
  
}

#endif
