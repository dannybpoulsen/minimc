#ifndef _LOOPS_UNROLL__
#define _LOOPS_UNROLL__

#include "model/analysis/manager.hpp"
#include "model/cfg.hpp"
#include "support/sequencer.hpp"

namespace MiniMC {
  namespace Model {
    namespace Modifications {
      
      struct UnrollLoops {
        virtual bool runFunction(const MiniMC::Model::Function_ptr&,std::size_t amount);
	
      };

    } // namespace Modifications
  }   // namespace Model
} // namespace MiniMC

#endif
