#ifndef _REPLACEPHI__
#define _REPLACEPHI__

#include <unordered_map>

#include "model/cfg.hpp"
#include "model/instructions.hpp"
#include "support/sequencer.hpp"

namespace MiniMC {
  namespace Model {
    namespace Modifications {

      struct LowerPhi : public MiniMC::Support::Sink<MiniMC::Model::Program> {
        virtual bool run(MiniMC::Model::Program& prgm) {
          for (auto& F : prgm.getFunctions()) {
            for (auto& E : F->getCFA().getEdges()) {
              if (E->getInstructions () ) {
                auto& instrstream = E->getInstructions () ;
                InstructionStream stream;
                std::unordered_map<MiniMC::Model::Value*, MiniMC::Model::Register_ptr> replacemap;
                if (instrstream.isPhi ()) {
                  for (auto& inst : instrstream) {
                    auto& content = inst.getOps<InstructionCode::Assign>();
                    auto nvar = F->getRegisterStackDescr().addRegister(
								       Symbol(std::static_pointer_cast<Register>( content.res)->getName() + "PHI-tmp"), content.res->getType());
                    replacemap.insert(std::make_pair(content.res.get(), nvar));

                    stream.addInstruction<MiniMC::Model::InstructionCode::Assign>({.res = replacemap.at(content.res.get()), .op1 = content.op1});
                  }

                  for (auto& inst : instrstream) {
                    auto& content = inst.getOps<InstructionCode::Assign>();

                    auto val = replacemap.count(content.op1.get()) ? replacemap.at(content.op1.get()) : content.op1;
                    stream.addInstruction<MiniMC::Model::InstructionCode::Assign>({.res = replacemap.at(content.res.get()), .op1 = val});
                  }

		  E->getInstructions () = stream;
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
