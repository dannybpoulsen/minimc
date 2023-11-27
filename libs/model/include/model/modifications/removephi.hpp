#ifndef _REPLACEPHI__
#define _REPLACEPHI__

#include <unordered_map>

#include "model/cfg.hpp"
#include "model/instructions.hpp"
#include "model/modifications/modifications.hpp"
#include "support/sequencer.hpp"

namespace MiniMC {
  namespace Model {
    namespace Modifications {
      
      struct LowerPhi : public ProgramModifier {
	MiniMC::Model::Program operator() (MiniMC::Model::Program&& prgm) override {
	  for (auto& F : prgm.getFunctions()) {
            for (auto& E : F->getCFA().getEdges()) {
	      auto frame = prgm.getRootFrame ();
	      if (E->getInstructions () ) {
                auto& instrstream = E->getInstructions () ;
                InstructionStream stream;
                std::unordered_map<MiniMC::Model::Value*, MiniMC::Model::Register_ptr> replacemap;
                if (E->isPhi ()) {
                  for (auto& inst : instrstream) {
		    
		    auto& content = inst.getAs<InstructionCode::Assign>().getOps ();
                    auto nvar = prgm.getMetaRegs().addRegister( frame.makeFresh ("Phi"), content.res->getType());
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
          return prgm;
        }
      };

    } // namespace Modifications
  }   // namespace Model

} // namespace MiniMC

#endif
