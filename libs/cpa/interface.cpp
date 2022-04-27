#include "cpa/interface.hpp"
#include "hash/hashing.hpp"
#include "model/cfg.hpp"
#include <ostream>

namespace MiniMC {
  namespace CPA {
    std::ostream& operator<<(std::ostream& os, const State& state) {
      auto& locstate = state.getLocationState ();
      proc_id procs = locstate.nbOfProcesses();
      for (proc_id i = 0; i < procs; ++i) {
        os << "Process." << i << "@" << locstate.getLocation(i)->getInfo().getName() << "\n";
        
      }
      return os;
    }
  } // namespace CPA
} // namespace MiniMC
