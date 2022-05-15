
#include "model/cfg.hpp"
#include "model/instructions.hpp"
#include <iostream>

namespace MiniMC {
  namespace Model {
    namespace Modifications {
      template <MiniMC::Model::InstructionCode i>
      void foldInstr(MiniMC::Model::Instruction& instr, MiniMC::Model::Program& prgm) {
        if constexpr (i == MiniMC::Model::InstructionCode::IntToBool) {
          auto& content  = instr.getOps<i> ();
          auto res = content.res;
          auto val = content.op1;
          auto type = val->getType();

          if (val->isConstant()) {
            assert(type->isInteger() );
            auto cfac = prgm.getConstantFactory();
            auto tfac = prgm.getTypeFactory();
	    
            auto tt = cfac->makeIntegerConstant(1, TypeID::Bool);
            auto ff = cfac->makeIntegerConstant(0, TypeID::Bool);

            BV64 value = 0;
            switch (type->getTypeID()) {
	    case MiniMC::Model::TypeID::I8:
                value = std::static_pointer_cast<MiniMC::Model::TConstant<MiniMC::BV8>>(val)->getValue();
                break;
	    case MiniMC::Model::TypeID::I16:
                value = std::static_pointer_cast<MiniMC::Model::TConstant<MiniMC::BV16>>(val)->getValue();
                break;
	    case MiniMC::Model::TypeID::I32:
                value = std::static_pointer_cast<MiniMC::Model::TConstant<MiniMC::BV32>>(val)->getValue();
                break;
	    case MiniMC::Model::TypeID::I64:
                value = std::static_pointer_cast<MiniMC::Model::TConstant<MiniMC::BV64>>(val)->getValue();
                break;
	    default:
	      throw MiniMC::Support::Exception ("Error");
            }

	    MiniMC::Model::Value_ptr resval;
	    if (value) {
              resval = tt; 
            } else {
              resval = ff; 
            }
            instr.replace(createInstruction<InstructionCode::Assign> ( {
		  .res = res,
		  .op1 = resval
		})
	      );
          }
        }
      }

      void foldConstants(Function& F) {
        auto& prgm = F.getPrgm();
        auto& cfg = F.getCFA();
        for (auto& e : cfg.getEdges()) {
          if (e->hasAttribute<MiniMC::Model::AttributeType::Instructions>()) {
            for (auto& i : e->getAttribute<MiniMC::Model::AttributeType::Instructions>()) {
              switch (i.getOpcode()) {
#define X(INSTR)                                                \
  case MiniMC::Model::InstructionCode::INSTR:                   \
    foldInstr<MiniMC::Model::InstructionCode::INSTR>(i, prgm); \
    break;

                OPERATIONS
#undef X
              }
            }
          }
        }
      }
    } // namespace Modifications
  }   // namespace Model
} // namespace MiniMC
