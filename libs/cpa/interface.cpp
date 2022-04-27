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
        auto& stackDescr = locstate.getLocation(i)->getCFG()->getFunction()->getVariableStackDescr();
        for (auto v : stackDescr.getVariables()) {
          os << v->getName() << " : ";
	  auto val = state.symbEvaluate (i,v);
	  os << *val;
	  os << "\n";
        }
      }
      return os;
    }
  } // namespace CPA
} // namespace MiniMC
