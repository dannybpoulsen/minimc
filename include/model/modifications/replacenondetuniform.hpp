/**
 * @file   replacenondetuniform.hpp
 * @author Danny Bøgsted Poulsen <caramon@homemachine>
 * @date   Mon Apr 20 17:05:24 2020
 * 
 * @brief  
 * 
 * 
 */
#ifndef _NONDETTOUNIFORM__
#define _NONDETTOUNIFORM__

#include <limits>

#include "model/cfg.hpp"
#include "model/variables.hpp"
#include "support/sequencer.hpp"
#include "host/types.hpp"

namespace MiniMC {
  namespace Model {
    namespace Modifications {
      /**
	   * This pass will replace all \ref MiniMC::Model::InstructionCode::NonDet  Instruction by \ref MiniMC::Model::InstructionCode::Uniform.
	   */
      struct ReplaceNonDetUniform : public MiniMC::Support::Sink<MiniMC::Model::Program> {
        virtual bool run(MiniMC::Model::Program& prgm) {
          for (auto& F : prgm.getFunctions()) {
            for (auto& E : F->getCFA().getEdges()) {
              if (E->hasAttribute<MiniMC::Model::AttributeType::Instructions>()) {
                for (auto& I : E->getAttribute<MiniMC::Model::AttributeType::Instructions>()) {
                  if (I.getOpcode() == MiniMC::Model::InstructionCode::NonDet) {
		    
                    auto& content = I.getOps<MiniMC::Model::InstructionCode::NonDet> ();
		    I.replace(createInstruction<InstructionCode::Uniform> ({
			  .res = content.res,
			  .min = content.min,
			  .max = content.max
			}));
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
