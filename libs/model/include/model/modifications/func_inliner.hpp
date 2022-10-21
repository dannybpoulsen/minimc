/**
 * @file   func_inliner.hpp
 * @date   Mon Apr 20 17:04:05 2020
 * 
 * @brief  
 * 
 * 
 */
#ifndef _funcinliner__
#define _funcinliner__

#include "model/cfg.hpp"
#include "support/sequencer.hpp"

namespace MiniMC {
  namespace Model {
    namespace Modifications {

      struct InlineFunctions  {
        InlineFunctions(MiniMC::Model::Program& prgm) : prgm(prgm) {}
        virtual bool runFunction(const MiniMC::Model::Function_ptr& F,std::size_t depth);
        
      private:
	MiniMC::Model::Program& prgm;
      };

    } // namespace Modifications
  }   // namespace Model
} // namespace MiniMC

#endif
