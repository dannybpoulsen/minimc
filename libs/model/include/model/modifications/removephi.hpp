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
	      auto frame = F->getFrame ();
	      if (E->getInstructions () ) {
                auto& instrstream = E->getInstructions () ;
                InstructionStream stream;
                std::unordered_map<MiniMC::Model::Value*, MiniMC::Model::Register_ptr> replacemap;
                if (E->isPhi ()) {
                  for (auto& inst : instrstream) {
		    
		    auto& content = inst.getAs<InstructionCode::Assign>().getOps ();
                    auto nvar = F->getRegisterDescr().addRegister( frame.makeFresh ("Phi"), content.res->getType());
                    replacemap.insert(std::make_pair(content.res.get(), nvar));
		    stream.add<MiniMC::Model::InstructionCode::Assign>(replacemap.at(content.res.get()), content.res);
                  }
		  
                  for (auto& inst : instrstream) {
                    auto& content = inst.getAs<InstructionCode::Assign>().getOps ();

                    auto val = replacemap.count(content.op1.get()) ? replacemap.at(content.op1.get()) : content.op1;
                    stream.add<MiniMC::Model::InstructionCode::Assign>(content.res, val);
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
