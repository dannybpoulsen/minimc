#ifndef _ENGINE_IMPLE__
#define _ENGINE_IMPLE__

#include "vm/value.hpp"
#include "vm/vmt.hpp"
#include "model/cfg.hpp"
#include "model/valuevisitor.hpp"

namespace MiniMC {
  namespace VMT {
    namespace Impl {
      template <MiniMC::Model::InstructionCode opc, typename T, typename IntOps, typename Caster>
      Status runInstruction(const MiniMC::Model::Instruction&, VMState<T>&,  IntOps&, Caster&, const MiniMC::Model::Program&) {
        throw NotImplemented<opc>();
      }

      template <MiniMC::Model::InstructionCode op, class T, class Value, class Operation>
      inline Status runCMPAdd(const MiniMC::Model::Instruction& instr, VMState<T>& writeState,Operation& operations) requires MiniMC::Model::InstructionData<op>::isTAC || MiniMC::Model::InstructionData<op>::isComparison {
        auto& content = instr.getOps<op>();
        auto& res = static_cast<MiniMC::Model::Register&>(*content.res);

        auto lval = writeState.getStackControl().getValueLookup ().lookupValue(content.op1).template as<Value>();
        auto rval = writeState.getStackControl().getValueLookup ().lookupValue(content.op2).template as<Value>();
        if constexpr (op == MiniMC::Model::InstructionCode::Add)
          writeState.getStackControl().getValueLookup ().saveValue(res, operations.Add(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::Sub)
          writeState.getStackControl().getValueLookup ().saveValue(res, operations.Sub(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::Mul)
          writeState.getStackControl().getValueLookup ().saveValue(res, operations.Mul(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::UDiv)
          writeState.getStackControl().getValueLookup ().saveValue(res, operations.UDiv(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::SDiv)
          writeState.getStackControl().getValueLookup ().saveValue(res, operations.SDiv(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::Shl)
          writeState.getStackControl().getValueLookup ().saveValue(res, operations.LShl(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::LShr)
          writeState.getStackControl().getValueLookup ().saveValue(res, operations.LShr(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::AShr)
          writeState.getStackControl().getValueLookup ().saveValue(res, operations.AShr(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::And)
          writeState.getStackControl().getValueLookup ().saveValue(res, operations.And(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::Or)
          writeState.getStackControl().getValueLookup ().saveValue(res, operations.Or(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::Xor)
          writeState.getStackControl().getValueLookup ().saveValue(res, operations.Xor(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_SGT)
          writeState.getStackControl().getValueLookup ().saveValue(res, operations.SGt(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_SGE) {
	  writeState.getStackControl().getValueLookup ().saveValue(res, operations.SGe(lval, rval));
	}
        else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_SLE)
          writeState.getStackControl().getValueLookup ().saveValue(res, operations.SLe(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_SLT)
          writeState.getStackControl().getValueLookup ().saveValue(res, operations.SLt(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_UGT)
          writeState.getStackControl().getValueLookup ().saveValue(res, operations.UGt(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_UGE)
          writeState.getStackControl().getValueLookup ().saveValue(res, operations.UGe(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_ULE)
          writeState.getStackControl().getValueLookup ().saveValue(res, operations.ULe(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_ULT)
          writeState.getStackControl().getValueLookup ().saveValue(res, operations.ULt(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_EQ)
          writeState.getStackControl().getValueLookup ().saveValue(res, operations.Eq(lval, rval));
        else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_NEQ)
          writeState.getStackControl().getValueLookup ().saveValue(res, operations.NEq(lval, rval));

        else
          throw NotImplemented<op>();

        return Status::Ok;
      }

      template <MiniMC::Model::InstructionCode op, class T, typename Operations, typename Caster>
      inline Status runInstruction(const MiniMC::Model::Instruction& instr, VMState<T>& writeState, Operations& ops, Caster&, const MiniMC::Model::Program&) requires MiniMC::Model::InstructionData<op>::isTAC || MiniMC::Model::InstructionData<op>::isComparison {
        auto& content = instr.getOps<op>();
        auto op1 = content.op1;

        switch (op1->getType()->getTypeID ()) {
	case MiniMC::Model::TypeID::I8:
            return runCMPAdd<op, T, typename T::I8>(instr, writeState,  ops);
          case MiniMC::Model::TypeID::I16:
            return runCMPAdd<op, T, typename T::I16>(instr, writeState, ops);
          case MiniMC::Model::TypeID::I32:
            return runCMPAdd<op, T, typename T::I32>(instr, writeState, ops);
          case MiniMC::Model::TypeID::I64:
            return runCMPAdd<op, T, typename T::I64>(instr, writeState, ops);
	default:
	  throw MiniMC::Support::Exception ("Throw");
	}
        throw NotImplemented<op>();
      }

      template <MiniMC::Model::InstructionCode op, class T, class Operations, class Caster>
      inline Status runInstruction(const MiniMC::Model::Instruction& instr, VMState<T>& writeState, Operations& operations, Caster& caster, const MiniMC::Model::Program&) requires MiniMC::Model::InstructionData<op>::isPointer {
        auto& content = instr.getOps<op>();
        auto& res = static_cast<MiniMC::Model::Register&>(*content.res);


	auto addrConverter = [&caster](const auto& addrVal) {
	  return addrVal.visit ([&caster](const auto& value) -> T::Pointer{
	    if constexpr (std::is_same_v<const typename  T::Pointer&,decltype(value)>) {
	      return value;
	    }

	    else if constexpr (std::is_same_v<const typename T::Pointer32&,decltype(value)>) {
	      return caster.Ptr32ToPtr (value); 
	    }
	    else {
	      throw MiniMC::Support::Exception ("SHouldn't get here");
	    }
	  }
	    );
	   
	};

	
        if constexpr (op == MiniMC::Model::InstructionCode::PtrAdd) {
	  auto ptr = addrConverter (writeState.getStackControl().getValueLookup ().lookupValue(content.ptr));
          auto calc = [&]<typename ValT>() {
            auto skipsize = writeState.getStackControl().getValueLookup ().lookupValue(content.skipsize).template as<ValT>();
            auto nbskips = writeState.getStackControl().getValueLookup ().lookupValue(content.nbSkips).template as<ValT>();
            auto totalskip = operations.Mul(skipsize, nbskips);
            writeState.getStackControl().getValueLookup ().saveValue(res, operations.PtrAdd(ptr, totalskip));
            return Status::Ok;
          };
          switch (content.skipsize->getType()->getTypeID ()) {
	  case MiniMC::Model::TypeID::I8:
              return calc.template operator()<typename T::I8>();
	  case MiniMC::Model::TypeID::I16:
              return calc.template operator()<typename T::I16>();
	  case MiniMC::Model::TypeID::I32:
              return calc.template operator()<typename T::I32>();
	  case MiniMC::Model::TypeID::I64:
              return calc.template operator()<typename T::I64>();
	  default:
	    throw MiniMC::Support::Exception ("Invalid Skip-type type");
	  }
        } else if constexpr (op == MiniMC::Model::InstructionCode::PtrEq) {
          auto lval = addrConverter (writeState.getStackControl().getValueLookup ().lookupValue(content.op1));
          auto rval = addrConverter (writeState.getStackControl().getValueLookup ().lookupValue(content.op2));
          writeState.getStackControl().getValueLookup ().saveValue(res, operations.PtrEq(lval, rval));
          return Status::Ok;
        }
        throw NotImplemented<op>();
	
      }
      template <MiniMC::Model::InstructionCode op, class T, class Operations, class Caster>
      inline Status runInstruction(const MiniMC::Model::Instruction& instr, VMState<T>& writeState, Operations&, Caster& caster, const MiniMC::Model::Program&) requires MiniMC::Model::InstructionData<op>::isMemory {
        auto& content = instr.getOps<op>();

	auto addrConverter = [&caster](const auto& addrVal) {
	  return addrVal.visit ([&caster](const auto& value) -> T::Pointer{
	    if constexpr (std::is_same_v<const typename  T::Pointer&,decltype(value)>) {
	      return value;
	    }

	    else if constexpr (std::is_same_v<const typename T::Pointer32&,decltype(value)>) {
	      return caster.Ptr32ToPtr (value); 
	    }
	    else {
	      throw MiniMC::Support::Exception ("SHouldn't get here");
	    }
	  }
	    );
	   
	};
	
	if constexpr (op == MiniMC::Model::InstructionCode::Load) {
	  auto& res = static_cast<MiniMC::Model::Register&>(*content.res);
	  auto addr = addrConverter (writeState.getStackControl().getValueLookup ().lookupValue(content.addr));
	  
          writeState.getStackControl().getValueLookup ().saveValue(res, writeState.getMemory().loadValue(addr, res.getType()));
        }

        else if constexpr (op == MiniMC::Model::InstructionCode::Store) {
          auto value = writeState.getStackControl().getValueLookup ().lookupValue(content.storee);
	  auto addr =  addrConverter (writeState.getStackControl().getValueLookup ().lookupValue(content.addr));
	  
	  value.visit ([&writeState,&addr](const auto& t) {
	    if constexpr (!std::is_same_v<const typename T::Bool&,decltype(t)>)
	      writeState.getMemory().storeValue(addr, t);
	    else {
	      throw MiniMC::Support::Exception ("Cannot Store this type");
	    }
	  });
	}
        
        else
          throw NotImplemented<op>();

        return Status::Ok;
      }

      template <MiniMC::Model::InstructionCode op, class T, class Operations, class Caster>
      inline Status runInstruction(const MiniMC::Model::Instruction& instr, VMState<T>& writeState,  Operations& operations, Caster&, const MiniMC::Model::Program&) requires MiniMC::Model::InstructionData<op>::isAssertAssume {
        auto& content = instr.getOps<op>();
        auto& pathcontrol = writeState.getPathControl();
        auto obj = writeState.getStackControl().getValueLookup ().lookupValue(content.expr).template as<typename T::Bool>();
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
      inline Status runInstruction(const MiniMC::Model::Instruction& instr, VMState<T>& writeState,  Operations&, Caster& caster, const MiniMC::Model::Program&) requires MiniMC::Model::InstructionData<op>::isCast {
        auto& content = instr.getOps<op>();
        auto& res = static_cast<MiniMC::Model::Register&>(*content.res);

        if constexpr (op == MiniMC::Model::InstructionCode::Trunc ||
                      op == MiniMC::Model::InstructionCode::ZExt ||
                      op == MiniMC::Model::InstructionCode::SExt) {
          auto op1 = writeState.getStackControl().getValueLookup ().lookupValue(content.op1);
          switch (content.op1->getType()->getTypeID()) {
            case MiniMC::Model::TypeID::I8:
              writeState.getStackControl().getValueLookup ().saveValue(res, doOp<op, T>(op1.template as<typename T::I8>(), caster, res.getType()));
              break;
            case MiniMC::Model::TypeID::I16:
              writeState.getStackControl().getValueLookup ().saveValue(res, doOp<op, T>(op1.template as<typename T::I16>(), caster, res.getType()));
              break;
            case MiniMC::Model::TypeID::I32:
              writeState.getStackControl().getValueLookup ().saveValue(res, doOp<op, T>(op1.template as<typename T::I32>(), caster, res.getType()));
              break;

            case MiniMC::Model::TypeID::I64:
              writeState.getStackControl().getValueLookup ().saveValue(res, doOp<op, T>(op1.template as<typename T::I64>(), caster, res.getType()));
              break;
            default:
              throw MiniMC::Support::Exception("Invalid Trunc/Extenstion");
          }
          return MiniMC::VMT::Status::Ok;
        }

        else if constexpr (op == MiniMC::Model::InstructionCode::BoolSExt) {
          auto op1 = writeState.getStackControl().getValueLookup ().lookupValue(content.op1).template as<typename T::Bool>();
          switch (res.getType()->getTypeID()) {
            case MiniMC::Model::TypeID::I8:
              writeState.getStackControl().getValueLookup ().saveValue(res, caster.template BoolSExt<1>(op1));
              break;
            case MiniMC::Model::TypeID::I16:
              writeState.getStackControl().getValueLookup ().saveValue(res, caster.template BoolSExt<2>(op1));
              break;
            case MiniMC::Model::TypeID::I32:
              writeState.getStackControl().getValueLookup ().saveValue(res, caster.template BoolSExt<4>(op1));
              break;
            case MiniMC::Model::TypeID::I64:
              writeState.getStackControl().getValueLookup ().saveValue(res, caster.template BoolSExt<8>(op1));
              break;
            default:
              throw MiniMC::Support::Exception("Invalid Extenstion");
          }
        }

        else if constexpr (op == MiniMC::Model::InstructionCode::BoolZExt) {
          auto op1 = writeState.getStackControl().getValueLookup ().lookupValue(content.op1).template as<typename T::Bool> ();
          switch (res.getType()->getTypeID()) {
            case MiniMC::Model::TypeID::I8:
              writeState.getStackControl().getValueLookup ().saveValue(res, caster.template BoolZExt<1>(op1));
              break;
            case MiniMC::Model::TypeID::I16:
              writeState.getStackControl().getValueLookup ().saveValue(res, caster.template BoolZExt<2>(op1));
              break;
            case MiniMC::Model::TypeID::I32:
              writeState.getStackControl().getValueLookup ().saveValue(res, caster.template BoolZExt<4>(op1));
              break;
            case MiniMC::Model::TypeID::I64:
              writeState.getStackControl().getValueLookup ().saveValue(res, caster.template BoolZExt<8>(op1));
              break;
            default:
              throw MiniMC::Support::Exception("Invalid Extenstion");
          }
        }

        else if constexpr (op == MiniMC::Model::InstructionCode::IntToPtr) {
          auto op1 = writeState.getStackControl().getValueLookup ().lookupValue(content.op1);
	  T result;
	  if (res.getType ()->getTypeID () == MiniMC::Model::TypeID::Pointer) {
	    result = op1.visit ([&caster](const auto& val) -> T {
	      if constexpr (std::is_same_v<const typename T::I8&,decltype(val)> ||
			    std::is_same_v<const typename T::I16&,decltype(val)> ||
			    std::is_same_v<const typename T::I32&,decltype(val)> ||
			    std::is_same_v<const typename T::I64&,decltype(val)>
			    ) {
		return T{caster.IntToPtr (val)};
	      }
	      else {
		throw MiniMC::Support::Exception ("Shouldn't get her");
	      }
	    });
	    
	  }

	  else {
	    result = op1.visit ([&caster](const auto& val) -> T {
	      if constexpr (std::is_same_v<const typename T::I8&,decltype(val)> ||
			    std::is_same_v<const typename T::I16&,decltype(val)> ||
			    std::is_same_v<const typename T::I32&,decltype(val)> ||
			    std::is_same_v<const typename T::I64&,decltype(val)>
			    ) {
		return T{caster.IntToPtr32 (val)};
	      }
	      else {
		throw MiniMC::Support::Exception ("Shouldn't get her");
	      }
	     
	    });
	    
	  }
	  writeState.getStackControl().getValueLookup ().saveValue(res, std::move(result));
            
        }

        else if constexpr (op == MiniMC::Model::InstructionCode::IntToBool) {
          switch (content.op1->getType()->getTypeID()) {
            case MiniMC::Model::TypeID::I8: {
              auto op1 = writeState.getStackControl().getValueLookup ().lookupValue(content.op1).template as<typename T::I8>();

              writeState.getStackControl().getValueLookup ().saveValue(res, caster.IntToBool(op1));
            } break;
            case MiniMC::Model::TypeID::I16: {
              auto op1 = writeState.getStackControl().getValueLookup ().lookupValue(content.op1).template as<typename T::I16>();

              writeState.getStackControl().getValueLookup ().saveValue(res, caster.IntToBool(op1));
            } break;
            case MiniMC::Model::TypeID::I32: {
              auto op1 = writeState.getStackControl().getValueLookup ().lookupValue(content.op1).template as<typename T::I32>();

              writeState.getStackControl().getValueLookup ().saveValue(res, caster.IntToBool(op1));
            } break;
            case MiniMC::Model::TypeID::I64: {
              auto op1 = writeState.getStackControl().getValueLookup ().lookupValue(content.op1).template as<typename T::I64>();

              writeState.getStackControl().getValueLookup ().saveValue(res, caster.IntToBool(op1));
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
      inline Status runInstruction(const MiniMC::Model::Instruction& instr, VMState<T>& writeState, Operations&, Caster&, const MiniMC::Model::Program& prgm) requires MiniMC::Model::InstructionData<op>::isInternal {
        auto& content = instr.getOps<op>();

        if constexpr (op == MiniMC::Model::InstructionCode::Assign) {
          auto& res = static_cast<MiniMC::Model::Register&>(*content.res);
          auto op1 = writeState.getStackControl().getValueLookup ().lookupValue(content.op1);
          writeState.getStackControl().getValueLookup ().saveValue(res, std::move(op1));
          return Status::Ok;
        }

        else if constexpr (op == MiniMC::Model::InstructionCode::Call) {
          auto& scontrol = writeState.getStackControl();
	  assert(content.function->isConstant());
	  
	  auto func = MiniMC::Model::visitValue (
						 MiniMC::Model::Overload {
						   [&prgm](auto& t) -> MiniMC::Model::Function_ptr {
						     using TP = std::decay_t<decltype(t)>;
						     if constexpr (std::is_same_v<TP,MiniMC::Model::Pointer> ||
								   std::is_same_v<TP,MiniMC::Model::Pointer32>) {
						       auto loadPtr = t.getValue ();
						       auto func = prgm.getFunction(loadPtr.base);
						       return func;
						     }
						     else
						       throw MiniMC::Support::Exception("Shouldn't happen");
						   }}
						 ,*content.function
						 );
	  
	  auto& vstack = func->getRegisterStackDescr();
	  
	  std::vector<T> params;
          auto inserter = std::back_inserter(params);
          std::for_each(content.params.begin(), content.params.end(), [&inserter, &writeState](auto& v) { inserter = writeState.getStackControl().getValueLookup ().lookupValue(v); });
	  
	  auto res = content.res;
          scontrol.push(vstack.getTotalRegisters (), res);
          
	  
	  for (auto& v : vstack.getRegisters()) {
            writeState.getStackControl().getValueLookup().saveValue  (*v,writeState.getStackControl().getValueLookup().defaultValue (v->getType ()));
          }
	  
	  auto it = params.begin ();
	  for (auto& p : func->getParameters ()) {
	    writeState.getStackControl().getValueLookup().saveValue  (*p,std::move(*it));
	    ++it;
	      
	  }
	  
	  
          return Status::Ok;
        }

        else if constexpr (op == MiniMC::Model::InstructionCode::Ret) {
          auto ret = writeState.getStackControl().getValueLookup ().lookupValue(content.value);
          writeState.getStackControl().pop(std::move(ret));
          return Status::Ok;
        }

        else if constexpr (op == MiniMC::Model::InstructionCode::RetVoid) {
          writeState.getStackControl().popNoReturn();
          return Status::Ok;
        }

        else if constexpr (op == MiniMC::Model::InstructionCode::Skip) {
          return Status::Ok;
        }
	
	else if constexpr (op == MiniMC::Model::InstructionCode::NonDet) {
	  auto& res = static_cast<MiniMC::Model::Register&>(*content.res);
	  auto ret = writeState.getStackControl().getValueLookup ().unboundValue(content.res->getType ());
          writeState.getStackControl().getValueLookup ().saveValue(res,std::move(ret));
          return Status::Ok;
        }
	
	else {
          throw NotImplemented<op>();
        }
      }

      template <MiniMC::Model::InstructionCode op, class T, class Caster>
      inline Status runInstruction(const MiniMC::Model::Instruction&, VMState<T>&, Caster&, const MiniMC::Model::Program&) requires MiniMC::Model::InstructionData<op>::isPredicate {
        throw NotImplemented<op>();
        return Status::Ok;
      }

      template <MiniMC::Model::InstructionCode op, class T, class Operation, class Caster>
      inline Status runInstruction(const MiniMC::Model::Instruction& instr, VMState<T>& writeState, Operation& operations, Caster&, const MiniMC::Model::Program&) requires MiniMC::Model::InstructionData<op>::isAggregate {
        auto& content = instr.getOps<op>();
        auto& res = static_cast<MiniMC::Model::Register&>(*content.res);
        assert(content.offset->isConstant());
        auto offset_constant = std::static_pointer_cast<MiniMC::Model::Constant>(content.offset);
        MiniMC::BV64 offset{0};

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

          auto aggr = writeState.getStackControl().getValueLookup ().lookupValue(content.aggregate).template as<typename T::Aggregate>();
          auto value = writeState.getStackControl().getValueLookup ().lookupValue(val_v);

          switch (val_v->getType()->getTypeID()) {
            case MiniMC::Model::TypeID::I8:
              writeState.getStackControl().getValueLookup ().saveValue(res, operations.template InsertBaseValue<typename T::I8>(aggr, offset, value.template as<typename T::I8>()));
              break;
            case MiniMC::Model::TypeID::I16:
              writeState.getStackControl().getValueLookup ().saveValue(res, operations.template InsertBaseValue<typename T::I16>(aggr, offset, value.template as<typename T::I16>()));
              break;
            case MiniMC::Model::TypeID::I32:
              writeState.getStackControl().getValueLookup ().saveValue(res, operations.template InsertBaseValue<typename T::I32>(aggr, offset, value.template as<typename T::I32>()));
              break;
            case MiniMC::Model::TypeID::I64:
              writeState.getStackControl().getValueLookup ().saveValue(res, operations.template InsertBaseValue<typename T::I64>(aggr, offset, value.template as<typename T::I64>()));
              break;
            case MiniMC::Model::TypeID::Pointer:
              writeState.getStackControl().getValueLookup ().saveValue(res, operations.template InsertBaseValue<typename T::Pointer>(aggr, offset, value.template as<typename T::Pointer>()));
              break;
            case MiniMC::Model::TypeID::Array:
            case MiniMC::Model::TypeID::Struct:
              writeState.getStackControl().getValueLookup ().saveValue(res, operations.template InsertAggregateValue(aggr, offset, value.template as<typename T::Aggregate>()));
              break;
            default:
              throw MiniMC::Support::Exception("Invalid Insert");
          }
        }

        else if constexpr (op == MiniMC::Model::InstructionCode::ExtractValue) {
          auto aggr = writeState.getStackControl().getValueLookup ().lookupValue(content.aggregate).template as<typename T::Aggregate>();

          switch (res.getType()->getTypeID()) {
            case MiniMC::Model::TypeID::I8:
              writeState.getStackControl().getValueLookup ().saveValue(res, operations.template ExtractBaseValue<typename T::I8>(aggr, offset));
              break;
            case MiniMC::Model::TypeID::I16:
              writeState.getStackControl().getValueLookup ().saveValue(res, operations.template ExtractBaseValue<typename T::I16>(aggr, offset));
              break;
            case MiniMC::Model::TypeID::I32:
              writeState.getStackControl().getValueLookup ().saveValue(res, operations.template ExtractBaseValue<typename T::I32>(aggr, offset));
              break;
            case MiniMC::Model::TypeID::I64:
              writeState.getStackControl().getValueLookup ().saveValue(res, operations.template ExtractBaseValue<typename T::I64>(aggr, offset));
              break;

            case MiniMC::Model::TypeID::Pointer:
              writeState.getStackControl().getValueLookup ().saveValue(res, operations.template ExtractBaseValue<typename T::Pointer>(aggr, offset));
              break;
            case MiniMC::Model::TypeID::Array:
            case MiniMC::Model::TypeID::Struct:
              writeState.getStackControl().getValueLookup ().saveValue(res, operations.ExtractAggregateValue(aggr, offset, res.getType()->getSize()));
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
                                                  VMState<T>& wstate) {
      auto end = instr.end();
      Status status = Status::Ok;
      auto it = instr.begin ();
      for ( it = instr.begin(); it != end && status == Status::Ok; ++it) {
	switch (it->getOpcode()) {
	  
#define X(OP)								\
  case MiniMC::Model::InstructionCode::OP:                                                                                                \
    status = Impl::runInstruction<MiniMC::Model::InstructionCode::OP, T, Operations, Caster>(*it, wstate, operations, caster,prgm); \
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
