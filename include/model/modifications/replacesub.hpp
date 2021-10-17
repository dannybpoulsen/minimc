/**
 * @file   replace.hpp
 *
 * @brief  
 * 
 * 
 */
#ifndef _REPLACE_SUBS__
#define _REPLACE_SUB__

#include "model/cfg.hpp"
#include "support/sequencer.hpp"

namespace MiniMC {
  namespace Model {
    namespace Modifications {
      struct ReplaceSub : public MiniMC::Support::Sink<MiniMC::Model::Program> {

        virtual bool run(MiniMC::Model::Program& prgm) {
          auto& cfac = prgm.getConstantFactory();
          for (auto& F : prgm.getFunctions()) {
            for (auto& E : F->getCFG()->getEdges()) {
              if (E->hasAttribute<MiniMC::Model::AttributeType::Instructions>()) {
                auto& instrstr = E->getAttribute<MiniMC::Model::AttributeType::Instructions>();
                auto it = instrstr.begin();
                auto end = instrstr.end();
                for (it; it != end; ++it) {
                  if (it->getOpcode() == MiniMC::Model::InstructionCode::Sub) {
                    std::cerr << "Replace " << std::endl;
                    auto& content = it->getOps<MiniMC::Model::InstructionCode::Sub> ();
                    auto nvar = F->getVariableStackDescr()->addVariable("", content.op1->getType());
                    auto one_constant = cfac->makeIntegerConstant(1, content.op1->getType());
                    std::vector<MiniMC::Model::Instruction> vec;
                    vec.push_back(MiniMC::Model::createInstruction<MiniMC::Model::InstructionCode::Not>({.res = nvar, .op1 = content.op2}));
                    vec.push_back(MiniMC::Model::createInstruction<MiniMC::Model::InstructionCode::Add>({.res = nvar, .op1  = nvar, .op2 = one_constant}));
                    vec.push_back(MiniMC::Model::createInstruction<MiniMC::Model::InstructionCode::Add>({.res = content.res, .op1 = content.op1, .op2 = nvar}));

                    it = instrstr.replaceInstructionBySeq(it, vec.begin(), vec.end());
                    end = instrstr.end();
                  }
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
