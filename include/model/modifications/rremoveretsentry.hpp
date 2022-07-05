/**
 * @file   rremoveretsentry.hpp
 * @date   Mon Apr 20 17:09:12 2020
 * 
 * @brief  
 * 
 * 
 */
#ifndef _removeretsentry__
#define _removeretsentry__

#include "model/cfg.hpp"
#include "support/sequencer.hpp"

namespace MiniMC {
  namespace Model {
    namespace Modifications {
      /** Replaces return instructions by Skip instructions in entry
	  points. By doing so  we avoid managing special cases (in
	  exploration algorithms) when an entry point terminates. 
      */
      struct RemoveRetEntryPoints : public MiniMC::Support::Sink<MiniMC::Model::Program> {
        virtual bool run(MiniMC::Model::Program& prgm) {
          for (auto& F : prgm.getEntryPoints()) {
            for (auto& E : F->getCFA().getEdges()) {
              if (E->getInstructions () ) {
                for (auto& I : E->getInstructions ()) {
                  if (I.getOpcode() == MiniMC::Model::InstructionCode::Ret ||
                      I.getOpcode() == MiniMC::Model::InstructionCode::RetVoid) {
                    I.replace(createInstruction<InstructionCode::Skip> (0));
                  }
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
