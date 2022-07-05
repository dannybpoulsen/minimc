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
          /*for (auto& E : F->getCFA().getEdges()) {
            if (E->getGuard ()) {
              auto& guard = E->getGuard ().get ();
              assert(!E->getInstructions ());
              MiniMC::Model::InstructionStream m;
              if (guard.negate) {
                m.addInstruction<MiniMC::Model::InstructionCode::NegAssume>({.expr = guard.guard});
              }

              else {
		m.addInstruction<MiniMC::Model::InstructionCode::Assume>({.expr = guard.guard});
	      }
	      E->getInstructions () = m;
              
              E->getGuard ().unset ();
              E->getFrom()->getInfo().template set<MiniMC::Model::Attributes::AssumptionPlace>();
            }
	    }*/
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
