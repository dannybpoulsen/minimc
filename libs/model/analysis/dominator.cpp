#include "model/analysis/dominator.hpp"
#include <algorithm>
#include <queue>

namespace MiniMC {
  namespace Model {
    namespace Analysis {

      DominatorInfo<MiniMC::Model::Location> calculateDominators(const MiniMC::Model::CFA& cfg) {
        DominatorInfo<MiniMC::Model::Location> domres;
        std::set<Location_ptr> init;
        for (auto& cur : cfg.getLocations()) {
          init.insert(cur);
        }

        for (auto& cur : cfg.getLocations()) {
          domres.setDominators(cur, init);
        }

        bool change = true;
        while (change) {
          change = false;
          for (auto& cur : cfg.getLocations()) {
            std::set<MiniMC::Model::Location_ptr> dominators;

            for (auto it = cur->iebegin(); it != cur->ieend(); ++it) {
              std::set<MiniMC::Model::Location_ptr> dominator_tmp;

              auto parent = (*it)->getFrom();
              if (it == cur->iebegin()) {
                std::copy(domres.dombegin(parent), domres.domend(parent), std::inserter(dominator_tmp, dominator_tmp.begin()));
              } else {
                std::set_intersection(dominators.begin(), dominators.end(),
                                      domres.dombegin(parent), domres.domend(parent),
                                      std::inserter(dominator_tmp, dominator_tmp.begin()));
              }

              std::swap(dominators, dominator_tmp);
            }
            dominators.insert(cur);
            if (domres.setDominators(cur, dominators))
              change = true;
          }
        }
        return domres;
      }

    } // namespace Analysis
  }   // namespace Model
} // namespace MiniMC
