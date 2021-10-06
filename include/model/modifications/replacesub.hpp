/**
 * @file   replace.hpp
 *
 * @brief  
 * 
 * 
 */
#ifndef _REPLACE_SUBS__
#define _REPLACE_SUB__

#include "model/cfg.hpp"
#include "support/sequencer.hpp"


namespace MiniMC {
  namespace Model {
    namespace Modifications {
      struct ReplaceSub : public MiniMC::Support::Sink<MiniMC::Model::Program> {
	
	
	
		virtual bool run (MiniMC::Model::Program&  prgm) {
		  auto& cfac = prgm.getConstantFactory () ;
		  for (auto& F : prgm.getFunctions ()) {
			for (auto& E : F->getCFG()->getEdges ()) {
			  if (E->hasAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
				auto& instrstr = E->getAttribute<MiniMC::Model::AttributeType::Instructions> ();
				auto it = instrstr.begin();
				auto end = instrstr.end();
				for (it; it!=end;++it) {
				  if (it->getOpcode () == MiniMC::Model::InstructionCode::Sub) {
					std::cerr << "Replace " << std::endl;
					MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::Sub> helper (*it);
					auto nvar = F->getVariableStackDescr ()->addVariable ("",helper.getLeftOp()->getType());
					auto one_constant = cfac->makeIntegerConstant (1,helper.getLeftOp()->getType());
					std::vector<MiniMC::Model::Instruction> vec;
					vec.push_back (MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::Not>{}.setOP (helper.getRightOp ())
								   .setRes (nvar).
								   BuildInstruction ());
					vec.push_back (MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::Add>{}.setRes(nvar)
								   .setLeft (nvar)
								   .setRight (one_constant)
								   .BuildInstruction ());
					vec.push_back (MiniMC::Model::InstBuilder<MiniMC::Model::InstructionCode::Add>{}.setRes(helper.getResult())
								   .setLeft (helper.getLeftOp())
								   .setRight (nvar)
								   .BuildInstruction ());
		    
					it = instrstr.replaceInstructionBySeq (it,vec.begin(),vec.end());
					end = instrstr.end() ; 
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
