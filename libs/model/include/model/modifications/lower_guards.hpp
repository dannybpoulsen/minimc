/**
 * @file   lower_guards.hpp
 * @date   Mon Apr 20 17:04:05 2020
 * 
 * @brief  
 * 
 * 
 */
#ifndef _lowerguards__
#define _lowerguards__

#include "model/cfg.hpp"
#include "support/sequencer.hpp"

namespace MiniMC {
  namespace Model {
    namespace Modifications {
	  /**
	   * LowerGuards converts Guards to Assume/NegAssume Instructions to simplfiy the analysis algorithms later on.
	   * It will also mark the source location as an \ref MiniMC::Model:::Location::Attributes::AssumptionPlace.
	   *
	   *
	   */
      struct LowerGuards : public MiniMC::Support::Sink<MiniMC::Model::Program> {
		template<MiniMC::Model::InstructionCode i,class Inserter>
		void add (MiniMC::Model::Guard& g,Inserter& inserter) {
		  MiniMC::Model::InstBuilder<i> builder;
		  builder.setAssert (g.guard);
		  inserter = builder.BuildInstruction ();
		}
		
		virtual bool run (MiniMC::Model::Program&  prgm) {
		  for (auto& F : prgm.getFunctions ()) {
			for (auto& E : F->getCFG()->getEdges ()) {
			  if (E->hasAttribute<MiniMC::Model::AttributeType::Guard> ()) {
				auto& guard = E->getAttribute<MiniMC::Model::AttributeType::Guard> ();
				assert(!E->hasAttribute<MiniMC::Model::AttributeType::Instructions> ());
				MiniMC::Model::InstructionStream m;
				E->setAttribute<MiniMC::Model::AttributeType::Instructions> (m);
				auto& instr = E->getAttribute<MiniMC::Model::AttributeType::Instructions> ();
				auto inserter = instr.back_inserter ();
				if (guard.negate) {
				  add<MiniMC::Model::InstructionCode::NegAssume> (guard,inserter);
				}
				
				else {
				  add<MiniMC::Model::InstructionCode::Assume> (guard,inserter);
				}
				E->delAttribute<MiniMC::Model::AttributeType::Guard> ();
				E->getFrom ()->getInfo().template set<MiniMC::Model::Attributes::AssumptionPlace> ();
				
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
