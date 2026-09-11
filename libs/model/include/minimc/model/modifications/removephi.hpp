#ifndef _REPLACEPHI__
#define _REPLACEPHI__

#include <unordered_map>
#include <unordered_set>

#include "minimc/model/program.hpp"
#include "minimc/model/instructions.hpp"
#include "minimc/model/modifications/modifications.hpp"
#include "minimc/support/overload.hpp"

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
                auto replacefunc = [&](MiniMC::Model::Register& reg)->MiniMC::Model::Value_ptr {
		  if (replacemap.count(&reg) > 0) {
		    return replacemap.at(&reg);
		  }
		  else 
		    return nullptr; 
		};
		if (E->isPhi ()) {
		  for (auto& inst : instrstream) {
                    auto& content = inst.getAs<VMInstructionCode::Assign>().getOps();

                    MiniMC::Model::visitSubExpressions(MiniMC::Support::Overload{
                                                            [&used](MiniMC::Model::Register& reg) { used.insert(&reg); },
                                                            [&used](MiniMC::Model::SymbolicConstant& reg) {
                                                              if (std::holds_alternative<MiniMC::Model::Register_ptr>(reg.getValue().getUserData())) {
                                                                used.insert(std::get<MiniMC::Model::Register_ptr>(reg.getValue().getUserData()).get());
                                                              }
                                                            },
                                                            MiniMC::Support::Ignore{}},
		      *content.op1
		      );
		      
        	  }

		  for (auto& inst : instrstream) {
		    
		    auto& content = inst.getAs<VMInstructionCode::Assign>().getOps ();
		    if (used.count(content.res.get())) {
		      auto nvar = prgm.getMetaRegs().addRegister( frame.makeFresh ("Phi"), content.res->getType());
		      replacemap.insert(std::make_pair(content.res.get(), nvar));
		      stream.add<MiniMC::Model::VMInstructionCode::Assign>(replacemap.at(content.res.get()), content.res);
		    }
		    
		  }
		  MiniMC::Model::Replacer replacer {replacefunc};
                  for (auto& inst : instrstream) {
                    auto& content = inst.getAs<VMInstructionCode::Assign>().getOps ();
		    
                    auto val = replacer.Replace(content.op1);
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
