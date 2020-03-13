#ifndef _MARKLOOPLOCS__
#define _MARKLOOPLOCS__

#include "model/cfg.hpp"
#include "support/sequencer.hpp"


namespace MiniMC {
  namespace Model {
    namespace Modifications {
      struct MarkLoopStates : public MiniMC::Support::Sink<MiniMC::Model::Program> {
		virtual bool run (MiniMC::Model::Program&  prgm);
	  };
		
	}
  }
}


#endif

