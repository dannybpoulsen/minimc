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

      struct InlineFunctions : public MiniMC::Support::Sink<MiniMC::Model::Program> {
        InlineFunctions(std::size_t d) : depth(d) {}
        virtual bool runFunction(const MiniMC::Model::Function_ptr& F);
        virtual bool run(MiniMC::Model::Program& prgm);

      private:
        std::size_t depth = 0;
      };

    } // namespace Modifications
  }   // namespace Model
} // namespace MiniMC

#endif
