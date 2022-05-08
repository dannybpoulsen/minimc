#ifndef _ENGINE_IMPLE__
#define _ENGINE_IMPLE__

#include "vm/value.hpp"
#include "vm/vmt.hpp"

namespace MiniMC {
  namespace VMT {
    namespace Impl {
      template <MiniMC::Model::InstructionCode opc, typename T, typename IntOps, typename Caster>
      Status runInstruction(const MiniMC::Model::Instruction&, VMState<T>&, VMState<T, true>&, IntOps&, Caster&) {
        throw NotImplemented<opc>();
      }

      template <MiniMC::Model::InstructionCode op, class T, class Value, class Operation>
      inline Status runCMPAdd(const MiniMC::Model::Instruction& instr, VMState<T>& writeState, VMState<T, true>& readState, Operation& operations) requires MiniMC::Model::InstructionData<op>::isTAC || MiniMC::Model::InstructionData<op>::isComparison {
        auto& content = instr.getOps<op>();
        auto& res = static_cast<MiniMC::Model::Register&>(*content.res);

        auto lval = readState.getValueLookup().lookupValue(content.op1).template as<Value>();
        auto rval = readState.getValueLookup().lookupValue(content.op2).template as<Value>();
        if constexpr (op == MiniMC::Model::InstructionCode::Add)
          writeState.getValueLookup().saveValue(res, operations.Add(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::Sub)
          writeState.getValueLookup().saveValue(res, operations.Sub(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::Mul)
          writeState.getValueLookup().saveValue(res, operations.Mul(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::UDiv)
          writeState.getValueLookup().saveValue(res, operations.UDiv(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::SDiv)
          writeState.getValueLookup().saveValue(res, operations.SDiv(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::Shl)
          writeState.getValueLookup().saveValue(res, operations.LShl(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::LShr)
          writeState.getValueLookup().saveValue(res, operations.LShr(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::AShr)
          writeState.getValueLookup().saveValue(res, operations.AShr(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::And)
          writeState.getValueLookup().saveValue(res, operations.And(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::Or)
          writeState.getValueLookup().saveValue(res, operations.Or(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::Xor)
          writeState.getValueLookup().saveValue(res, operations.Xor(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_SGT)
          writeState.getValueLookup().saveValue(res, operations.SGt(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_SGE)
          writeState.getValueLookup().saveValue(res, operations.SGe(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_SLE)
          writeState.getValueLookup().saveValue(res, operations.SLe(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_SLT)
          writeState.getValueLookup().saveValue(res, operations.SLt(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_UGT)
          writeState.getValueLookup().saveValue(res, operations.UGt(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_UGE)
          writeState.getValueLookup().saveValue(res, operations.UGe(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_ULE)
          writeState.getValueLookup().saveValue(res, operations.ULe(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_ULT)
          writeState.getValueLookup().saveValue(res, operations.ULt(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_EQ)
          writeState.getValueLookup().saveValue(res, operations.Eq(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_NEQ)
          writeState.getValueLookup().saveValue(res, operations.NEq(lval, rval));

        else
          throw NotImplemented<op>();

        return Status::Ok;
      }

      template <MiniMC::Model::InstructionCode op, class T, typename Operations, typename Caster>
      inline Status runInstruction(const MiniMC::Model::Instruction& instr, VMState<T>& writeState, VMState<T, true>& readState, Operations& ops, Caster&) requires MiniMC::Model::InstructionData<op>::isTAC || MiniMC::Model::InstructionData<op>::isComparison {
        auto& content = instr.getOps<op>();
        auto op1 = content.op1;

        switch (op1->getType()->getSize()) {
          case 1:
            return runCMPAdd<op, T, typename T::I8>(instr, writeState, readState, ops);
          case 2:
            return runCMPAdd<op, T, typename T::I16>(instr, writeState, readState, ops);
          case 4:
            return runCMPAdd<op, T, typename T::I32>(instr, writeState, readState, ops);
          case 8:
            return runCMPAdd<op, T, typename T::I64>(instr, writeState, readState, ops);
        }
        throw NotImplemented<op>();
      }

      template <MiniMC::Model::InstructionCode op, class T, class Operations, class Caster>
      inline Status runInstruction(const MiniMC::Model::Instruction& instr, VMState<T>& writeState, VMState<T, true>& readState, Operations& operations, Caster&) requires MiniMC::Model::InstructionData<op>::isPointer {
        auto& content = instr.getOps<op>();
        auto& res = static_cast<MiniMC::Model::Register&>(*content.res);

        if constexpr (op == MiniMC::Model::InstructionCode::PtrAdd) {
          auto ptr = readState.getValueLookup().lookupValue(content.ptr).template as<typename T::Pointer>();
          auto calc = [&]<typename ValT>() {
            auto skipsize = readState.getValueLookup().lookupValue(content.skipsize).template as<ValT>();
            auto nbskips = readState.getValueLookup().lookupValue(content.nbSkips).template as<ValT>();
            auto totalskip = operations.Mul(skipsize, nbskips);
            writeState.getValueLookup().saveValue(res, operations.PtrAdd(ptr, totalskip));
            return Status::Ok;
          };
          switch (content.skipsize->getType()->getSize()) {
	    case 1:
              return calc.template operator()<typename T::I8>();
            case 2:
              return calc.template operator()<typename T::I16>();
            case 4:
              return calc.template operator()<typename T::I32>();
            case 8:
              return calc.template operator()<typename T::I64>();
          }
        } else if constexpr (op == MiniMC::Model::InstructionCode::PtrEq) {
          auto lval = readState.getValueLookup().lookupValue(content.op1).template as<typename T::Pointer>();
          auto rval = readState.getValueLookup().lookupValue(content.op2).template as<typename T::Pointer>();
          writeState.getValueLookup().saveValue(res, operations.PtrEq(lval, rval));
          return Status::Ok;
        }
        throw NotImplemented<op>();
      }

      template <MiniMC::Model::InstructionCode op, class T, class Operations, class Caster>
      inline Status runInstruction(const MiniMC::Model::Instruction& instr, VMState<T>& writeState, VMState<T, true>& readState, Operations&, Caster&) requires MiniMC::Model::InstructionData<op>::isMemory {
        auto& content = instr.getOps<op>();
	if constexpr (op == MiniMC::Model::InstructionCode::Alloca ) {
          auto& res = static_cast<MiniMC::Model::Register&>(*content.res);
          auto size = readState.getValueLookup().lookupValue(content.op1);
          writeState.getValueLookup().saveValue(res, writeState.getStackControl().alloc (size.template as<typename T::I64>()));
          return Status::Ok;
        } 
        else if constexpr (op == MiniMC::Model::InstructionCode::FindSpace) {
          auto& res = static_cast<MiniMC::Model::Register&>(*content.res);
          auto size = readState.getValueLookup().lookupValue(content.op1);
          writeState.getValueLookup().saveValue(res, writeState.getMemory().alloca(size.template as<typename T::I64>()));
          return Status::Ok;
        } else if constexpr (op == MiniMC::Model::InstructionCode::Load) {
	  auto& res = static_cast<MiniMC::Model::Register&>(*content.res);
	  auto addr = readState.getValueLookup().lookupValue(content.addr).template as<typename T::Pointer>();

          writeState.getValueLookup().saveValue(res, readState.getMemory().loadValue(addr, res.getType()));
        }

        else if constexpr (op == MiniMC::Model::InstructionCode::Store) {
          auto value = readState.getValueLookup().lookupValue(content.storee);
          auto addr = readState.getValueLookup().lookupValue(content.addr).template as<typename T::Pointer>();
          switch (content.storee->getType()->getTypeID()) {
            case MiniMC::Model::TypeID::Pointer:
              writeState.getMemory().storeValue(addr, value.template as<typename T::Pointer>());
              break;
            case MiniMC::Model::TypeID::I8:
              writeState.getMemory().storeValue(addr, value.template as<typename T::I8>());
              break;
            case MiniMC::Model::TypeID::I16:
              writeState.getMemory().storeValue(addr, value.template as<typename T::I16>());
              break;
            case MiniMC::Model::TypeID::I32:
              writeState.getMemory().storeValue(addr, value.template as<typename T::I32>());
              break;
            case MiniMC::Model::TypeID::I64:
              writeState.getMemory().storeValue(addr, value.template as<typename T::I64>());
              break;
            default:
              throw NotImplemented<op>();
          }
        }

        else if constexpr (op == MiniMC::Model::InstructionCode::Free) {
          auto obj = readState.getValueLookup().lookupValue(content.object).template as<typename T::Pointer>();
          writeState.getMemory().free(obj);
        }

        else
          throw NotImplemented<op>();

        return Status::Ok;
      }

      template <MiniMC::Model::InstructionCode op, class T, class Operations, class Caster>
      inline Status runInstruction(const MiniMC::Model::Instruction& instr, VMState<T>& writeState, VMState<T, true>& readState, Operations& operations, Caster&) requires MiniMC::Model::InstructionData<op>::isAssertAssume {
        auto& content = instr.getOps<op>();
        auto& pathcontrol = writeState.getPathControl();
        auto obj = readState.getValueLookup().lookupValue(content.expr).template as<typename T::Bool>();
        if constexpr (op == MiniMC::Model::InstructionCode::Assume) {
          auto res = pathcontrol.addAssumption(obj);
          return (res == TriBool::False ? Status::AssumeViolated : Status::Ok);
        } else if constexpr (op == MiniMC::Model::InstructionCode::NegAssume) {
          auto neg{operations.BoolNegate(obj)};
          auto res = pathcontrol.addAssumption(neg);
          return (res == TriBool::False ? Status::AssumeViolated : Status::Ok);
        }

        else if constexpr (op == MiniMC::Model::InstructionCode::Assert) {
          auto res = pathcontrol.addAssert(obj);
          return (res == TriBool::False ? Status::AssertViolated : Status::Ok);

        } else
          throw NotImplemented<op>();
      }

      template <MiniMC::Model::InstructionCode opc, class LeftOp, size_t bw, class Caster, class Val>
      Val doOp(const LeftOp& op, Caster& caster) {
        if constexpr (opc == MiniMC::Model::InstructionCode::Trunc) {
          if constexpr (bw * 8 > LeftOp::intbitsize()) {
            throw MiniMC::Support::Exception("Invalid Truncaation");
          } else
            return caster.template Trunc<bw, LeftOp>(op);
        } else if constexpr (opc == MiniMC::Model::InstructionCode::ZExt) {
          if constexpr (bw * 8 < LeftOp::intbitsize()) {
            throw MiniMC::Support::Exception("Invalid Extension");
          } else
            return caster.template ZExt<bw, LeftOp>(op);
        } else if constexpr (opc == MiniMC::Model::InstructionCode::SExt) {
          if constexpr (bw * 8 < LeftOp::intbitsize()) {
            throw MiniMC::Support::Exception("Invalid Extension");
          } else
            return caster.template SExt<bw, LeftOp>(op);
        } else {
          []<bool b = false>() { static_assert(b); }
          ();
        }
      }

      template <MiniMC::Model::InstructionCode opc, class Val, class T, class Caster>
      Val doOp(const T& op, Caster& caster, const MiniMC::Model::Type_ptr& p) {
        switch (p->getSize()) {
          case 1:
            return doOp<opc, T, 1, Caster, Val>(op, caster);
          case 2:
            return doOp<opc, T, 2, Caster, Val>(op, caster);
          case 4:
            return doOp<opc, T, 4, Caster, Val>(op, caster);
          case 8:
            return doOp<opc, T, 8, Caster, Val>(op, caster);
          default:
            throw MiniMC::Support::Exception("Error");
        }
      }

      template <MiniMC::Model::InstructionCode op, class T, class Operations, class Caster>
      inline Status runInstruction(const MiniMC::Model::Instruction& instr, VMState<T>& writeState, VMState<T, true>& readState, Operations&, Caster& caster) requires MiniMC::Model::InstructionData<op>::isCast {
        auto& content = instr.getOps<op>();
        auto& res = static_cast<MiniMC::Model::Register&>(*content.res);

        if constexpr (op == MiniMC::Model::InstructionCode::Trunc ||
                      op == MiniMC::Model::InstructionCode::ZExt ||
                      op == MiniMC::Model::InstructionCode::SExt) {
          auto op1 = readState.getValueLookup().lookupValue(content.op1);
          switch (content.op1->getType()->getTypeID()) {
            case MiniMC::Model::TypeID::I8:
              writeState.getValueLookup().saveValue(res, doOp<op, T>(op1.template as<typename T::I8>(), caster, res.getType()));
              break;
            case MiniMC::Model::TypeID::I16:
              writeState.getValueLookup().saveValue(res, doOp<op, T>(op1.template as<typename T::I16>(), caster, res.getType()));
              break;
            case MiniMC::Model::TypeID::I32:
              writeState.getValueLookup().saveValue(res, doOp<op, T>(op1.template as<typename T::I32>(), caster, res.getType()));
              break;

            case MiniMC::Model::TypeID::I64:
              writeState.getValueLookup().saveValue(res, doOp<op, T>(op1.template as<typename T::I64>(), caster, res.getType()));
              break;
            default:
              throw MiniMC::Support::Exception("Invalid Trunc/Extenstion");
          }
          return MiniMC::VMT::Status::Ok;
        }

        else if constexpr (op == MiniMC::Model::InstructionCode::BoolSExt) {
          auto op1 = readState.getValueLookup().lookupValue(content.op1).template as<typename T::Bool>();
          switch (res.getType()->getTypeID()) {
            case MiniMC::Model::TypeID::I8:
              writeState.getValueLookup().saveValue(res, caster.template BoolSExt<1>(op1));
              break;
            case MiniMC::Model::TypeID::I16:
              writeState.getValueLookup().saveValue(res, caster.template BoolSExt<2>(op1));
              break;
            case MiniMC::Model::TypeID::I32:
              writeState.getValueLookup().saveValue(res, caster.template BoolSExt<4>(op1));
              break;
            case MiniMC::Model::TypeID::I64:
              writeState.getValueLookup().saveValue(res, caster.template BoolSExt<8>(op1));
              break;
            default:
              throw MiniMC::Support::Exception("Invalid Extenstion");
          }
        }

        else if constexpr (op == MiniMC::Model::InstructionCode::BoolZExt) {
          auto op1 = readState.getValueLookup().lookupValue(content.op1).template as<typename T::Bool>();
          switch (res.getType()->getTypeID()) {
            case MiniMC::Model::TypeID::I8:
              writeState.getValueLookup().saveValue(res, caster.template BoolZExt<1>(op1));
              break;
            case MiniMC::Model::TypeID::I16:
              writeState.getValueLookup().saveValue(res, caster.template BoolZExt<2>(op1));
              break;
            case MiniMC::Model::TypeID::I32:
              writeState.getValueLookup().saveValue(res, caster.template BoolZExt<4>(op1));
              break;
            case MiniMC::Model::TypeID::I64:
              writeState.getValueLookup().saveValue(res, caster.template BoolZExt<8>(op1));
              break;
            default:
              throw MiniMC::Support::Exception("Invalid Extenstion");
          }
        }

        else if constexpr (op == MiniMC::Model::InstructionCode::IntToPtr) {
          auto op1 = readState.getValueLookup().lookupValue(content.op1).template as<typename T::Pointer>();
          switch (content.op1->getType()->getTypeID()) {
            case MiniMC::Model::TypeID::I8: {
              auto op1 = readState.getValueLookup().lookupValue(content.op1).template as<typename T::I8>();

              writeState.getValueLookup().saveValue(res, caster.IntToPtr(op1));
            } break;
            case MiniMC::Model::TypeID::I16: {
              auto op1 = readState.getValueLookup().lookupValue(content.op1).template as<typename T::I16>();

              writeState.getValueLookup().saveValue(res, caster.IntToPtr(op1));
            } break;
            case MiniMC::Model::TypeID::I32: {
              auto op1 = readState.getValueLookup().lookupValue(content.op1).template as<typename T::I32>();

              writeState.getValueLookup().saveValue(res, caster.IntToPtr(op1));
            } break;
            case MiniMC::Model::TypeID::I64: {
              auto op1 = readState.getValueLookup().lookupValue(content.op1).template as<typename T::I64>();

              writeState.getValueLookup().saveValue(res, caster.IntToPtr(op1));
            } break;
            default:
              throw MiniMC::Support::Exception("Invalied IntToPtr");
          }
        }

        else if constexpr (op == MiniMC::Model::InstructionCode::IntToBool) {
          switch (content.op1->getType()->getTypeID()) {
            case MiniMC::Model::TypeID::I8: {
              auto op1 = readState.getValueLookup().lookupValue(content.op1).template as<typename T::I8>();

              writeState.getValueLookup().saveValue(res, caster.IntToBool(op1));
            } break;
            case MiniMC::Model::TypeID::I16: {
              auto op1 = readState.getValueLookup().lookupValue(content.op1).template as<typename T::I16>();

              writeState.getValueLookup().saveValue(res, caster.IntToBool(op1));
            } break;
            case MiniMC::Model::TypeID::I32: {
              auto op1 = readState.getValueLookup().lookupValue(content.op1).template as<typename T::I32>();

              writeState.getValueLookup().saveValue(res, caster.IntToBool(op1));
            } break;
            case MiniMC::Model::TypeID::I64: {
              auto op1 = readState.getValueLookup().lookupValue(content.op1).template as<typename T::I64>();

              writeState.getValueLookup().saveValue(res, caster.IntToBool(op1));
            } break;
            default:
              throw MiniMC::Support::Exception("Invalied IntToBool");
          }
        }

        else
          throw NotImplemented<op>();
        return Status::Ok;
      }

      template <MiniMC::Model::InstructionCode op, class T, class Operations, class Caster>
      inline Status runInstruction(const MiniMC::Model::Instruction& instr, VMState<T>& writeState, VMState<T, true>& readState, Operations&, Caster&) requires MiniMC::Model::InstructionData<op>::isInternal {
        auto& content = instr.getOps<op>();

        if constexpr (op == MiniMC::Model::InstructionCode::Assign) {
          auto& res = static_cast<MiniMC::Model::Register&>(*content.res);
          auto op1 = readState.getValueLookup().lookupValue(content.op1);
          writeState.getValueLookup().saveValue(res, std::move(op1));
          return Status::Ok;
        }

        else if constexpr (op == MiniMC::Model::InstructionCode::Call) {
          auto& scontrol = writeState.getStackControl();
	  assert(content.function->isConstant());
	  
	  auto func = std::static_pointer_cast<MiniMC::Model::Pointer> (content.function)->getValue ();
          std::vector<T> params;
          auto inserter = std::back_inserter(params);
          std::for_each(content.params.begin(), content.params.end(), [&inserter, &readState](auto& v) { inserter = readState.getValueLookup().lookupValue(v); });

          auto res = content.res;
          scontrol.push(func, params, res);
          return Status::Ok;
        }

        else if constexpr (op == MiniMC::Model::InstructionCode::Ret) {
          auto ret = readState.getValueLookup().lookupValue(content.value);
          writeState.getStackControl().pop(std::move(ret));
          return Status::Ok;
        }

        else if constexpr (op == MiniMC::Model::InstructionCode::RetVoid) {
          writeState.getStackControl().popNoReturn();
          return Status::Ok;
        }

        else if constexpr (op == MiniMC::Model::InstructionCode::Skip) {
          return Status::Ok;
        } else {
          throw NotImplemented<op>();
        }
      }

      template <MiniMC::Model::InstructionCode op, class T, class Caster>
      inline Status runInstruction(const MiniMC::Model::Instruction&, VMState<T>&, VMState<T, true>&, Caster&) requires MiniMC::Model::InstructionData<op>::isPredicate {
        throw NotImplemented<op>();
        return Status::Ok;
      }

      template <MiniMC::Model::InstructionCode op, class T, class Operation, class Caster>
      inline Status runInstruction(const MiniMC::Model::Instruction& instr, VMState<T>& writeState, VMState<T, true>& readState, Operation& operations, Caster&) requires MiniMC::Model::InstructionData<op>::isAggregate {
        auto& content = instr.getOps<op>();
        auto& res = static_cast<MiniMC::Model::Register&>(*content.res);
        assert(content.offset->isConstant());
        auto offset_constant = std::static_pointer_cast<MiniMC::Model::Constant>(content.offset);
        MiniMC::int64_t offset{0};

        /* This is nasty. We should update the typechecker to ensure onæy I64 bit integers are allowed as offsets*/
        switch (offset_constant->getSize()) {
          case 1:
            offset = std::static_pointer_cast<MiniMC::Model::I8Integer>(offset_constant)->getValue();
            break;
          case 2:
            offset = std::static_pointer_cast<MiniMC::Model::I16Integer>(offset_constant)->getValue();
            break;
          case 4:
            offset = std::static_pointer_cast<MiniMC::Model::I32Integer>(offset_constant)->getValue();
            break;
          case 8:
            offset = std::static_pointer_cast<MiniMC::Model::I64Integer>(offset_constant)->getValue();
            break;
          default:
            throw MiniMC::Support::Exception("Invalid Conversion");
        }

        if constexpr (op == MiniMC::Model::InstructionCode::InsertValue) {
          auto val_v = content.insertee;

          auto aggr = readState.getValueLookup().lookupValue(content.aggregate).template as<typename T::Aggregate>();
          auto value = readState.getValueLookup().lookupValue(val_v);

          switch (val_v->getType()->getTypeID()) {
            case MiniMC::Model::TypeID::I8:
              writeState.getValueLookup().saveValue(res, operations.template InsertBaseValue<typename T::I8>(aggr, offset, value.template as<typename T::I8>()));
              break;
            case MiniMC::Model::TypeID::I16:
              writeState.getValueLookup().saveValue(res, operations.template InsertBaseValue<typename T::I16>(aggr, offset, value.template as<typename T::I16>()));
              break;
            case MiniMC::Model::TypeID::I32:
              writeState.getValueLookup().saveValue(res, operations.template InsertBaseValue<typename T::I32>(aggr, offset, value.template as<typename T::I32>()));
              break;
            case MiniMC::Model::TypeID::I64:
              writeState.getValueLookup().saveValue(res, operations.template InsertBaseValue<typename T::I64>(aggr, offset, value.template as<typename T::I64>()));
              break;
            case MiniMC::Model::TypeID::Pointer:
              writeState.getValueLookup().saveValue(res, operations.template InsertBaseValue<typename T::Pointer>(aggr, offset, value.template as<typename T::Pointer>()));
              break;
            case MiniMC::Model::TypeID::Array:
            case MiniMC::Model::TypeID::Struct:
              writeState.getValueLookup().saveValue(res, operations.template InsertAggregateValue(aggr, offset, value.template as<typename T::Aggregate>()));
              break;
            default:
              throw MiniMC::Support::Exception("Invalid Insert");
          }
        }

        else if constexpr (op == MiniMC::Model::InstructionCode::ExtractValue) {
          auto aggr = readState.getValueLookup().lookupValue(content.aggregate).template as<typename T::Aggregate>();

          switch (res.getType()->getTypeID()) {
            case MiniMC::Model::TypeID::I8:
              writeState.getValueLookup().saveValue(res, operations.template ExtractBaseValue<typename T::I8>(aggr, offset));
              break;
            case MiniMC::Model::TypeID::I16:
              writeState.getValueLookup().saveValue(res, operations.template ExtractBaseValue<typename T::I16>(aggr, offset));
              break;
            case MiniMC::Model::TypeID::I32:
              writeState.getValueLookup().saveValue(res, operations.template ExtractBaseValue<typename T::I32>(aggr, offset));
              break;
            case MiniMC::Model::TypeID::I64:
              writeState.getValueLookup().saveValue(res, operations.template ExtractBaseValue<typename T::I64>(aggr, offset));
              break;

            case MiniMC::Model::TypeID::Pointer:
              writeState.getValueLookup().saveValue(res, operations.template ExtractBaseValue<typename T::Pointer>(aggr, offset));
              break;
            case MiniMC::Model::TypeID::Array:
            case MiniMC::Model::TypeID::Struct:
              writeState.getValueLookup().saveValue(res, operations.ExtractAggregateValue(aggr, offset, res.getType()->getSize()));
              break;
            default:
              throw MiniMC::Support::Exception("Invalid Extract");
          }
        } else
          throw NotImplemented<op>();
        return Status::Ok;
      }

    } // namespace Impl

    template <class T, class Operations, class Caster>
    Status Engine<T, Operations, Caster>::execute(const MiniMC::Model::InstructionStream& instr,
                                                  VMState<T>& wstate,
                                                  VMState<T, true>& readstate) {
      auto end = instr.end();
      Status status = Status::Ok;
      for (auto it = instr.begin(); it != end && status == Status::Ok; ++it) {
	switch (it->getOpcode()) {
#define X(OP)                                                                                                                             \
  case MiniMC::Model::InstructionCode::OP:                                                                                                \
    status = Impl::runInstruction<MiniMC::Model::InstructionCode::OP, T, Operations, Caster>(*it, wstate, readstate, operations, caster); \
    break;
          OPERATIONS

#undef X
          default:
            status = Status::UnsupportedOperation;
        }
      }

      return status;
    }

  } // namespace VMT
} // namespace MiniMC

#endif
