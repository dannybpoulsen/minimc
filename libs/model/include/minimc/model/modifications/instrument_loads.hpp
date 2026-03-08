#ifndef _INSTRUMENTS_LOADS__
#define _INSTRUMENTS_LOADS__

#include <algorithm>

#include "minimc/model/cfg.hpp"
#include "minimc/model/modifications/modifications.hpp"

namespace MiniMC {
  namespace Model {
    namespace Modifications {
      struct InstrumentLoads : public ProgramModifier {
	virtual bool runFunction(const MiniMC::Model::Function_ptr& F);
	MiniMC::Model::Program operator() (MiniMC::Model::Program&& prgm) override {
          for (auto& F : prgm.getFunctions()) {
            runFunction(F);
          }
          return prgm;
        }
      };

    } // namespace Modifications
  }   // namespace Model
} // namespace MiniMC

#endif
