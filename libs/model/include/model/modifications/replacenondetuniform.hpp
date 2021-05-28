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
#include "support/types.hpp"



namespace MiniMC {
  namespace Model {
    namespace Modifications {
	  /**
	   * This pass will replace all \ref MiniMC::Model::InstructionCode::NonDet  Instruction by \ref MiniMC::Model::InstructionCode::Uniform.
	   */
	  struct ReplaceNonDetUniform : public MiniMC::Support::Sink<MiniMC::Model::Program> {
		virtual bool run (MiniMC::Model::Program&  prgm) {
		  auto& fact = prgm.getConstantFactory ();
		  for (auto& F : prgm.getFunctions ()) {
			for (auto& E : F->getCFG()->getEdges ()) {
			  if (E->hasAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
				for (auto& I : E->getAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
				  if (I.getOpcode () == MiniMC::Model::InstructionCode::NonDet ) {
		    
					MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::Uniform> uniform;
					MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::NonDet> nondet (I);	  
					uniform.setResult (nondet.getResult ());
					uniform.setMin (nondet.getMin ());
					uniform.setMax (nondet.getMax ());
					I.replace (uniform.BuildInstruction ());
				  }
				}
			  }
			}
		  }
		  return true;
		}
      };

    }
  }
}


#endif
