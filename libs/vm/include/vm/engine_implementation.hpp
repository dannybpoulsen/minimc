#ifndef _ENGINE_IMPLE__
#define _ENGINE_IMPLE__

#include "model/cfg.hpp"
#include "model/valuevisitor.hpp"
#include "vm/value.hpp"
#include "vm/vmt.hpp"

namespace MiniMC {
  namespace VMT {

    template<class State, typename Operations, typename Caster>
    struct Impl {
      using T = typename State::Domain;
      template <class I>
      static Status runInstruction(const I&, State&, Operations&, Caster&, const MiniMC::Model::Program&)  {
	return Status::Ok;
	//throw NotImplemented<opc>();
      }
     

      template <class Value, class I>
      static Status runCMPAdd(const I& instr, State& writeState, Operations& operations, Caster&)
        requires MiniMC::Model::isTAC_v<I> || MiniMC::Model::isComparison_v<I>
      {
	constexpr auto op = I::getOpcode ();
        auto& content = instr.getOps ();
        auto& res = static_cast<MiniMC::Model::Register&>(*content.res);

        auto lval = writeState.getValueLookup().lookupValue(*content.op1).template as<Value>();
        auto rval = writeState.getValueLookup().lookupValue(*content.op2).template as<Value>();
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
        else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_SGE) {
          writeState.getValueLookup().saveValue(res, operations.SGe(lval, rval));
        } else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_SLE)
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

      template <class Value>
      static auto castPtrToAppropriateInteger(const Value& v, Caster& caster) {
        if constexpr (std::is_same_v<Value, typename T::Pointer>) {
          return caster.template PtrToInt<typename T::I64>(v);
        } else if constexpr (std::is_same_v<Value, typename T::Pointer32>) {
          return caster.template Ptr32ToInt<typename T::I32>(v);
        } else {
          static_assert("Not a pointer you try converting");
        }
      }

