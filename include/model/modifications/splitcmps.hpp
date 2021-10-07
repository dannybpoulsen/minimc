/**
 * @file   splitcmps.hpp
 * @date   Mon Apr 20 17:09:39 2020
 * 
 * @brief  
 * 
 * 
 */
#ifndef _SPLITCMPS__
#define _SPLITCMPS__

#include <algorithm>
#include <vector>

#include "model/analysis/manager.hpp"
#include "model/cfg.hpp"
#include "support/sequencer.hpp"
#include "support/workinglist.hpp"

namespace MiniMC {
  namespace Model {
    namespace Modifications {
      struct SplitCompares : public MiniMC::Support::Sink<MiniMC::Model::Program> {

        virtual bool runFunction(const MiniMC::Model::Function_ptr& F) {
          auto isCMP = [](auto& inst) { return MiniMC::Model::isOneOf<MiniMC::Model::InstructionCode::ICMP_SGT,
                                                                      MiniMC::Model::InstructionCode::ICMP_SGE,
                                                                      MiniMC::Model::InstructionCode::ICMP_SLT,
                                                                      MiniMC::Model::InstructionCode::ICMP_SLE,
                                                                      MiniMC::Model::InstructionCode::ICMP_EQ,
                                                                      MiniMC::Model::InstructionCode::ICMP_NEQ,
                                                                      MiniMC::Model::InstructionCode::ICMP_UGT,
                                                                      MiniMC::Model::InstructionCode::ICMP_UGE,
                                                                      MiniMC::Model::InstructionCode::ICMP_ULT,
                                                                      MiniMC::Model::InstructionCode::ICMP_ULE>(inst); };

          auto cfg = F->getCFG();
          auto it = cfg->getEdges().begin();
          auto end = cfg->getEdges().end();
          MiniMC::Support::WorkingList<MiniMC::Model::Edge_ptr> wlist;
          auto inserter = wlist.inserter();
          std::for_each(cfg->getEdges().begin(),
                        cfg->getEdges().end(),
                        [&](const MiniMC::Model::Edge_ptr& e) { inserter = e; });

          for (auto E : wlist) {
            if (E->hasAttribute<MiniMC::Model::AttributeType::Instructions>()) {
              auto& instrs = E->getAttribute<MiniMC::Model::AttributeType::Instructions>();
              if (isCMP(instrs.last())) {
                auto buildEdge = [&]<MiniMC::Model::InstructionCode inst,
                                     MiniMC::Model::InstructionCode left>(auto& v) {
                  MiniMC::Model::InstHelper<inst> helper(instrs.last());
                  auto loc = cfg->makeLocation(E->getTo()->getInfo());

                  auto it = E->getTo()->ebegin();
                  auto end = E->getTo()->eend();

                  for (; it != end; ++it) {
                    auto nedge = cfg->makeEdge(loc, it->getTo());
                    nedge->copyAttributesFrom(**it);
                  }
                  std::vector<MiniMC::Model::Instruction> instr;
                  std::copy(instrs.begin(), instrs.end(), std::back_inserter(instr));
                  auto tt = cfg->makeEdge(E->getFrom(), loc);
                  tt->setAttribute<MiniMC::Model::AttributeType::Instructions>(instr);
                  std::vector<MiniMC::Model::Instruction> ttI{MiniMC::Model::InstBuilder<left>().setLeft(helper.getLeftOp()).setRight(helper.getRightOp()).BuildInstruction(),
                                                              MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::Assign>().setResult(helper.getResult()).setValue(v).BuildInstruction()};
                  auto& llnew = tt->getAttribute<MiniMC::Model::AttributeType::Instructions>();
                  llnew.replaceInstructionBySeq(llnew.end() - 1, ttI.begin(), ttI.end());
                };
                auto cfac = F->getPrgm()->getConstantFactory();
                auto tfac = F->getPrgm()->getTypeFactory();
                auto boolT = tfac->makeBoolType();
                auto tt = cfac->makeIntegerConstant(1, boolT);
                auto ff = cfac->makeIntegerConstant(0, boolT);

                switch (instrs.last().getOpcode()) {
                  case MiniMC::Model::InstructionCode::ICMP_SGT:
                    buildEdge.operator()<MiniMC::Model::InstructionCode::ICMP_SGT, MiniMC::Model::InstructionCode::PRED_ICMP_SGT>(tt);
                    buildEdge.operator()<MiniMC::Model::InstructionCode::ICMP_SGT, MiniMC::Model::InstructionCode::PRED_ICMP_SLE>(ff);

                    break;
                  case MiniMC::Model::InstructionCode::ICMP_SGE:
                    buildEdge.operator()<MiniMC::Model::InstructionCode::ICMP_SGE, MiniMC::Model::InstructionCode::PRED_ICMP_SGE>(tt);
                    buildEdge.operator()<MiniMC::Model::InstructionCode::ICMP_SGE, MiniMC::Model::InstructionCode::PRED_ICMP_SLT>(ff);

                    break;
                  case MiniMC::Model::InstructionCode::ICMP_SLT:
                    buildEdge.operator()<MiniMC::Model::InstructionCode::ICMP_SLT, MiniMC::Model::InstructionCode::PRED_ICMP_SLT>(tt);
                    buildEdge.operator()<MiniMC::Model::InstructionCode::ICMP_SLT, MiniMC::Model::InstructionCode::PRED_ICMP_SGE>(ff);

                    break;
                  case MiniMC::Model::InstructionCode::ICMP_SLE:
                    buildEdge.operator()<MiniMC::Model::InstructionCode::ICMP_SLE, MiniMC::Model::InstructionCode::PRED_ICMP_SLE>(tt);
                    buildEdge.operator()<MiniMC::Model::InstructionCode::ICMP_SLE, MiniMC::Model::InstructionCode::PRED_ICMP_SGT>(ff);

                    break;
                  case MiniMC::Model::InstructionCode::ICMP_EQ:
                    buildEdge.operator()<MiniMC::Model::InstructionCode::ICMP_EQ, MiniMC::Model::InstructionCode::PRED_ICMP_EQ>(tt);
                    buildEdge.operator()<MiniMC::Model::InstructionCode::ICMP_NEQ, MiniMC::Model::InstructionCode::PRED_ICMP_NEQ>(ff);

                    break;
                  case MiniMC::Model::InstructionCode::ICMP_NEQ:
                    buildEdge.operator()<MiniMC::Model::InstructionCode::ICMP_NEQ, MiniMC::Model::InstructionCode::PRED_ICMP_NEQ>(tt);
                    buildEdge.operator()<MiniMC::Model::InstructionCode::ICMP_NEQ, MiniMC::Model::InstructionCode::PRED_ICMP_EQ>(ff);

                    break;
                  case MiniMC::Model::InstructionCode::ICMP_UGT:
                    buildEdge.operator()<MiniMC::Model::InstructionCode::ICMP_UGT, MiniMC::Model::InstructionCode::PRED_ICMP_UGT>(tt);
                    buildEdge.operator()<MiniMC::Model::InstructionCode::ICMP_UGT, MiniMC::Model::InstructionCode::PRED_ICMP_ULE>(ff);

                    break;
                  case MiniMC::Model::InstructionCode::ICMP_UGE:
                    buildEdge.operator()<MiniMC::Model::InstructionCode::ICMP_SGE, MiniMC::Model::InstructionCode::PRED_ICMP_UGE>(tt);
                    buildEdge.operator()<MiniMC::Model::InstructionCode::ICMP_SGE, MiniMC::Model::InstructionCode::PRED_ICMP_ULT>(ff);

                    break;
                  case MiniMC::Model::InstructionCode::ICMP_ULT:
                    buildEdge.operator()<MiniMC::Model::InstructionCode::ICMP_ULT, MiniMC::Model::InstructionCode::PRED_ICMP_ULT>(tt);
                    buildEdge.operator()<MiniMC::Model::InstructionCode::ICMP_ULT, MiniMC::Model::InstructionCode::PRED_ICMP_UGE>(ff);
                    break;
                  case MiniMC::Model::InstructionCode::ICMP_ULE:
                    buildEdge.operator()<MiniMC::Model::InstructionCode::ICMP_ULE, MiniMC::Model::InstructionCode::PRED_ICMP_ULE>(tt);
                    buildEdge.operator()<MiniMC::Model::InstructionCode::ICMP_ULE, MiniMC::Model::InstructionCode::PRED_ICMP_UGT>(ff);

                    break;
                };
                if (E->getTo()->nbIncomingEdges() <= 1) {
                  cfg->deleteLocation(E->getTo());
                } else {
                  cfg->deleteEdge(E);
                }
              }
            }
          }
          return true;
        }

        virtual bool run(MiniMC::Model::Program& prgm) {
          for (auto& F : prgm.getFunctions()) {
            this->runFunction(F);
          }
          return true;
        }
      };

      struct KillUnneededBranching : public MiniMC::Support::Sink<MiniMC::Model::Program> {
        KillUnneededBranching() {}
        virtual bool run(MiniMC::Model::Program& prgm);
      };

    } // namespace Modifications
  }   // namespace Model
} // namespace MiniMC

#endif
