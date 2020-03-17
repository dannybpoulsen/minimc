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

	  struct MarkAllStates : public MiniMC::Support::Sink<MiniMC::Model::Program> {
		virtual bool run (MiniMC::Model::Program&  prgm) {
		  for (auto& F : prgm.getFunctions ()) {
			for (auto& l : F->getCFG()->getLocations ()) {
			  l->template set <MiniMC::Model::Location::Attributes::NeededStore> ();
			}
		  }
		  return true;
		}
	  };
	}
  }
}


#endif

