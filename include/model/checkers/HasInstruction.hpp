#ifndef _HasInstruction__
#define _HasInstruction__

#include "model/cfg.hpp"
#include "support/sequencer.hpp"
#include "support/feedback.hpp"
#include "support/localisation.hpp"

namespace MiniMC {
  namespace Model {
    namespace Checkers {

	  template<MiniMC::Model::InstructionCode... codes>
	  struct HasNoInstruction : public MiniMC::Support::Sink<MiniMC::Model::Program> {
		HasNoInstruction (MiniMC::Support::Messager& ptr,const std::string& s) : mess (ptr),error_mess(s) {}
		virtual bool run (const MiniMC::Model::Program&  prgm) {
		  for (auto& F : prgm.getFunctions ()) {
			for (auto& E : F->getCFG()->getEdges ()) {
			  if (E->hasAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
				for (auto& I : E->getAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
				  if (MiniMC::Model::isOneOf<codes...> (I)) { 
					mess.error (error_mess.format (I.getOpcode ()));
					return false;
				  }
				}

			  }
			}
		  }
		  return true;
		}
	  private:
		MiniMC::Support::Messager& mess;
		MiniMC::Support::Localiser error_mess;
		
	  };
	}
  }
}

#endif
