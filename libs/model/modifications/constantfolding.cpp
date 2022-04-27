
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
	    
            auto tt = cfac->makeIntegerConstant(1, tfac->makeBoolType());
            auto ff = cfac->makeIntegerConstant(0, tfac->makeBoolType());

            uint64_t value = 0;
            switch (type->getTypeID()) {
	    case MiniMC::Model::TypeID::I8:
                value = std::static_pointer_cast<MiniMC::Model::TConstant<MiniMC::uint8_t>>(val)->getValue();
                break;
	    case MiniMC::Model::TypeID::I16:
                value = std::static_pointer_cast<MiniMC::Model::TConstant<MiniMC::uint16_t>>(val)->getValue();
                break;
	    case MiniMC::Model::TypeID::I32:
                value = std::static_pointer_cast<MiniMC::Model::TConstant<MiniMC::uint32_t>>(val)->getValue();
                break;
	    case MiniMC::Model::TypeID::I64:
                value = std::static_pointer_cast<MiniMC::Model::TConstant<MiniMC::uint64_t>>(val)->getValue();
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
        auto& cfg = F.getCFG();
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
