#ifndef _DOMINATOR_ANALYSIS_MANAGER__
#define _DOMINATOR_ANALYSIS_MANAGER__
#include <algorithm>
#include <memory>
#include <set>
#include <unordered_map>
#include <vector>

#include "model/cfg.hpp"

namespace MiniMC {
  namespace Model {
    namespace Analysis {
      template <class T>
      class DominatorInfo {
      public:
        using T_ptr = std::shared_ptr<T>;
        using const_iterator = typename std::set<T_ptr>::const_iterator;

        const_iterator dombegin(const T_ptr& n) const { return dominator_map[n.get()].begin(); }
        const_iterator domend(const T_ptr& n) const { return dominator_map[n.get()].end(); }
        bool dominates(const T_ptr& node, const T_ptr& dominatedBy) const {
          return dominator_map[node.get()].count(dominatedBy);
        }

        bool setDominators(const T_ptr& n, const std::set<T_ptr>& h) {
          if (dominator_map[n.get()] == h)
            return false;
          else {
            dominator_map[n.get()] = h;
            return true;
          }
        }

      private:
        mutable std::unordered_map<const T*, std::set<T_ptr>> dominator_map;
      };

      DominatorInfo<MiniMC::Model::Location> calculateDominators(const MiniMC::Model::CFA& cfg);

    } // namespace Analysis
  }   // namespace Model
} // namespace MiniMC

#endif
