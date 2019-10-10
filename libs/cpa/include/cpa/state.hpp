#ifndef _STATE__
#define _STATE__

#include <ostream>
#include <memory>
#include "hash/hashing"

namespace MiniMC {
  namespace CPA {
    class State {
    public:
      virtual ~State () {}
      virtual std::ostream& output (std::ostream&) const = 0;
      virtual MiniMC::Hash::hash_t hash (MiniMC::Hash::seed_t seed = 0) const = 0;
    };

    using State_ptr = std::shared_ptr<State>;
    
    inline std::ostream& operator<< (std::ostream& os, const State& state) {
      return state.output(os);
    }
    
  }
}

#endif
