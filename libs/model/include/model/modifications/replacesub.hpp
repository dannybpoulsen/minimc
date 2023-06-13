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
#include "support/overload.hpp"

namespace MiniMC {
  namespace Model {
    namespace Modifications {
      struct ReplaceSub : public MiniMC::Support::Sink<MiniMC::Model::Program> {

        virtual bool run(MiniMC::Model::Program& prgm) {
          auto& cfac = prgm.getConstantFactory();
          for (auto& F : prgm.getFunctions()) {
	    auto frame = F->getFrame ();
            for (auto& E : F->getCFA().getEdges()) {
              if (E->getInstructions ()) {
                auto& instrstr = E->getInstructions ();
                auto it = instrstr.begin();
                auto end = instrstr.end();
                for (; it != end; ++it) {
		  it->visit (MiniMC::Support::Overload {
		      [&F,&cfac,&instrstr,&it,&end,&frame](const MiniMC::Model::TInstruction<MiniMC::Model::InstructionCode::Sub>& instr) {
			auto& content = instr.getOps ();
			auto nvar = F->getRegisterDescr().addRegister(frame.makeFresh (), content.op1->getType());
			auto one_constant = cfac.makeIntegerConstant(1, content.op1->getType()->getTypeID ());
			std::vector<MiniMC::Model::Instruction> vec;
			vec.push_back(MiniMC::Model::Instruction::make<MiniMC::Model::InstructionCode::Not>(nvar, content.op2));
			vec.push_back(MiniMC::Model::Instruction::make<MiniMC::Model::InstructionCode::Add>(nvar, nvar, one_constant));
			vec.push_back(MiniMC::Model::Instruction::make<MiniMC::Model::InstructionCode::Add>(content.res, content.op1, nvar));

			it = instrstr.replaceInstructionBySeq(it, vec.begin(), vec.end());
			end = instrstr.end();
		      },
			[](auto&) {}
			}
		    );
                }
              }
            }
          }
          return true;
        }
      };

    } // namespace Modifications
  }   // namespace Model
} // namespace MiniMC

#endif
