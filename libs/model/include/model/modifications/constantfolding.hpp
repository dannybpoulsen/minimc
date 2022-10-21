#ifndef _CONSTANT_FOLDING__
#define _CONSTANT_FOLDING__

#include "model/cfg.hpp"
#include "support/sequencer.hpp"

namespace MiniMC {
  namespace Model {
    namespace Modifications {
      void foldConstants(Function& f);

      struct FoldConstants : public MiniMC::Support::Sink<MiniMC::Model::Program> {
        virtual bool run(MiniMC::Model::Program& prgm) {
          for (auto& F : prgm.getFunctions()) {
            foldConstants(*F);
          }
          return true;
        }
      };
    } // namespace Modifications

  } // namespace Model
} // namespace MiniMC

#endif
