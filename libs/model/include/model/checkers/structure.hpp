/**
 * @file   structure.hpp
 * @author Danny Bøgsted Poulsen <caramon@homemachine>
 * @date   Mon Apr 20 14:37:50 2020
 * 
 * @brief  
 * 
 * 
 */

#ifndef _STRUCTURECHECK__
#define _STRUCTURECHECK__

#include "model/cfg.hpp"
#include "support/feedback.hpp"
#include "support/sequencer.hpp"

namespace MiniMC {
  namespace Model {
    namespace Checkers {

      /**
	   * Run structural checks of the CFG such as whether Call instructions are the last instruction. 
	   * Look at 
	   * - \ref MiniMC::Model::Checkers::checkNoGuardAndInstructionStream()
	   * - \ref MiniMC::Model::Checkers::checkGuardIsBoolean()
	   * - \ref MiniMC::Model::Checkers::checkCallIsLast()
	   * - \ref MiniMC::Model::Checkers::checkPhiIsOnlyAssign()
	   * - \ref MiniMC::Model::Checkers::checkAssertIsLast()
	   * for the actual checks 
	   */
      struct StructureChecker : public MiniMC::Support::Sink<MiniMC::Model::Program> {
        StructureChecker()  {}
        virtual bool run(MiniMC::Model::Program& prgm);
      };

    } // namespace Checkers
  }   // namespace Model
} // namespace MiniMC

#endif
