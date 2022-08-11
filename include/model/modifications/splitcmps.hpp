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

          auto& cfg = F->getCFA();
          MiniMC::Support::WorkingList<MiniMC::Model::Edge_ptr> wlist;
          auto inserter = wlist.inserter();
          std::for_each(cfg.getEdges().begin(),
                        cfg.getEdges().end(),
                        [&](const MiniMC::Model::Edge_ptr& e) { inserter = e; });

          for (auto E : wlist) {
            if (E->getInstructions ()) {
              auto& instrs = E->getInstructions ();
              if (isCMP(instrs.last())) {
                auto buildEdge = [&]<MiniMC::Model::InstructionCode inst,
                                     MiniMC::Model::InstructionCode left>(auto& v) {
                  auto& origcontent = instrs.last().getOps<inst> ();
                  auto loc = cfg.makeLocation(E->getTo()->getInfo());

                  auto it = E->getTo()->ebegin();
                  auto end = E->getTo()->eend();

                  for (; it != end; ++it) {
                    auto nedge = cfg.makeEdge(loc, (*it)->getTo());
                    nedge->copyAttributesFrom(**it);
                  }
                  std::vector<MiniMC::Model::Instruction> instr;
                  std::copy(instrs.begin(), instrs.end(), std::back_inserter(instr));
                  auto tt = cfg.makeEdge(E->getFrom(), loc);
                  tt->getInstructions () = instr;
                  std::vector<MiniMC::Model::Instruction> ttI{
		    MiniMC::Model::createInstruction<left>({.op1 = origcontent.op1, .op2 = origcontent.op2}),
		    MiniMC::Model::createInstruction<MiniMC::Model::InstructionCode::Assign>({.res = origcontent.res, .op1 = v})
		  };
                  auto& llnew = tt->getInstructions ();
                  llnew.replaceInstructionBySeq(llnew.end() - 1, ttI.begin(), ttI.end());
                };
                auto& cfac = F->getPrgm().getConstantFactory();
                auto tt = cfac.makeIntegerConstant(1, TypeID::Bool);
                auto ff = cfac.makeIntegerConstant(0, TypeID::Bool);

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
		default:
		  throw MiniMC::Support::Exception ("Not a  comparison");
		};
                if (E->getTo()->nbIncomingEdges() <= 1) {
                  cfg.deleteLocation(E->getTo());
                } else {
                  cfg.deleteEdge(E.get());
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
