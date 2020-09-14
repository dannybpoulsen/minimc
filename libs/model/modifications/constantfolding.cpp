
#include <iostream>
#include "model/instructions.hpp"
#include "model/modifications/constantfolding.hpp"

namespace MiniMC  {
  namespace Model {
	namespace Modifications {
	  template<MiniMC::Model::InstructionCode i>
	  void foldInstr (MiniMC::Model::Instruction& instr) {
		//Default - do nothing
	  }
	  
	  template<>
	  void foldInstr<MiniMC::Model::InstructionCode::IntToBool>  (MiniMC::Model::Instruction& instr, MiniMC::Program& prgm) {  
		assert(instr.getOpcode  () == MiniMC::Model::InstructionCode::IntToBool);
		MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::IntToBool> helper (instr);
		auto res = helper.getResult ();
		auto val = helper.getCastee ();
		auto type = val->getType ();
		
		if (val->isConstant ()) {
		  auto cfac = prgm->getConstantFactory ();
		  std::cerr << "Foldable" << std::endl;
		}
	  }
	  
	  void foldConstants (Function& F) {
		auto prgm = F.getProgram();
		auto cfg = F.getCFG ();
		for (auto& e : cfg->getEdges ()) {
		  if (e->hasAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
			for (auto& i : e->getAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
			  switch (i.getOpcode ()) {
#define X(INSTR)								\
				case MiniMC::Model::InstructionCode::INSTR:				\
				  foldInstr<MiniMC::Model::InstructionCode::INSTR> (i,*prgm); \
				  break; \

				OPERATIONS
#undef X
			  }
			}
		  }
		}
	  }
	}
  }
}
	  
