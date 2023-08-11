#ifndef _LOOPS_UNROLL__
#define _LOOPS_UNROLL__

#include "model/modifications/modifications.hpp"
#include "model/cfg.hpp"

namespace MiniMC {
  namespace Model {
    namespace Modifications {
      
      struct UnrollLoops : public ProgramModifier {
	UnrollLoops (std::size_t amount) : amount(amount) {}
	MiniMC::Model::Program operator() (MiniMC::Model::Program&& prgm) override {
	  for (auto& f : prgm.getFunctions ())
	    runFunction (f);
	  return prgm;
	}
        virtual bool runFunction(const MiniMC::Model::Function_ptr&);
	private:
	  std::size_t amount;
      };

    } // namespace Modifications
  }   // namespace Model
} // namespace MiniMC

#endif
