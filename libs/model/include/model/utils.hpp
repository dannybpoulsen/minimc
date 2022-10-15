#ifndef _MODEL_UTISL__
#define _MODEL_UTISL__

#include <unordered_map>

#include "model/cfg.hpp"
#include "model/instructions.hpp"
#include "model/variables.hpp"

namespace MiniMC {
  namespace Model {
    template <MiniMC::Model::InstructionCode c>
    MiniMC::Model::Value_ptr definesValue(const MiniMC::Model::Instruction& instr) {
      assert(instr.getOpcode() == c);
      if constexpr (MiniMC::Model::InstructionData<c>::hasResVar) {
        return instr.getOps<c>().res;
      }
      return nullptr;
    }

    template <MiniMC::Model::InstructionCode c, class Iterator>
    void findUsesOfInstr(const MiniMC::Model::Instruction& instr, Iterator iter) {
      assert(instr.getOpcode() == c);
      auto content = instr.getOps<c> ();
      if constexpr (MiniMC::Model::InstructionData<c>::isTAC ||
                    MiniMC::Model::InstructionData<c>::isComparison) {
        iter = content.op1;
        iter = content.op2;
      }

      else if constexpr (MiniMC::Model::InstructionData<c>::isCast) {
        iter = content.op1;
      }

      else if constexpr (c == MiniMC::Model::InstructionCode::Load) {
        iter = content.addr;
      }

      else if constexpr (c == MiniMC::Model::InstructionCode::Store) {
        iter = content.addr;
        iter = content.storee;
      }

      else if constexpr (c == MiniMC::Model::InstructionCode::ExtractValue) {
        iter = content.aggregate;
        iter = content.offset;
      }

      else if constexpr (c == MiniMC::Model::InstructionCode::InsertValue) {
        iter = content.aggregate;
        iter = content.offset;
        iter = content.inserteee;

      }

      /*else if constexpr (c == MiniMC::Model::InstructionCode::InsertValueFromConst) {
        iter = helper.getAggregate();
        iter = helper.getnsertee();
        for (size_t i = 0; i < helper.nbOps(); i++) {
          iter = helper.getOp(i);
        }
      }
      */
      else if constexpr (c == MiniMC::Model::InstructionCode::PtrAdd) {
        iter = content.nbSkips;
        iter = content.ptr;
        iter = content.skipsize;
      }

      else if constexpr (c == MiniMC::Model::InstructionCode::PtrEq) {

      }

      else if constexpr (c == MiniMC::Model::InstructionCode::Call) {
        iter = content.function;
        for (size_t i = 0; i < content.params.size; i++) {
          iter = content.params.at(i);
        }

      }

      else if constexpr (c == MiniMC::Model::InstructionCode::Assign) {
        iter = content.op1;

      }

      else if constexpr (c == MiniMC::Model::InstructionCode::Ret) {
        iter = content.value;

      } else if constexpr (c == MiniMC::Model::InstructionCode::RetVoid) {
      } else if constexpr (c == MiniMC::Model::InstructionCode::NonDet) {
	iter = content.min;
	iter = content.max;
      }

      else if constexpr (c == MiniMC::Model::InstructionCode::Assert) {
        iter = content.expr;

      }

      else if constexpr (c == MiniMC::Model::InstructionCode::Assume) {
        iter = content.expr;
      }

      else if constexpr (c == MiniMC::Model::InstructionCode::NegAssume) {
        iter = content.expr;

      }

      else if constexpr (c == MiniMC::Model::InstructionCode::Uniform) {
        iter = content.min;
        iter = content.max;
      }
    }

    inline MiniMC::Model::Value_ptr valueDefinedBy(const MiniMC::Model::Instruction& instr) {
      switch (instr.getOpcode()) {
#define X(CODE)                              \
  case MiniMC::Model::InstructionCode::CODE: \
    return definesValue<MiniMC::Model::InstructionCode::CODE>(instr);
        OPERATIONS
#undef X
      }
      return nullptr;
    }

    template <class Iterator>
    MiniMC::Model::Value_ptr valuesUsedBy(const MiniMC::Model::Instruction& instr, Iterator iter) {
      switch (instr.getOpcode()) {
#define X(CODE)                              \
  case MiniMC::Model::InstructionCode::CODE: \
    return findUsesOfInstr<MiniMC::Model::InstructionCode::CODE, Iterator>(instr, iter);
        OPERATIONS
#undef X
      }
    }

  } // namespace Model
} // namespace MiniMC

#endif
