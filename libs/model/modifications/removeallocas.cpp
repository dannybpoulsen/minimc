#include "model/modifications/removealloca.hpp"
namespace MiniMC {
  namespace Model {
    namespace Modifications {
      bool RemoveAllocas::run(MiniMC::Model::Program& prgm) {
          auto source_loc = std::make_shared<MiniMC::Model::SourceInfo>();
          for (auto& F : prgm.getFunctions()) {
            auto stackvar = F->getRegisterStackDescr().addRegister("stack", prgm.getTypeFactory()->makePointerType());
            auto& cfg = F->getCFA();
            auto size = prgm.getConstantFactory()->makeIntegerConstant(0, prgm.getTypeFactory()->makeIntegerType(64));
            InstructionStream stream({createInstruction<InstructionCode::FindSpace>({.res = stackvar, .op1 = size})});
	    //createInstruction<InstructionCode::Malloc>({.object = stackvar, .size = size})}
		

            auto ninitloc = cfg.makeLocation(MiniMC::Model::LocationInfo("StackAlloc", 0, *source_loc));
            auto oinitloc = cfg.getInitialLocation();
            auto edge = cfg.makeEdge(ninitloc, oinitloc);
            edge->template setAttribute<AttributeType::Instructions>(stream);
            cfg.setInitial(ninitloc);

            for (auto& E : F->getCFA().getEdges()) {
              if (E->hasAttribute<MiniMC::Model::AttributeType::Instructions>()) {
                for (auto& I : E->getAttribute<MiniMC::Model::AttributeType::Instructions>()) {
                  if (I.getOpcode() == InstructionCode::Alloca) {
		    auto& content = I.getOps<InstructionCode::Alloca> ();
                    I.replace(createInstruction<InstructionCode::ExtendObj>(
                        {.res = content.res,
                         .object = stackvar,
                         .size = content.op1}));
                  }
                }
                auto& instrstream = E->getAttribute<MiniMC::Model::AttributeType::Instructions>();
                if (instrstream.last().getOpcode() == InstructionCode::Ret) {
                  auto retinstr = createInstruction<InstructionCode::Ret> ({.value = instrstream.last().getOps<InstructionCode::Ret>().value});
                  instrstream.last().replace(createInstruction<InstructionCode::Free>({.object = stackvar}));
                  instrstream.addInstruction (retinstr);
		  
                }

                else if (instrstream.last().getOpcode() == InstructionCode::RetVoid) {

                  instrstream.last().replace(createInstruction<InstructionCode::Free>({.object = stackvar}));
                  instrstream.addInstruction<InstructionCode::RetVoid> (0);
                }
              }
            }
          }
          return true;
        }
    }
  }
} // namespace MiniMC
