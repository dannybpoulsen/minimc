#include <stack>
#include <unordered_map>
#include <vector>

#include "model/analysis/dominator.hpp"
#include "model/analysis/loops.hpp"

namespace MiniMC {
  namespace Model {
    namespace Analysis {

      Loop_ptr constructLoop(
          const MiniMC::Model::Location_ptr& head,
          const MiniMC::Model::Location_ptr& latch) {

        std::stack<MiniMC::Model::Location_ptr> stack;
        auto loop = std::make_unique<Loop>(head);

        if (!loop->contains(latch)) {
          loop->insertLatch(latch);
          stack.push(latch);
        }

        while (!stack.empty()) {
          auto search = stack.top();
          stack.pop();
          if (!loop->contains(search)) {
            loop->insertBody(search);
            for (auto it = search->iebegin(); it != search->ieend(); ++it) {
              if ((*it)->getFrom() != head) {
                stack.push((*it)->getFrom());
              }
            }
          }
        }
        return loop;
      }

      LoopInfo createLoopInfo(const MiniMC::Model::CFA& cfg) {

        auto dominatorinfo = MiniMC::Model::Analysis::calculateDominators(cfg);
        std::unordered_map<MiniMC::Model::Location_ptr, Loop_ptr> loopData;
        for (auto& edge : cfg.getEdges()) {
          if (dominatorinfo.dominates(edge->getFrom(), edge->getTo())) {
            auto loop = constructLoop(edge->getTo(), edge->getFrom());
            if (loopData.count(edge->getTo())) {
              auto& ll = loopData[edge->getTo()];
              std::for_each(loop->latch_begin(), loop->latch_end(), [&](const auto& l) { ll->insertLatch(l); });
              std::for_each(loop->body_begin(), loop->body_end(), [&](const auto& l) { ll->insertBody(l); });

            } else {
              loopData[edge->getTo()] = std::move(loop);
            }
          }
        }

        LoopInfo info;
        for (auto& it : loopData) {
          it.second->finalise();
          info.addLoop(it.second);
          info.nestLoops();
        }
        return info;
      }

    } // namespace Analysis
  }   // namespace Model
} // namespace MiniMC
