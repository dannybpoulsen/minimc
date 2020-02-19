#ifndef _NONDETTOUNIFORM__
#define _NONDETTOUNIFORM__

#include <limits>

#include "model/cfg.hpp"
#include "model/variables.hpp"
#include "support/sequencer.hpp"
#include "support/types.hpp"



namespace MiniMC {
  namespace Model {
    namespace Modifications {
      struct ReplaceNonDetUniform : public MiniMC::Support::Sink<MiniMC::Model::Program> {
	virtual bool run (MiniMC::Model::Program&  prgm) {
	  auto& fact = prgm.getConstantFactory ();
	  for (auto& F : prgm.getFunctions ()) {
	    for (auto& E : F->getCFG()->getEdges ()) {
	      if (E->hasAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
		for (auto& I : E->getAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
		  if (I.getOpcode () == MiniMC::Model::InstructionCode::NonDet ) {
		    
		    MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::Uniform> uniform;
		    MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::NonDet> nondet (I);	  
		    auto type = nondet.getResult ()->getType ();
		    MiniMC::uint64_t min = 0;
		    MiniMC::uint64_t max = 0;
		    if (type->getTypeID () == MiniMC::Model::TypeID::Integer) {
		      
		      
		      switch (type->getSize()) {
		      case 1:
			min =std::numeric_limits<MiniMC::uint8_t>::min ();
			max =std::numeric_limits<MiniMC::uint8_t>::max ();
			break;
		      case 2:
			min =std::numeric_limits<MiniMC::uint16_t>::min ();
			max =std::numeric_limits<MiniMC::uint16_t>::max ();
			break;
		      case 4:
			min =std::numeric_limits<MiniMC::uint32_t>::min ();
			max =std::numeric_limits<MiniMC::uint32_t>::max ();
			break;
		      case 8:
			min =std::numeric_limits<MiniMC::uint64_t>::min ();
			max =std::numeric_limits<MiniMC::uint64_t>::max ();
			break;
		      default:
			assert(false);
		      }
		    }
		    auto minC = fact->makeIntegerConstant (min);
		    auto maxC = fact->makeIntegerConstant (max);
		    minC->setType (type);
		    maxC->setType (type);
		    uniform.setResult (nondet.getResult ());
		    uniform.setMin (minC);
		    uniform.setMax (maxC);
		    I.replace (uniform.BuildInstruction ());
		  }
		}
	      }
	    }
	  }
	  return true;
	}
      };

    }
  }
}


#endif
