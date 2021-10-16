
#include "model/cfg.hpp"
#include "model/instructions.hpp"
#include <iostream>

namespace MiniMC {
  namespace Model {
    namespace Modifications {
      template <MiniMC::Model::InstructionCode i>
      void foldInstr(MiniMC::Model::Instruction& instr, MiniMC::Model::Program& prgm) {
        if constexpr (i == MiniMC::Model::InstructionCode::IntToBool) {
          MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::IntToBool> helper(instr);
          auto res = helper.getResult();
          auto val = helper.getCastee();
          auto type = val->getType();

          if (val->isConstant()) {
            assert(type->getTypeID() == MiniMC::Model::TypeID::Integer);
            auto cfac = prgm.getConstantFactory();
            auto tfac = prgm.getTypeFactory();

            auto tt = cfac->makeIntegerConstant(1, tfac->makeBoolType());
            auto ff = cfac->makeIntegerConstant(0, tfac->makeBoolType());

            uint64_t value = 0;
            switch (type->getSize()) {
              case 1:
                value = std::static_pointer_cast<MiniMC::Model::IntegerConstant<MiniMC::uint8_t>>(val)->getValue();
                break;
              case 2:
                value = std::static_pointer_cast<MiniMC::Model::IntegerConstant<MiniMC::uint16_t>>(val)->getValue();
                break;
              case 4:
                value = std::static_pointer_cast<MiniMC::Model::IntegerConstant<MiniMC::uint32_t>>(val)->getValue();
                break;
              case 8:
                value = std::static_pointer_cast<MiniMC::Model::IntegerConstant<MiniMC::uint64_t>>(val)->getValue();
                break;
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
        auto prgm = F.getPrgm();
        auto cfg = F.getCFG();
        for (auto& e : cfg->getEdges()) {
          if (e->hasAttribute<MiniMC::Model::AttributeType::Instructions>()) {
            for (auto& i : e->getAttribute<MiniMC::Model::AttributeType::Instructions>()) {
              switch (i.getOpcode()) {
#define X(INSTR)                                                \
  case MiniMC::Model::InstructionCode::INSTR:                   \
    foldInstr<MiniMC::Model::InstructionCode::INSTR>(i, *prgm); \
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
