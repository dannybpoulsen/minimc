#ifndef _STRUCTURECHECK__
#define _STRUCTURECHECK__

#include "minimc/model/cfg.hpp"
#include "minimc/support/feedback.hpp"

#include <memory>

namespace MiniMC {
  namespace Model {
    namespace Checkers {
      /**
	   * Typecheck the progrm \p prgm.
	   
	   */
      struct StructuralChecker  {
        bool Check(MiniMC::Model::Program&, MiniMC::Support::Messager = MiniMC::Support::Messager{});
	
      };
      
    } // namespace Checkers
  }   // namespace Model
} // namespace MiniMC

#endif
