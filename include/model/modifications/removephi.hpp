#ifndef _REPLACEPHI__
#define _REPLACEPHI__

#include <unordered_map>

#include "model/cfg.hpp"
#include "model/instructions.hpp"
#include "model/modifications/simplify_cfg.hpp"
#include "support/sequencer.hpp"

namespace MiniMC {
  namespace Model {
    namespace Modifications {

      struct RemovePhi : public MiniMC::Support::Sink<MiniMC::Model::Program> {
        virtual bool run(MiniMC::Model::Program& prgm) {
          for (auto& F : prgm.getFunctions()) {
            for (auto& E : F->getCFG()->getEdges()) {
              if (E->hasAttribute<MiniMC::Model::AttributeType::Instructions>()) {
                auto& instrstream = E->getAttribute<MiniMC::Model::AttributeType::Instructions>();
                InstructionStream stream;
                std::unordered_map<MiniMC::Model::Value*, MiniMC::Model::Variable_ptr> replacemap;
                if (instrstream.isPhi) {
                  for (auto& inst : instrstream) {
                    InstHelper<InstructionCode::Assign> ass(inst);
                    auto nvar = F->getVariableStackDescr()->addVariable(std::static_pointer_cast<Register>(ass.getResult())->getName() + "PHI-tmp", ass.getResult()->getType());
                    replacemap.insert(std::make_pair(ass.getResult().get(), nvar));
                    
                    stream.back_inserter() = MiniMC::Model::createInstruction<MiniMC::Model::InstructionCode::Assign> ({.res = replacemap.at(ass.getResult().get()),.op1 = ass.getValue ()});
                  }

                  for (auto& inst : instrstream) {
                    InstHelper<InstructionCode::Assign> ass(inst);
                    auto val = replacemap.count(ass.getValue().get()) ? replacemap.at(ass.getValue().get()) : ass.getValue();
                    stream.back_inserter() = MiniMC::Model::createInstruction<MiniMC::Model::InstructionCode::Assign> ({.res = replacemap.at(ass.getResult().get()),.op1 = val});
                  }

                  E->delAttribute<MiniMC::Model::AttributeType::Instructions>();
                  E->setAttribute<MiniMC::Model::AttributeType::Instructions>(stream);
                }
              }
            }
          }
          return true;
        }
      };

    } // namespace Modifications
  }   // namespace Model

} // namespace MiniMC

#endif
