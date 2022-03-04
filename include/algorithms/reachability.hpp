#ifndef _REACHABILITY__
#define _REACHABILITY__

#include <functional>

#include "algorithms/successorgen.hpp"
#include "cpa/interface.hpp"
#include "support/feedback.hpp"
#include "support/localisation.hpp"

namespace MiniMC {
  namespace Algorithms {

    struct DummyInsert {
      void operator=(const MiniMC::CPA::State_ptr&) {
      }
    };

    template <class Passed>
    struct PassedInsert {
      PassedInsert(MiniMC::Support::Progresser& p, Passed& pass) : waitmess("Waiting: %1%, Passed: %2%"), progresser(p), passed(pass) {}
      void operator=(const MiniMC::CPA::State_ptr& ptr) {
        progresser.progressMessage(waitmess.format(passed.getWSize(), passed.getPSize()));
      }
      MiniMC::Support::Localiser waitmess;
      MiniMC::Support::Progresser& progresser;
      Passed& passed;
    };

    using Predicate = std::function<bool(const MiniMC::CPA::State_ptr&)>;

    template <class Passed, class Inserter = DummyInsert>
    MiniMC::CPA::State_ptr reachabilitySearch(Passed& passed, Inserter& insert, const MiniMC::CPA::State_ptr& searchFrom, Predicate predicate, MiniMC::CPA::Transferer_ptr transfer) {
      passed.insert(searchFrom);
      MiniMC::Algorithms::Generator generator(transfer);
      while (passed.hasWaiting()) {
        auto cur = passed.pull();
	if (predicate(cur)) {
          return cur;
        }
        insert = cur;
        auto it_pair = generator.generate(cur);
        auto it = it_pair.first;
        auto end = it_pair.second;
        for (; it != end; ++it) {
          if (!it->hasErrors()) {
            passed.insert(it->state);
          }
        }
      }
      return nullptr;
    }
  } // namespace Algorithms
} // namespace MiniMC

#endif
