#ifndef _HasInstruction__
#define _HasInstruction__

#include "model/cfg.hpp"
#include "support/feedback.hpp"
#include "support/localisation.hpp"
#include "support/sequencer.hpp"

namespace MiniMC {
  namespace Model {
    namespace Checkers {

      template <MiniMC::Model::InstructionCode... codes>
      struct HasNoInstruction : public MiniMC::Support::Sink<MiniMC::Model::Program> {
        HasNoInstruction(const std::string& s) : error_mess(s) {}
        virtual bool run(const MiniMC::Model::Program& prgm) {
          for (auto& F : prgm.getFunctions()) {
            for (auto& E : F->getCFA().getEdges()) {
              if (E->getInstructions ()) {
                for (auto& I : E->getInstructions()) {
                  if (MiniMC::Model::isOneOf<codes...>(I)) {
		    MiniMC::Support::Messager{}.message (error_mess.format(I.getOpcode()));
                    return false;
                  }
                }
              }
            }
          }
          return true;
        }

        MiniMC::Support::Localiser error_mess;
      };
    } // namespace Checkers
  }   // namespace Model
} // namespace MiniMC

#endif
