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
      struct TypeChecker  {
        TypeChecker()  {}
        virtual bool run(MiniMC::Model::Program& prgm, MiniMC::Support::Messager = MiniMC::Support::Messager{});
      };

    } // namespace Checkers
  }   // namespace Model
} // namespace MiniMC

#endif
