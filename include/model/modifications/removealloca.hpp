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
            auto size = prgm.getConstantFactory()->makeIntegerConstant(0, prgm.getTypeFactory()->makeIntegerType(64));
            InstructionStream stream({createInstruction<InstructionCode::FindSpace>({.res = stackvar, .size = size}),
                                      createInstruction<InstructionCode::Malloc>({.object = stackvar, .size = size})});

            auto ninitloc = cfg->makeLocation(MiniMC::Model::LocationInfo("StackAlloc", 0, *source_loc));
            auto oinitloc = cfg->getInitialLocation().get();
            auto edge = cfg->makeEdge(ninitloc, oinitloc);
            edge->template setAttribute<AttributeType::Instructions>(stream);
            cfg->setInitial(ninitloc);

            for (auto& E : F->getCFG()->getEdges()) {
              if (E->hasAttribute<MiniMC::Model::AttributeType::Instructions>()) {
                for (auto& I : E->getAttribute<MiniMC::Model::AttributeType::Instructions>()) {
                  if (I.getOpcode() == InstructionCode::Alloca) {
                    InstHelper<InstructionCode::Alloca> helper(I);
                    I.replace(createInstruction<InstructionCode::ExtendObj>(
                        {.res = helper.getResult(),
                         .object = stackvar,
                         .size = helper.getSize()}));
                  }
                }
                auto& instrstream = E->getAttribute<MiniMC::Model::AttributeType::Instructions>();
                if (instrstream.last().getOpcode() == InstructionCode::Ret) {
                  auto retinstr = createInstruction<InstructionCode::Ret> ({.value = instrstream.last().getOps<InstructionCode::Ret>().value});
                  instrstream.last().replace(createInstruction<InstructionCode::Free>({.object = stackvar}));
                  instrstream.back_inserter() = retinstr;
		  
                }

                else if (instrstream.last().getOpcode() == InstructionCode::RetVoid) {

                  instrstream.last().replace(createInstruction<InstructionCode::Free>({.object = stackvar}));
                  instrstream.back_inserter() = createInstruction<InstructionCode::RetVoid> (0);
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
