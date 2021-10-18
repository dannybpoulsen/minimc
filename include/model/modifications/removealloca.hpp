/**
 * @file   insertboolcasts.hpp
 * @date   Mon Apr 20 17:03:54 2020
 * 
 * @brief  
 * 
 * 
 */
#ifndef _REMOVE_ALLOCAS__
#define _REMOVE_ALLOCAS__

#include "model/cfg.hpp"
#include "support/sequencer.hpp"

namespace MiniMC {
  namespace Model {
    namespace Modifications {
      struct RemoveAllocas : public MiniMC::Support::Sink<MiniMC::Model::Program> {

        virtual bool run(MiniMC::Model::Program& prgm);
      };

    } // namespace Modifications
  }   // namespace Model
} // namespace MiniMC

#endif
