#ifndef _LOOPS_UNROLL__
#define _LOOPS_UNROLL__

#include "model/cfg.hpp"
#include "support/sequencer.hpp"
#include "model/analysis/manager.hpp"


namespace MiniMC {
  namespace Model {
    namespace Modifications {
	  struct UnrollOptions {
		size_t maxAmount = 0;
	};

	  void unrollLoop (MiniMC::Model::CFG_ptr& cfg, const MiniMC::Model::Analysis::Loop_ptr& loop,std::size_t amount);
	  
	  struct UnrollLoops : public MiniMC::Support::Sink<MiniMC::Model::Program> {
	    UnrollLoops (	std::size_t size) : maxAmount (size) {} 
	    virtual bool run (MiniMC::Model::Program&  prgm);
	    virtual bool runFunction (const MiniMC::Model::Function_ptr&);
	  private:
	    size_t maxAmount = 0;
	  };
      
    }
  }
}


#endif
