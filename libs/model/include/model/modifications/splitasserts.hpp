/**
 * @file   splitasserts.hpp
 * @date   Mon Apr 20 17:09:39 2020
 * 
 * @brief  
 * 
 * 
 */
#ifndef _SPLITASSERTS__
#define _SPLITASSERT__

#include <algorithm>

#include "model/cfg.hpp"
#include "support/sequencer.hpp"
#include "support/workinglist.hpp"

namespace MiniMC {
  namespace Model {
    namespace Modifications {
      /**
	   *
	   * For each \ref MiniMC::Model::InstructionCode::Assert
	   * instruction, a special Location with \ref
	   * MiniMC::Location::Attributes::AssertViolated set is inserted.
	   * The edge containing the Instruction is split, such that the
	   * one edge goes to the current successor Location while one goes newly
	   * constructed Location.  
	   * In this way exploration algorithms do not need to special
	   * handling for \ref  MiniMC::Model::InstructionCode::Assert
	   * instructions, but can instead for locations with \ref
	   * MiniMC::Location::Attributes::AssertViolated set.
	   */
      struct SplitAsserts : public MiniMC::Support::Sink<MiniMC::Model::Program> {
        virtual bool runFunction(const MiniMC::Model::Function_ptr& F);
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
