/**
 * @file   splitasserts.hpp
 * @date   Mon Apr 20 17:09:39 2020
 * 
 * @brief  
 * 
 * 
 */
#ifndef _SPLITASSERTS__
#define _SPLITASSERTS__

#include <algorithm>

#include "minimc/model/cfg.hpp"
#include "minimc/model/modifications/modifications.hpp"

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
      struct SplitAsserts : public ProgramModifier {
	virtual bool runFunction(const MiniMC::Model::Function_ptr& F);
	MiniMC::Model::Program operator() (MiniMC::Model::Program&& prgm) override {
          for (auto& F : prgm.getFunctions()) {
            runFunction(F);
          }
          return prgm;
        }
      };

    } // namespace Modifications
  }   // namespace Model
} // namespace MiniMC

#endif
