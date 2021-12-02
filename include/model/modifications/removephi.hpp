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
                if (instrstream.isPhi ()) {
                  for (auto& inst : instrstream) {
                    auto& content = inst.getOps<InstructionCode::Assign>();
                    auto nvar = F->getVariableStackDescr()->addVariable(std::static_pointer_cast<Register>(content.res)->getName() + "PHI-tmp", content.res->getType());
                    replacemap.insert(std::make_pair(content.res.get(), nvar));

                    stream.addInstruction<MiniMC::Model::InstructionCode::Assign>({.res = replacemap.at(content.res.get()), .op1 = content.op1});
                  }

                  for (auto& inst : instrstream) {
                    auto& content = inst.getOps<InstructionCode::Assign>();

                    auto val = replacemap.count(content.op1.get()) ? replacemap.at(content.op1.get()) : content.op1;
                    stream.addInstruction<MiniMC::Model::InstructionCode::Assign>({.res = replacemap.at(content.res.get()), .op1 = val});
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
