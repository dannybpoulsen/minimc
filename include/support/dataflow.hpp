#ifndef _DATA_FLOW__
#define _DATA_FLOW__

#include <functional>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include "support/queue_stack.hpp"

namespace MiniMC {
  namespace Support {
    template <class Loc_ptr, class T>
    struct GenKillState {
      GenKillState(const Loc_ptr& loc) : location(loc) {}
      bool kill(std::function<bool(T&)> killPred) {
        bool changed = false;
        for (auto it = state.begin(); it != state.end(); ++it) {
          if (killPred(*it)) {
            it = state.erase(it);
            changed = true;
          }
        }
        return changed;
      }

      bool gen(const T& t) {
        bool changed = false;
        if (!state.count(t)) {
          state.insert(t);
          changed = true;
        }
        return changed;
      }

      const Loc_ptr& getLocation() const {
        return location;
      }

      auto begin() const {
        return state.begin();
      }

      auto end() const {
        return state.end();
      }

    private:
      std::unordered_set<T> state;
      Loc_ptr location;
    };

    template <class Loc, class T, class Edge>
    struct GenKillInterface {
      static bool update(const GenKillState<Loc, T>& from,
                         const Edge& edge,
                         GenKillState<Loc, T>& to) {
        return false;
      }
    };

    template <class Loc, class T, class Edge, class Interface, class Iter>
    auto ForwardDataFlowAnalysis(Iter begin, Iter end) {
      using Loc_ptr = std::shared_ptr<Loc>;
      std::unordered_map<Loc_ptr, GenKillState<Loc_ptr, T>> states;
      MiniMC::Support::Queue<Loc> waiting;
      for (auto it = begin; it != end; ++it) {
        states.insert(std::make_pair(*it, GenKillState<Loc_ptr, T>(*it)));
        waiting.insert(*it);
      }

      while (!waiting.empty()) {
        Loc_ptr fromLoc = waiting.pull();
        GenKillState<Loc_ptr, T>& cur = states.at(fromLoc);
        for (auto eit = fromLoc->ebegin(); eit != fromLoc->eend(); ++eit) {
          GenKillState<Loc_ptr, T>& to = states.at(eit->getTo());
          if (Interface::update(cur, *eit, to)) {
            waiting.insert(eit->getTo());
          }
        }
      }

      return states;
    }

  } // namespace Support
} // namespace MiniMC

#endif
