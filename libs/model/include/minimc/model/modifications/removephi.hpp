#ifndef _REPLACEPHI__
#define _REPLACEPHI__

#include <unordered_map>
#include <unordered_set>

#include "minimc/model/cfg.hpp"
#include "minimc/model/instructions.hpp"
#include "minimc/model/modifications/modifications.hpp"

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
		std::unordered_set<MiniMC::Model::Value*> used;
               
		if (E->isPhi ()) {
		  for (auto& inst : instrstream) {
		    auto& content = inst.getAs<VMInstructionCode::Assign>().getOps ();
                    used.insert (content.op1.get());
		  }
		  for (auto& inst : instrstream) {
		    
		    auto& content = inst.getAs<VMInstructionCode::Assign>().getOps ();
		    if (used.count(content.res.get())) {
		      auto nvar = prgm.getMetaRegs().addRegister( frame.makeFresh ("Phi"), content.res->getType());
		      replacemap.insert(std::make_pair(content.res.get(), nvar));
		      stream.add<MiniMC::Model::VMInstructionCode::Assign>(replacemap.at(content.res.get()), content.res);
		    }
		    
		  }
		  
                  for (auto& inst : instrstream) {
                    auto& content = inst.getAs<VMInstructionCode::Assign>().getOps ();
		    
                    auto val = replacemap.count(content.op1.get()) ? replacemap.at(content.op1.get()) : content.op1;
                    stream.add<MiniMC::Model::VMInstructionCode::Assign>(content.res, val);
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
