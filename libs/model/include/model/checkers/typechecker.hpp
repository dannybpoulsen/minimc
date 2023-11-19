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

namespace MiniMC {
  namespace Model {
    namespace Checkers {
      /**
	   * Typecheck the progrm \p prgm.
	   
	   */
      struct TypeChecker  {
        TypeChecker(MiniMC::Model::Program& prgm) : prgm(prgm)  {}
	bool Check( MiniMC::Support::Messager = MiniMC::Support::Messager{});
	bool Check(MiniMC::Model::Function&, MiniMC::Support::Messager = MiniMC::Support::Messager{});
	
      private:
	MiniMC::Model::Program& prgm;
      };

    } // namespace Checkers
  }   // namespace Model
} // namespace MiniMC

#endif
 
