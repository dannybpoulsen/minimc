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
        
        virtual bool runFunction(const MiniMC::Model::Function_ptr& F) {
          for (auto& E : F->getCFA().getEdges()) {
            if (E->hasAttribute<MiniMC::Model::AttributeType::Guard>()) {
              auto& guard = E->getAttribute<MiniMC::Model::AttributeType::Guard>();
              assert(!E->hasAttribute<MiniMC::Model::AttributeType::Instructions>());
              MiniMC::Model::InstructionStream m;
              E->setAttribute<MiniMC::Model::AttributeType::Instructions>(m);
              auto& instr = E->getAttribute<MiniMC::Model::AttributeType::Instructions>();
              if (guard.negate) {
                instr.addInstruction<MiniMC::Model::InstructionCode::NegAssume>({.expr = guard.guard});
              }

              else {
		instr.addInstruction<MiniMC::Model::InstructionCode::Assume>({.expr = guard.guard});
	      }
              E->delAttribute<MiniMC::Model::AttributeType::Guard>();
              E->getFrom()->getInfo().template set<MiniMC::Model::Attributes::AssumptionPlace>();
            }
          }
          return true;
        }

        virtual bool run(MiniMC::Model::Program& prgm) {
          for (auto& F : prgm.getFunctions()) {
            runFunction(F);
          }
          return true;
        }
      };

    } // namespace Modifications
  }   // namespace Model
} // namespace MiniMC

#endif