      template <class Value,class T>
      static Status runPointerCMP(const T& instr, State& writeState, Operations& operations, Caster& caster)
        requires MiniMC::Model::isComparison_v<T>
      {
	constexpr auto op = T::getOpcode ();
        auto& content = instr.getOps ();
        assert(content.op1->getType()->getTypeID() == MiniMC::Model::TypeID::Pointer);
        assert(content.op2->getType()->getTypeID() == MiniMC::Model::TypeID::Pointer);
        auto& res = static_cast<MiniMC::Model::Register&>(*content.res);

        auto lval = castPtrToAppropriateInteger<Value>(writeState.getValueLookup().lookupValue(*content.op1).template as<Value>(), caster);
        auto rval = castPtrToAppropriateInteger<Value>(writeState.getValueLookup().lookupValue(*content.op2).template as<Value>(), caster);

        if constexpr (op == MiniMC::Model::InstructionCode::ICMP_SGT)
          writeState.getValueLookup().saveValue(res, operations.SGt(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_SGE) {
          writeState.getValueLookup().saveValue(res, operations.SGe(lval, rval));
        } else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_SLE)
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

      template <class I>
      static Status runInstruction(const I& instr, State& writeState, Operations& ops, Caster& caster, const MiniMC::Model::Program&)
        requires MiniMC::Model::isTAC_v<I> &&
	IntOperationCompatible<typename T::I8, typename T::Bool, Operations> &&
	IntOperationCompatible<typename T::I16, typename T::Bool, Operations> &&
	IntOperationCompatible<typename T::I32, typename T::Bool, Operations> &&
	IntOperationCompatible<typename T::I64, typename T::Bool, Operations> &&
      ValueLookupable<State>

      {
        auto& content = instr.getOps();
        auto op1 = content.op1;

        switch (op1->getType()->getTypeID()) {
          case MiniMC::Model::TypeID::I8:
            return runCMPAdd<typename T::I8>(instr, writeState, ops, caster);
          case MiniMC::Model::TypeID::I16:
            return runCMPAdd<typename T::I16>(instr, writeState, ops, caster);
          case MiniMC::Model::TypeID::I32:
            return runCMPAdd<typename T::I32>(instr, writeState, ops, caster);
          case MiniMC::Model::TypeID::I64:
            return runCMPAdd<typename T::I64>(instr, writeState, ops, caster);
          default:
            throw NotImplemented<I::getOpcode ()>();
        }
      }

      template <class I>
      static Status runInstruction(const I& instr, State& writeState, Operations& ops, Caster& caster, const MiniMC::Model::Program&)
        requires MiniMC::Model::isComparison_v<I> &&
                 IntOperationCompatible<typename T::I8, typename T::Bool, Operations> &&
                 IntOperationCompatible<typename T::I16, typename T::Bool, Operations> &&
                 IntOperationCompatible<typename T::I32, typename T::Bool, Operations> &&
                 IntOperationCompatible<typename T::I64, typename T::Bool, Operations> &&
                 ValueLookupable<State>

      {
        auto& content = instr.getOps ();
        auto op1 = content.op1;

        switch (op1->getType()->getTypeID()) {
          case MiniMC::Model::TypeID::I8:
            return runCMPAdd<typename T::I8>(instr, writeState, ops, caster);
          case MiniMC::Model::TypeID::I16:
            return runCMPAdd<typename T::I16>(instr, writeState, ops, caster);
          case MiniMC::Model::TypeID::I32:
            return runCMPAdd<typename T::I32>(instr, writeState, ops, caster);
          case MiniMC::Model::TypeID::I64:
            return runCMPAdd<typename T::I64>(instr, writeState, ops, caster);
          case MiniMC::Model::TypeID::Pointer:
            return runPointerCMP<typename T::Pointer>(instr, writeState, ops, caster);
          case MiniMC::Model::TypeID::Pointer32:
            return runPointerCMP<typename T::Pointer32>(instr, writeState, ops, caster);

          default:
            throw MiniMC::Support::Exception("Something wen't wrong");
        }
        throw NotImplemented<I::getOpcode()>();
      }

      template <class I>
      static Status runInstruction(const I& instr, State& writeState, Operations& operations, Caster& caster, const MiniMC::Model::Program&)
        requires MiniMC::Model::isPointer_v<I> &&
                 PointerOperationCompatible<typename T::I8, typename T::Pointer, typename T::Bool, Operations> &&
                 PointerOperationCompatible<typename T::I16, typename T::Pointer, typename T::Bool, Operations> &&
                 PointerOperationCompatible<typename T::I32, typename T::Pointer, typename T::Bool, Operations> &&
                 PointerOperationCompatible<typename T::I64, typename T::Pointer, typename T::Bool, Operations> &&
                 ValueLookupable<State>

      {
	constexpr auto op = I::getOpcode ();
        auto& content = instr.getOps ();
        auto& res = static_cast<MiniMC::Model::Register&>(*content.res);

        auto addrConverter = [&caster](const auto& addrVal) {
          return addrVal.visit([&caster](const auto& value) -> T::Pointer {
            if constexpr (std::is_same_v<const typename T::Pointer&, decltype(value)>) {
              return value;
            }

            else if constexpr (std::is_same_v<const typename T::Pointer32&, decltype(value)>) {
              return caster.Ptr32ToPtr(value);
            } else {
              throw MiniMC::Support::Exception("SHouldn't get here");
            }
          });
        };

        if constexpr (op == MiniMC::Model::InstructionCode::PtrAdd) {
          auto ptr = addrConverter(writeState.getValueLookup().lookupValue(*content.ptr));
          auto calc = [&]<typename ValT>() {
            auto skipsize = writeState.getValueLookup().lookupValue(*content.skipsize).template as<ValT>();
            auto nbskips = writeState.getValueLookup().lookupValue(*content.nbSkips).template as<ValT>();
            auto totalskip = operations.Mul(skipsize, nbskips);
            writeState.getValueLookup().saveValue(res, operations.PtrAdd(ptr, totalskip));
            return Status::Ok;
          };
          switch (content.skipsize->getType()->getTypeID()) {
            case MiniMC::Model::TypeID::I8:
              return calc.template operator()<typename T::I8>();
            case MiniMC::Model::TypeID::I16:
              return calc.template operator()<typename T::I16>();
            case MiniMC::Model::TypeID::I32:
              return calc.template operator()<typename T::I32>();
            case MiniMC::Model::TypeID::I64:
              return calc.template operator()<typename T::I64>();
            default:
              throw MiniMC::Support::Exception("Invalid Skip-type type");
          }
        }
        if constexpr (op == MiniMC::Model::InstructionCode::PtrSub) {
          auto ptr = addrConverter(writeState.getValueLookup().lookupValue(*content.ptr));
          auto calc = [&]<typename ValT>() {
            auto skipsize = writeState.getValueLookup().lookupValue(*content.skipsize).template as<ValT>();
            auto nbskips = writeState.getValueLookup().lookupValue(*content.nbSkips).template as<ValT>();
            auto totalskip = operations.Mul(skipsize, nbskips);
            writeState.getValueLookup().saveValue(res, operations.PtrSub(ptr, totalskip));
            return Status::Ok;
          };
          switch (content.skipsize->getType()->getTypeID()) {
            case MiniMC::Model::TypeID::I8:
              return calc.template operator()<typename T::I8>();
            case MiniMC::Model::TypeID::I16:
              return calc.template operator()<typename T::I16>();
            case MiniMC::Model::TypeID::I32:
              return calc.template operator()<typename T::I32>();
            case MiniMC::Model::TypeID::I64:
              return calc.template operator()<typename T::I64>();
            default:
              throw MiniMC::Support::Exception("Invalid Skip-type type");
          }
        } else if constexpr (op == MiniMC::Model::InstructionCode::PtrEq) {
          auto lval = addrConverter(writeState.getValueLookup().lookupValue(*content.op1));
          auto rval = addrConverter(writeState.getValueLookup().lookupValue(*content.op2));
          writeState.getValueLookup().saveValue(res, operations.PtrEq(lval, rval));
          return Status::Ok;
        }
        throw NotImplemented<op>();
      }
      
      template <class  I>
      static Status runInstruction(const I& instr, State& writeState, Operations&, Caster& caster, const MiniMC::Model::Program&)
        requires MiniMC::Model::isMemory_v<I> &&
        CastCompatible<typename T::I8, typename T::I16, typename T::I32, typename T::I64, typename T::Bool, typename T::Pointer, typename T::Pointer32, Caster> &&
	ValueLookupable<State> 
      {
	constexpr auto op = I::getOpcode ();
        auto& content = instr.getOps();

        auto addrConverter = [&caster](const auto& addrVal) {
          return addrVal.visit([&caster](const auto& value) -> T::Pointer {
            if constexpr (std::is_same_v<const typename T::Pointer&, decltype(value)>) {
              return value;
            }

            else if constexpr (std::is_same_v<const typename T::Pointer32&, decltype(value)>) {
              return caster.Ptr32ToPtr(value);
            } else {
              throw MiniMC::Support::Exception("SHouldn't get here");
            }
          });
        };

        if constexpr (op == MiniMC::Model::InstructionCode::Load ) {
	  auto& res = static_cast<MiniMC::Model::Register&>(*content.res);
	  if constexpr (MemoryControllable<State>) {

	    auto addr = addrConverter(writeState.getValueLookup().lookupValue(*content.addr));
	    
	    writeState.getValueLookup().saveValue(res, writeState.getMemory().loadValue(addr, res.getType()));
	  }
	  else {
	    writeState.getValueLookup().saveValue(res, writeState.getValueLookUp ().unboundValue (res.getType ()));
	  }

	  return Status::Ok;
	  
	  
	}
	
        else if constexpr (op == MiniMC::Model::InstructionCode::Store) {
	  if constexpr (MemoryControllable<State>) {
	    auto value = writeState.getValueLookup().lookupValue(*content.storee);
	    auto addr = addrConverter(writeState.getValueLookup().lookupValue(*content.addr));
	    
	    value.visit([&writeState, &addr](const auto& t) {
	      if constexpr (!std::is_same_v<const typename T::Bool&, decltype(t)>)
		writeState.getMemory().storeValue(addr, t);
            else {
              throw MiniMC::Support::Exception("Cannot Store this type");
            }
	    });
	    
	  }
	  return Status::Ok;
	  
	}
	  
	else {
	  throw NotImplemented<op>();
	}
	  
      }
      
      template <class I>
      static Status runInstruction(const I& instr, State& writeState, Operations& operations, Caster&, const MiniMC::Model::Program&)
        requires MiniMC::Model::isAssertAssume_v<I> &&
	ValueLookupable<State>
      {
	constexpr auto op = instr.getOpcode ();
        auto& content = instr.getOps();
        auto& pathcontrol = writeState.getPathControl();
        auto obj = writeState.getValueLookup().lookupValue(*content.expr).template as<typename T::Bool>();
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

      template <MiniMC::Model::InstructionCode opc, class LeftOp, size_t bw>
      static T doOp(const LeftOp& op, Caster& caster) {
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

      template <MiniMC::Model::InstructionCode opc, class LeftOp>
      static T doOp(const LeftOp& op, Caster& caster, const MiniMC::Model::Type_ptr& p) {
        switch (p->getSize()) {
          case 1:
            return doOp<opc, LeftOp, 1>(op, caster);
          case 2:
            return doOp<opc, LeftOp, 2>(op, caster);
          case 4:
            return doOp<opc, LeftOp, 4>(op, caster);
          case 8:
            return doOp<opc, LeftOp, 8>(op, caster);
          default:
            throw MiniMC::Support::Exception("Error");
        }
      }

      template <class I>
      static Status runInstruction(const I& instr, State& writeState, Operations&, Caster& caster, const MiniMC::Model::Program&)
        requires MiniMC::Model::isCast_v<I> &&
	         CastCompatible<typename T::I8, typename T::I16, typename T::I32, typename T::I64, typename T::Bool, typename T::Pointer, typename T::Pointer32, Caster> &&
         	ValueLookupable<State>
      {
	constexpr auto op = I::getOpcode ();
        auto& content = instr.getOps ();
        auto& res = static_cast<MiniMC::Model::Register&>(*content.res);

        if constexpr (op == MiniMC::Model::InstructionCode::Trunc ||
                      op == MiniMC::Model::InstructionCode::ZExt ||
                      op == MiniMC::Model::InstructionCode::SExt) {
          auto op1 = writeState.getValueLookup().lookupValue(*content.op1);
          switch (content.op1->getType()->getTypeID()) {
            case MiniMC::Model::TypeID::I8:
              writeState.getValueLookup().saveValue(res, doOp<op, typename T::I8>(op1.template as<typename T::I8>(), caster, res.getType()));
              break;
            case MiniMC::Model::TypeID::I16:
              writeState.getValueLookup().saveValue(res, doOp<op, typename T::I16>(op1.template as<typename T::I16>(), caster, res.getType()));
              break;
            case MiniMC::Model::TypeID::I32:
              writeState.getValueLookup().saveValue(res, doOp<op, typename T::I32>(op1.template as<typename T::I32>(), caster, res.getType()));
              break;

	  case MiniMC::Model::TypeID::I64:
	    writeState.getValueLookup().saveValue(res, doOp<op, typename T::I64>(op1.template as<typename T::I64>(), caster, res.getType()));
              break;
            default:
              throw MiniMC::Support::Exception("Invalid Trunc/Extenstion");
          }
          return MiniMC::VMT::Status::Ok;
        }

        else if constexpr (op == MiniMC::Model::InstructionCode::BoolSExt) {
          auto op1 = writeState.getValueLookup().lookupValue(*content.op1).template as<typename T::Bool>();
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
          auto op1 = writeState.getValueLookup().lookupValue(*content.op1).template as<typename T::Bool>();
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
          auto op1 = writeState.getValueLookup().lookupValue(*content.op1);
          T result;
          if (res.getType()->getTypeID() == MiniMC::Model::TypeID::Pointer) {
            result = op1.visit([&caster](const auto& val) -> T {
              if constexpr (std::is_same_v<const typename T::I8&, decltype(val)> ||
                            std::is_same_v<const typename T::I16&, decltype(val)> ||
                            std::is_same_v<const typename T::I32&, decltype(val)> ||
                            std::is_same_v<const typename T::I64&, decltype(val)>) {
                return T{caster.IntToPtr(val)};
              } else {
                throw MiniMC::Support::Exception("Shouldn't get her");
              }
            });

          }

          else {
            result = op1.visit([&caster](const auto& val) -> T {
              if constexpr (std::is_same_v<const typename T::I8&, decltype(val)> ||
                            std::is_same_v<const typename T::I16&, decltype(val)> ||
                            std::is_same_v<const typename T::I32&, decltype(val)> ||
                            std::is_same_v<const typename T::I64&, decltype(val)>) {
                return T{caster.IntToPtr32(val)};
              } else {
                throw MiniMC::Support::Exception("Shouldn't get her");
              }
            });
          }
          writeState.getValueLookup().saveValue(res, std::move(result));

        }

        else if constexpr (op == MiniMC::Model::InstructionCode::IntToBool) {
          switch (content.op1->getType()->getTypeID()) {
            case MiniMC::Model::TypeID::I8: {
              auto op1 = writeState.getValueLookup().lookupValue(*content.op1).template as<typename T::I8>();

              writeState.getValueLookup().saveValue(res, caster.IntToBool(op1));
            } break;
            case MiniMC::Model::TypeID::I16: {
              auto op1 = writeState.getValueLookup().lookupValue(*content.op1).template as<typename T::I16>();

              writeState.getValueLookup().saveValue(res, caster.IntToBool(op1));
            } break;
            case MiniMC::Model::TypeID::I32: {
              auto op1 = writeState.getValueLookup().lookupValue(*content.op1).template as<typename T::I32>();

              writeState.getValueLookup().saveValue(res, caster.IntToBool(op1));
            } break;
            case MiniMC::Model::TypeID::I64: {
              auto op1 = writeState.getValueLookup().lookupValue(*content.op1).template as<typename T::I64>();

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

      template <class I>
      static Status runInstruction(const I& instr, State& writeState, Operations&, Caster&, const MiniMC::Model::Program& prgm)
        requires MiniMC::Model::isInternal_v<I>
      {
	constexpr auto op = I::getOpcode ();
        

        if constexpr (op == MiniMC::Model::InstructionCode::Assign && ValueLookupable<State>) {
	  auto& content = instr.getOps();
	  auto& res = static_cast<MiniMC::Model::Register&>(*content.res);
          auto op1 = writeState.getValueLookup().lookupValue(*content.op1);
          writeState.getValueLookup().saveValue(res, std::move(op1));
          return Status::Ok;
        }

        else if constexpr (op == MiniMC::Model::InstructionCode::Call && (StackControllable<State> || SimpStackControllable<State>)) {
	  auto& content = instr.getOps();
	  auto& scontrol = writeState.getStackControl();
          assert(content.function->isConstant());

          auto func = MiniMC::Model::visitValue(
              MiniMC::Model::Overload{
                  [&prgm](const MiniMC::Model::Pointer& t) -> MiniMC::Model::Function_ptr {
                    auto loadPtr = t.getValue();
                    auto func = prgm.getFunction(loadPtr.base);
                    return func;
                  },
                  [&prgm](const MiniMC::Model::Pointer32& t) -> MiniMC::Model::Function_ptr {
                    auto loadPtr = t.getValue();
                    auto func = prgm.getFunction(loadPtr.base);
                    return func;
                  },
                  [&prgm](const MiniMC::Model::SymbolicConstant& t) -> MiniMC::Model::Function_ptr {
                    auto symb = t.getValue();
                    auto func = prgm.getFunction(symb);
                    return func;
                  },
                  [](const auto&) -> MiniMC::Model::Function_ptr {
                    throw MiniMC::Support::Exception("Shouldn't happen");
                  }},
              *content.function);

	  auto& vstack = func->getRegisterDescr();
	  std::vector<T> params;
	  
	  
	  
	  if constexpr (ValueLookupable<State>) {
	    if (func->isVarArgs()) {
	      throw MiniMC::Support::Exception("Vararg functions are not supported");
	    }
	    
	    
	    auto inserter = std::back_inserter(params);
	    std::for_each(content.params.begin(), content.params.end(), [&inserter, &writeState](auto& v) { inserter = writeState.getValueLookup().lookupValue(*v); });
	  }
	  
	  
	  auto res = content.res;
	  scontrol.push(func->getCFA().getInitialLocation(), vstack.getTotalRegisters(), res);
	  
	  if constexpr (ValueLookupable<State>) {
	    for (auto& v : vstack.getRegisters()) {
	      writeState.getValueLookup().saveValue(*v, writeState.getValueLookup().defaultValue(*v->getType()));
	    }
	    
	    auto it = params.begin();
	    for (auto& p : func->getParameters()) {
	    writeState.getValueLookup().saveValue(*p, std::move(*it));
	    ++it;
	    }

	  }
	
	  return Status::Ok;
	}

        else if constexpr (op == MiniMC::Model::InstructionCode::Ret) {
	  auto& content = instr.getOps();
	  if constexpr (StackControllable<State> ) {
	    auto ret = writeState.getValueLookup().lookupValue(*content.value);
	    writeState.getStackControl().pop(std::move(ret));
	  }
	  else if constexpr (SimpStackControllable<State> ) {
	    writeState.getStackControl().popNoReturn();
	  
	  }
	  else{
	    throw NotImplemented<op> {};
	  }
		 
          return Status::Ok;
        }

        else if constexpr (op == MiniMC::Model::InstructionCode::RetVoid) {
          writeState.getStackControl().popNoReturn();
          return Status::Ok;
        }

        else if constexpr (op == MiniMC::Model::InstructionCode::Skip) {
          return Status::Ok;
        }

        else if constexpr (op == MiniMC::Model::InstructionCode::NonDet && ValueLookupable<State>) {
	  auto& content = instr.getOps();
          auto& res = static_cast<MiniMC::Model::Register&>(*content.res);
          auto ret = writeState.getValueLookup().unboundValue(*content.res->getType());
          writeState.getValueLookup().saveValue(res, std::move(ret));
          return Status::Ok;
        }

        else {
	  return Status::Ok;
	  //throw NotImplemented<op>();
        }
      }

      template <MiniMC::Model::InstructionCode op>
      static Status runInstruction(const MiniMC::Model::Instruction&, State&, Caster&, const MiniMC::Model::Program&)
        requires MiniMC::Model::InstructionData<op>::isPredicate
      {
        throw NotImplemented<op>();
        return Status::Ok;
      }

      template <class I>
      static Status runInstruction(const I& instr, State& writeState, Operations& operations, Caster&, const MiniMC::Model::Program&)
        requires MiniMC::Model::isAggregate_v<I> &&
                 AggregateCompatible<typename T::I8, typename T::Aggregate, Operations> &&
                 AggregateCompatible<typename T::I16, typename T::Aggregate, Operations> &&
                 AggregateCompatible<typename T::I32, typename T::Aggregate, Operations> &&
                 AggregateCompatible<typename T::I64, typename T::Aggregate, Operations> &&
                 ValueLookupable<State>

      {
	constexpr auto op = I::getOpcode ();
        auto& content = instr.getOps ();
        auto& res = static_cast<MiniMC::Model::Register&>(*content.res);
        assert(content.offset->isConstant());
        auto offset_constant = std::static_pointer_cast<MiniMC::Model::Constant>(content.offset);
        MiniMC::BV64 offset{0};

        offset = MiniMC::Model::visitValue(MiniMC::Model::Overload{
	      [](const MiniMC::Model::I16Integer& value) -> MiniMC::BV64 { return value.getValue(); },
	      [](const MiniMC::Model::I32Integer& value) -> MiniMC::BV64 { return value.getValue(); },
	      [](const MiniMC::Model::I64Integer& value) -> MiniMC::BV64 { return value.getValue(); },
	      [](const auto&) -> MiniMC::BV64 { throw MiniMC::Support::Exception("Invalid aggregate offset"); }},
	  *offset_constant);
	
        if constexpr (op == MiniMC::Model::InstructionCode::InsertValue) {
          auto val_v = content.insertee;

          auto aggr = writeState.getValueLookup().lookupValue(*content.aggregate).template as<typename T::Aggregate>();
          auto value = writeState.getValueLookup().lookupValue(*val_v);

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
          auto aggr = writeState.getValueLookup().lookupValue(*content.aggregate).template as<typename T::Aggregate>();

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

      };

    template <class State, class Operations, class Caster>
    Status Engine<State, Operations, Caster>::execute(const MiniMC::Model::Instruction& instr,
						      State& wstate) {
      
      return instr.visit ([this,&wstate](auto& t) {return Impl<State,Operations,Caster>::template runInstruction (t, wstate, operations, caster, prgm);});

    }

    template <class State, class Operations, class Caster>
    Status Engine<State, Operations, Caster>::execute(const MiniMC::Model::InstructionStream& instr,
                                                  State& wstate) {
      auto end = instr.end();
      Status status = Status::Ok;
      auto it = instr.begin();
      for (it = instr.begin(); it != end && status == Status::Ok; ++it) {
        status = execute (*it,wstate);
	if (status != Status::Ok)
	  return status;
      }
      return status;
    }

  } // namespace VMT
} // namespace MiniMC

#endif
