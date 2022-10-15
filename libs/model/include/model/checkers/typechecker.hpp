/**
 * @file   typechecker.hpp
 * @date   Mon Apr 20 17:02:30 2020
 * 
 * @brief  
 * 
 * 
 */
#ifndef _TYPECHECK__
#define _TYPECHECK__

#include "model/cfg.hpp"
#include "support/feedback.hpp"
#include "support/sequencer.hpp"

namespace MiniMC {
  namespace Model {
    namespace Checkers {
      /**
	   * Typecheck the progrm \p prgm.
	   
	   */
      struct TypeChecker : public MiniMC::Support::Sink<MiniMC::Model::Program> {
        TypeChecker()  {}
        virtual bool run(MiniMC::Model::Program& prgm);
      };

    } // namespace Checkers
  }   // namespace Model
} // namespace MiniMC

#endif
