#ifndef _STRUCTURECHECK__
#define _STRUCTURECHECK__

#include "model/cfg.hpp"
#include "support/sequencer.hpp"
#include "support/feedback.hpp"

namespace MiniMC {
  namespace Model {
    namespace Checkers {
      struct StructureChecker : public MiniMC::Support::Sink<MiniMC::Model::Program> {
	StructureChecker (MiniMC::Support::Messager& ptr) : messager (ptr) {}
	virtual bool run (MiniMC::Model::Program&  prgm);
      private:
	MiniMC::Support::Messager& messager;
      };

    }
  }
}


#endif
