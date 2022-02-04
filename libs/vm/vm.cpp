#include "vm/vm.hpp"
#include "model/instructions.hpp"
#include "model/variables.hpp"
#include "support/exceptions.hpp"

namespace MiniMC {
  namespace VM {
    /*
    template <MiniMC::Model::InstructionCode opc>
    Status runInstruction(const MiniMC::Model::Instruction&, VMState&, const VMState&) {
      throw MiniMC::VM::NotImplemented<opc> ();
    }

    template <MiniMC::Model::InstructionCode op>
    inline Status runInstruction(const MiniMC::Model::Instruction& instr, VMState& writeState, const VMState& readState) requires MiniMC::Model::InstructionData<op>::isTAC || MiniMC::Model::InstructionData<op>::isComparison {
      auto& content = instr.getOps<op>();
      auto res = std::static_pointer_cast<MiniMC::Model::Register>(content.res);

      auto lval = readState.getValueLookup().lookupValue(content.op1);
      auto rval = readState.getValueLookup().lookupValue(content.op2);
      if constexpr (op == MiniMC::Model::InstructionCode::Add)
        writeState.getValueLookup().saveValue(res, lval->Add(rval));
      else if constexpr (op == MiniMC::Model::InstructionCode::Sub)
        writeState.getValueLookup().saveValue(res, lval->Sub(rval));
      else if constexpr (op == MiniMC::Model::InstructionCode::Mul)
        writeState.getValueLookup().saveValue(res, lval->Mul(rval));
      else if constexpr (op == MiniMC::Model::InstructionCode::UDiv)
        writeState.getValueLookup().saveValue(res, lval->UDiv(rval));
      else if constexpr (op == MiniMC::Model::InstructionCode::SDiv)
        writeState.getValueLookup().saveValue(res, lval->SDiv(rval));
      else if constexpr (op == MiniMC::Model::InstructionCode::Shl)
        writeState.getValueLookup().saveValue(res, lval->Shl(rval));
      else if constexpr (op == MiniMC::Model::InstructionCode::LShr)
        writeState.getValueLookup().saveValue(res, lval->LShr(rval));
      else if constexpr (op == MiniMC::Model::InstructionCode::AShr)
        writeState.getValueLookup().saveValue(res, lval->AShr(rval));
      else if constexpr (op == MiniMC::Model::InstructionCode::And)
        writeState.getValueLookup().saveValue(res, lval->And(rval));
      else if constexpr (op == MiniMC::Model::InstructionCode::Or)
        writeState.getValueLookup().saveValue(res, lval->Or(rval));
      else if constexpr (op == MiniMC::Model::InstructionCode::Xor)
        writeState.getValueLookup().saveValue(res, lval->Xor(rval));
      else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_SGT)
        writeState.getValueLookup().saveValue(res, lval->SGt(rval));
      else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_SGE)
        writeState.getValueLookup().saveValue(res, lval->SGe(rval));
      else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_SLE)
        writeState.getValueLookup().saveValue(res, lval->SLe(rval));
      else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_SLT)
        writeState.getValueLookup().saveValue(res, lval->SLt(rval));
      else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_UGT)
        writeState.getValueLookup().saveValue(res, lval->UGt(rval));
      else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_UGE)
        writeState.getValueLookup().saveValue(res, lval->UGe(rval));
      else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_ULE)
        writeState.getValueLookup().saveValue(res, lval->ULe(rval));
      else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_ULT)
        writeState.getValueLookup().saveValue(res, lval->ULt(rval));
      else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_EQ)
        writeState.getValueLookup().saveValue(res, lval->Eq(rval));
      else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_NEQ)
        writeState.getValueLookup().saveValue(res, lval->NEq(rval));

      else
        throw NotImplemented<op>();

      return Status::Ok;
    }

    template <MiniMC::Model::InstructionCode op>
    inline Status runInstruction(const MiniMC::Model::Instruction& instr, VMState& writeState, const VMState& readState) requires MiniMC::Model::InstructionData<op>::isPointer {
      auto& content = instr.getOps<op>();
      auto res = std::static_pointer_cast<MiniMC::Model::Register>(content.res);

      if constexpr (op == MiniMC::Model::InstructionCode::PtrAdd) {
        auto ptr = readState.getValueLookup().lookupValue(content.ptr);
        auto skipsize = readState.getValueLookup().lookupValue(content.skipsize);
        auto nbskips = readState.getValueLookup().lookupValue(content.nbSkips);
        auto totalskip = skipsize->Mul(nbskips);
        writeState.getValueLookup().saveValue(res, ptr->PtrAdd(totalskip));
      } else if constexpr (op == MiniMC::Model::InstructionCode::PtrEq) {
        auto lval = readState.getValueLookup().lookupValue(content.op1);
        auto rval = readState.getValueLookup().lookupValue(content.op2);
        writeState.getValueLookup().saveValue(res, lval->PtrEq(rval));
      }
      return Status::Ok;
    }

    template <MiniMC::Model::InstructionCode op>
    inline Status runInstruction(const MiniMC::Model::Instruction& instr, VMState& writeState, const VMState& readState) requires MiniMC::Model::InstructionData<op>::isMemory {
      auto& content = instr.getOps<op>();

      if constexpr (op == MiniMC::Model::InstructionCode::Alloca || op == MiniMC::Model::InstructionCode::FindSpace) {
        auto res = std::static_pointer_cast<MiniMC::Model::Register>(content.res);
        auto size = readState.getValueLookup().lookupValue(content.op1);
        writeState.getValueLookup().saveValue(res, writeState.getMemory().alloca(size));
        return Status::Ok;
      } else if constexpr (op == MiniMC::Model::InstructionCode::Load) {
        auto res = std::static_pointer_cast<MiniMC::Model::Register>(content.res);
        auto addr = readState.getValueLookup().lookupValue(content.addr);

        writeState.getValueLookup().saveValue(res, readState.getMemory().loadValue(addr, res->getType()));
      }

      else if constexpr (op == MiniMC::Model::InstructionCode::Store) {
        auto value = readState.getValueLookup().lookupValue(content.storee);
        auto addr = readState.getValueLookup().lookupValue(content.addr);
        writeState.getMemory().storeValue(addr, value);
      }

      else if constexpr (op == MiniMC::Model::InstructionCode::Free) {
        auto obj = readState.getValueLookup().lookupValue(content.object);
        writeState.getMemory().free(obj);
      }

      else
        throw NotImplemented<op>();

      return Status::Ok;
    }

    template <MiniMC::Model::InstructionCode op>
    inline Status runInstruction(const MiniMC::Model::Instruction& instr, VMState& writeState, const VMState& readState) requires MiniMC::Model::InstructionData<op>::isAssertAssume {
      auto& content = instr.getOps<op>();
      auto& pathcontrol = writeState.getPathControl();
      auto obj = readState.getValueLookup().lookupValue(content.expr);
      if constexpr (op == MiniMC::Model::InstructionCode::Assume) {
	pathcontrol.addAssumption(obj);
        return (obj->triBool() == TriBool::False ? Status::AssumeViolated : Status::Ok);
      } else if constexpr (op == MiniMC::Model::InstructionCode::NegAssume) {

	pathcontrol.addAssumption(obj->BoolNegate());
        return (obj->triBool() == TriBool::True ? Status::AssumeViolated : Status::Ok);
      }

      else if constexpr (op == MiniMC::Model::InstructionCode::Assert) {
        pathcontrol.addAssert(obj);
        return (obj->triBool() == TriBool::False ? Status::AssertViolated : Status::Ok);

      } else
        throw NotImplemented<op>();
    }

    template <MiniMC::Model::InstructionCode op>
    inline Status runInstruction(const MiniMC::Model::Instruction& instr, VMState& writeState, const VMState& readState) requires MiniMC::Model::InstructionData<op>::isCast {
      auto& content = instr.getOps<op>();
      auto res = std::static_pointer_cast<MiniMC::Model::Register>(content.res);
      auto op1 = readState.getValueLookup().lookupValue(content.op1);

      if constexpr (op == MiniMC::Model::InstructionCode::Trunc) {
        writeState.getValueLookup().saveValue(res, op1->Trunc(res->getType()));
      }

      else if constexpr (op == MiniMC::Model::InstructionCode::ZExt) {
        writeState.getValueLookup().saveValue(res, op1->ZExt(res->getType()));
      }

      else if constexpr (op == MiniMC::Model::InstructionCode::SExt) {
        writeState.getValueLookup().saveValue(res, op1->SExt(res->getType()));
      }

      else if constexpr (op == MiniMC::Model::InstructionCode::BoolSExt) {
        writeState.getValueLookup().saveValue(res, op1->BoolSExt(res->getType()));
      }

      else if constexpr (op == MiniMC::Model::InstructionCode::BoolZExt) {
        writeState.getValueLookup().saveValue(res, op1->BoolZExt(res->getType()));
      }

      else if constexpr (op == MiniMC::Model::InstructionCode::IntToPtr) {
        writeState.getValueLookup().saveValue(res, op1->IntToPtr());
      }

      else if constexpr (op == MiniMC::Model::InstructionCode::IntToPtr) {
	auto ptrcaster = op1->PtrToI64 ();
	writeState.getValueLookup().saveValue(res, ptrcaster->Trunc (res->getType ()));
      }

      else if constexpr (op == MiniMC::Model::InstructionCode::IntToBool) {
	
	writeState.getValueLookup().saveValue(res, op1->IntToBool ());
      }
      
      else
        throw NotImplemented<op>();
      return Status::Ok;
    }

    template <MiniMC::Model::InstructionCode op>
    inline Status runInstruction(const MiniMC::Model::Instruction& instr, VMState& writeState, const VMState& readState) requires MiniMC::Model::InstructionData<op>::isInternal {
      auto& content = instr.getOps<op>();
      
      if constexpr (op  ==MiniMC::Model::InstructionCode::Assign) {
	auto res = std::static_pointer_cast<MiniMC::Model::Register>(content.res);
	auto op1 = readState.getValueLookup().lookupValue(content.op1);
	
	writeState.getValueLookup().saveValue(res, std::move(op1));
	return Status::Ok;
      }

      if constexpr (op  ==MiniMC::Model::InstructionCode::Skip) {
	return Status::Ok;
      }
      
    }
    template <MiniMC::Model::InstructionCode op>
    inline Status runInstruction(const MiniMC::Model::Instruction&, VMState&, const VMState&) requires MiniMC::Model::InstructionData<op>::isPredicate {
      throw NotImplemented<op>();
      return Status::Ok;
    }
    
    Status Engine::execute(const MiniMC::Model::InstructionStream& instr, VMState& wstate, const VMState& readstate) {
      auto end = instr.end();
      Status status = Status::Ok;
      for (auto it = instr.begin(); it != end && status == Status::Ok; ++it) {
        switch (it->getOpcode()) {
#define X(OP)                                                                            \
  case MiniMC::Model::InstructionCode::OP:                                               \
    status = runInstruction<MiniMC::Model::InstructionCode::OP>(*it, wstate, readstate); \
    break;
          OPERATIONS
          default:
            status = Status::UnsupportedOperation;
        }
      }

      return status;
    }
    */
  } // namespace VM
} // namespace MiniMC
