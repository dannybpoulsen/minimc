/**
 * @file   insertboolcasts.hpp
 * @date   Mon Apr 20 17:03:54 2020
 * 
 * @brief  
 * 
 * 
 */
#ifndef _REMOVE_ALLOCAS__
#define _REMOVE_ALLOCAS__

#include "model/cfg.hpp"
#include "support/sequencer.hpp"

namespace MiniMC {
  namespace Model {
    namespace Modifications {
      struct RemoveAllocas : public MiniMC::Support::Sink<MiniMC::Model::Program> {

        virtual bool run(MiniMC::Model::Program& prgm) {
          auto source_loc = std::make_shared<MiniMC::Model::SourceInfo>();
          for (auto& F : prgm.getFunctions()) {
            auto stackvar = F->getVariableStackDescr()->addVariable("stack", prgm.getTypeFactory()->makePointerType());
            auto cfg = F->getCFG();
            InstBuilder<InstructionCode::FindSpace> instr;
            InstBuilder<InstructionCode::Malloc> minstr;

            instr.setResult(stackvar);
            instr.setSize(prgm.getConstantFactory()->makeIntegerConstant(0, prgm.getTypeFactory()->makeIntegerType(64)));
            minstr.setPointer(stackvar);
            minstr.setSize(prgm.getConstantFactory()->makeIntegerConstant(0, prgm.getTypeFactory()->makeIntegerType(64)));
            InstructionStream stream({instr.BuildInstruction(), minstr.BuildInstruction()});

            auto ninitloc = cfg->makeLocation(MiniMC::Model::LocationInfo("StackAlloc", 0, *source_loc));
            auto oinitloc = cfg->getInitialLocation().get();
            auto edge = cfg->makeEdge(ninitloc, oinitloc);
            edge->template setAttribute<AttributeType::Instructions>(stream);
            cfg->setInitial(ninitloc);

            for (auto& E : F->getCFG()->getEdges()) {
              if (E->hasAttribute<MiniMC::Model::AttributeType::Instructions>()) {
                for (auto& I : E->getAttribute<MiniMC::Model::AttributeType::Instructions>()) {
                  if (I.getOpcode() == InstructionCode::Alloca) {
                    InstBuilder<InstructionCode::ExtendObj> instr;
                    InstHelper<InstructionCode::Alloca> helper(I);

                    instr.setRes(helper.getResult());
                    instr.setSize(helper.getSize());
                    instr.setPointer(stackvar);
                    I.replace(instr.BuildInstruction());
                  }
                }
                auto& instrstream = E->getAttribute<MiniMC::Model::AttributeType::Instructions>();
                if (instrstream.last().getOpcode() == InstructionCode::Ret) {
                  InstBuilder<InstructionCode::Free> freebuilder;
                  InstBuilder<InstructionCode::Ret> retbuilder;

                  freebuilder.setPointer(stackvar);
                  retbuilder.setRetValue(instrstream.last().getOp(0));
                  instrstream.last().replace(freebuilder.BuildInstruction());
                  instrstream.back_inserter() = retbuilder.BuildInstruction();

                }

                else if (instrstream.last().getOpcode() == InstructionCode::RetVoid) {
                  InstBuilder<InstructionCode::Free> freebuilder;
                  InstBuilder<InstructionCode::RetVoid> retbuilder;

                  freebuilder.setPointer(stackvar);
                  instrstream.last().replace(freebuilder.BuildInstruction());
                  instrstream.back_inserter() = retbuilder.BuildInstruction();
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
