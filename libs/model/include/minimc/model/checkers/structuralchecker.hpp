#ifndef _STRUCTURECHECK__
#define _STRUCTURECHECK__

#include "minimc/model/function.hpp"
#include "minimc/support/feedback.hpp"

#include <memory>

namespace MiniMC {
  namespace Model {
    namespace Checkers {
      /**
	   * Typecheck the progrm \p prgm.
	   
	   */
      struct StructuralChecker  {
	StructuralChecker (MiniMC::Support::Messager messager= MiniMC::Support::Messager{}) : messager(messager) {}
        bool Check(MiniMC::Model::Program&);
      private:
	MiniMC::Support::Messager messager;
      };
      
    } // namespace Checkers
  }   // namespace Model
} // namespace MiniMC

#endif
