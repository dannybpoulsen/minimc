
#include <unordered_set>

#include "model/analysis/find_location_defs.hpp"
#include "support/dataflow.hpp"
#include "support/queue_stack.hpp"

#include "model/utils.hpp"

namespace MiniMC {
  namespace Model {
    namespace Analysis {
      struct GenKillInterface {
        static bool update(const MiniMC::Support::GenKillState<MiniMC::Model::Location_ptr, MiniMC::Model::Instruction*>& from,
                           const MiniMC::Model::Edge_ptr& edge,
                           MiniMC::Support::GenKillState<MiniMC::Model::Location_ptr, MiniMC::Model::Instruction*>& to) {
          assert(edge->getFrom().get() == from.getLocation());
          assert(edge->getTo().get() == to.getLocation());
          bool changed = false;
          std::unordered_set<MiniMC::Model::Register_ptr> varsDef;

          if (edge->template hasAttribute<MiniMC::Model::AttributeType::Instructions>()) {
            auto& instrs = edge->template getAttribute<MiniMC::Model::AttributeType::Instructions>();

            for (auto iit = instrs.rbegin(); iit != instrs.rend(); ++iit) {
              auto valueDefined = valueDefinedBy(*iit);

              if (valueDefined) {
                MiniMC::Model::Register_ptr varDefined = std::static_pointer_cast<MiniMC::Model::Register>(valueDefined);
                if (varsDef.count(varDefined))
                  continue;
                varsDef.insert(varDefined);
                changed |= to.gen(&*iit);
              }
            }
          }

          for (auto& instr : from) {
            if (!varsDef.count(std::static_pointer_cast<MiniMC::Model::Register>(valueDefinedBy(*instr)))) {
              changed |= to.gen(instr);
            }
          }

          return changed;
        }
      };

      CFGDefs calculateDefs(MiniMC::Model::Function& f) {
        auto& cfg = f.getCFG();
        auto& vStack = f.getVariableStackDescr();
        CFGDefs defs(cfg, vStack.getTotalVariables());

        auto lit = cfg.getLocations().begin();
        auto eit = cfg.getLocations().end();
        auto res = MiniMC::Support::ForwardDataFlowAnalysis<MiniMC::Model::Location,
                                                            MiniMC::Model::Instruction*,
                                                            MiniMC::Model::Edge_ptr,
                                                            GenKillInterface,
                                                            decltype(lit)>(lit, eit);
        //Analysis done. Convert result to what caller expects :-)
        for (auto& tup : res) {
          MiniMC::Model::Location_ptr loc = tup.first;
          auto& genkillstate = tup.second;
          for (auto instr : genkillstate) {
            defs.getDefs(loc).insert(std::static_pointer_cast<MiniMC::Model::Register>(valueDefinedBy(*instr)),
                                     instr);
          }
        }

        return defs;
      }
    } // namespace Analysis
  }   // namespace Model
} // namespace MiniMC
