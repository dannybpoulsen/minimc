#ifndef _ENGINE_IMPLE__
#define _ENGINE_IMPLE__

#include "model/cfg.hpp"
#include "model/valuevisitor.hpp"
#include "vm/value.hpp"
#include "vm/vmt.hpp"
#include "support/overload.hpp"

namespace MiniMC {
  namespace VMT {

    template<class T,class R>
    concept Integer = std::is_same_v<R,typename T::I8> || std::is_same_v<R,typename T::I16> || std::is_same_v<R,typename T::I32> || std::is_same_v<R,typename T::I64>;

    template<class T,class R>
    concept Pointer = std::is_same_v<R,typename T::Pointer> || std::is_same_v<R,typename T::Pointer32>;

    
    template<class State, typename Operations, typename Caster>
    struct Impl {
      using T = typename State::Domain;
      template <class I>
      static Status runInstruction(const I&, State&, Operations&, Caster&, const MiniMC::Model::Program&)  {
	return Status::Ok;
	//throw NotImplemented<opc>();
      }

      
      template <class Value>
      static auto castPtrToAppropriateInteger(const Value& v, Caster& caster) {
        if constexpr (std::is_same_v<Value, typename T::Pointer>) {
          return caster.template PtrToInt<typename T::I64>(v);
        } else if constexpr (std::is_same_v<Value, typename T::Pointer32>) {
          return caster.template Ptr32ToInt<typename T::I32>(v);
        } else {
          return v;
        }
      }
      

      
      template <class I>
      static Status runInstruction(const I& instr, State& writeState, Operations& operations, Caster&, const MiniMC::Model::Program&)
        requires MiniMC::Model::isTAC_v<I> &&
	IntOperationCompatible<typename T::I8, typename T::Bool, Operations> &&
	IntOperationCompatible<typename T::I16, typename T::Bool, Operations> &&
	IntOperationCompatible<typename T::I32, typename T::Bool, Operations> &&
	IntOperationCompatible<typename T::I64, typename T::Bool, Operations> &&
	ValueLookupable<State>

      {
	constexpr auto op = I::getOpcode ();
        auto& content = instr.getOps ();
        auto& res = static_cast<MiniMC::Model::Register&>(*content.res);

        auto lval = writeState.getValueLookup().lookupValue(*content.op1);
        auto rval = writeState.getValueLookup().lookupValue(*content.op2);
	lval.visit (MiniMC::Support::Overload {
	    [&writeState,&operations,&res]<typename R>(R& lval, R& rval) requires Integer<T,R> {
	      
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
	       else
		throw NotImplemented<op>();
		},
	     [] (auto&, auto&) {
	       throw NotImplemented<op>();
	     }
	      },
	  rval
	  );
	return Status::Ok;
	  
      }
	  
      template <class I>
      static Status runInstruction(const I& instr, State& writeState, Operations& operations, Caster& caster, const MiniMC::Model::Program&)
        requires MiniMC::Model::isComparison_v<I> &&
                 IntOperationCompatible<typename T::I8, typename T::Bool, Operations> &&
                 IntOperationCompatible<typename T::I16, typename T::Bool, Operations> &&
                 IntOperationCompatible<typename T::I32, typename T::Bool, Operations> &&
                 IntOperationCompatible<typename T::I64, typename T::Bool, Operations> &&
                 ValueLookupable<State>

      {
	constexpr auto op = I::getOpcode ();
        auto& content = instr.getOps ();
        auto& res = static_cast<MiniMC::Model::Register&>(*content.res);

        auto lval = writeState.getValueLookup().lookupValue(*content.op1);
        auto rval = writeState.getValueLookup().lookupValue(*content.op2);
	lval.visit (MiniMC::Support::Overload {
	    [&writeState,&operations,&res,&caster]<typename R>(R& l, R& r) requires Integer<T,R> || Pointer<T,R> {
	      auto lval = Impl::castPtrToAppropriateInteger (l,caster);
	      auto rval = Impl::castPtrToAppropriateInteger (r,caster);
	      
	      if constexpr (op == MiniMC::Model::InstructionCode::ICMP_SGT)
	        writeState.getValueLookup().saveValue(res, operations.SGt(lval, rval));
	      else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_SGE) {
		writeState.getValueLookup().saveValue(res, operations.SGe(lval, rval));
	      }
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
		},
	     [] (auto&, auto&) {
	       throw NotImplemented<op>();
	     }
	   }
	  
	  ,rval);
	return Status::Ok;
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

        auto addrConverter = MiniMC::Support::Overload {
	  [](typename T::Pointer& addrVal) {
	    return addrVal;
	  },
	  [&caster] (typename T::Pointer32& addrVal) {
	    return caster.Ptr32ToPtr (addrVal);
	  },
	  [](auto& )->T::Pointer { throw MiniMC::Support::Exception("SHouldn't get here");
	  }

	};
	
        if constexpr (op == MiniMC::Model::InstructionCode::PtrAdd) {
          auto ptr = writeState.getValueLookup().lookupValue(*content.ptr).visit (addrConverter);;
	  auto visitor = MiniMC::Support::Overload {
	    [&operations,&writeState,&ptr,&res]<typename ValT>(ValT& skipsize,ValT& nbskips) requires Integer<T,ValT> {
	      auto totalskip = operations.Mul(skipsize, nbskips);
	      writeState.getValueLookup().saveValue(res, operations.PtrAdd(ptr, totalskip));
	      return Status::Ok;
	    },
	    [](auto&,  auto& )->Status {      throw MiniMC::Support::Exception("Invalid Skip-type type");
	    }
	  };
	  return writeState.getValueLookup ().lookupValue(*content.skipsize).visit (visitor,
										    writeState.getValueLookup().lookupValue(*content.nbSkips)
										    );
	  
        }
        if constexpr (op == MiniMC::Model::InstructionCode::PtrSub) {
          auto ptr = writeState.getValueLookup().lookupValue(*content.ptr).visit (addrConverter);;
	  auto visitor = MiniMC::Support::Overload {
	    [&operations,&writeState,&ptr,&res]<typename ValT>(ValT& skipsize,ValT& nbskips) requires Integer<T,ValT> {
	      auto totalskip = operations.Mul(skipsize, nbskips);
	      writeState.getValueLookup().saveValue(res, operations.PtrSub(ptr, totalskip));
	      return Status::Ok;
	    },
	    [](auto&,  auto& )->Status {      throw MiniMC::Support::Exception("Invalid Skip-type type");
	    }
	  };
	  return writeState.getValueLookup ().lookupValue(*content.skipsize).visit (visitor,
										    writeState.getValueLookup().lookupValue(*content.nbSkips)
										    ); 
        } else if constexpr (op == MiniMC::Model::InstructionCode::PtrEq) {
          auto lval = writeState.getValueLookup().lookupValue(*content.op1).visit (addrConverter);
          auto rval = writeState.getValueLookup().lookupValue(*content.op2).visit (addrConverter);
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

	auto addrConverter = MiniMC::Support::Overload {
	  [](typename T::Pointer& addrVal) {
	    return addrVal;
	  },
	  [&caster] (typename T::Pointer32& addrVal) {
	    return caster.Ptr32ToPtr (addrVal);
	  },
	  [](auto& )->T::Pointer { throw MiniMC::Support::Exception("SHouldn't get here");
	  }

	};
	
        if constexpr (op == MiniMC::Model::InstructionCode::Load ) {
	  auto& res = static_cast<MiniMC::Model::Register&>(*content.res);
	  if constexpr (MemoryControllable<State>) {
	    auto addr = writeState.getValueLookup().lookupValue(*content.addr).visit (addrConverter);
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
	    auto addr = writeState.getValueLookup().lookupValue(*content.addr).visit(addrConverter);
	    
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

      
      template <MiniMC::Model::InstructionCode opc, class LeftOp, MiniMC::Model::TypeID to>
      static T doCastOp(const LeftOp& op, Caster& caster) {
	constexpr auto bw = MiniMC::Model::BitWidth<to>; 
	if constexpr (opc == MiniMC::Model::InstructionCode::Trunc) {
          if constexpr (bw  > LeftOp::intbitsize()) {
            throw MiniMC::Support::Exception("Invalid Truntion");
          } else
            return caster.template Trunc<to, LeftOp>(op);
        } else if constexpr (opc == MiniMC::Model::InstructionCode::ZExt) {
          if constexpr (bw  < LeftOp::intbitsize()) {
            throw MiniMC::Support::Exception("Invalid Extension");
          } else
            return caster.template ZExt<to, LeftOp>(op);
        } else if constexpr (opc == MiniMC::Model::InstructionCode::SExt) {
          if constexpr (bw < LeftOp::intbitsize()) {
            throw MiniMC::Support::Exception("Invalid Extension");
          } else
            return caster.template SExt<to, LeftOp>(op);
        } else {
          []<bool b = false>() { static_assert(b); }
          ();
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
	  
	  auto result = op1.visit (MiniMC::Support::Overload {
	      [&op1,&res,&caster,&writeState]<typename K>(K& val) -> T requires Integer<T,K> {
		switch (res.getType ()->getTypeID ()) {
		  case MiniMC::Model::TypeID::I8:
		     return doCastOp<op, K, MiniMC::Model::TypeID::I8>(val, caster);
		  case MiniMC::Model::TypeID::I16:
		     return doCastOp<op, K, MiniMC::Model::TypeID::I16>(val, caster);
		  case MiniMC::Model::TypeID::I32:
		     return doCastOp<op, K, MiniMC::Model::TypeID::I32>(val, caster);
		  case MiniMC::Model::TypeID::I64:
 		     return doCastOp<op, K, MiniMC::Model::TypeID::I64>(val, caster);
		  default:
		     throw MiniMC::Support::Exception("Error");
		}
	      },
	      [](auto& ) -> T {throw MiniMC::Support::Exception("Invalid Trunc/Extenstion");}
	    
		}
	    );
	  writeState.getValueLookup().saveValue(res, std::move(result));
	    
	  return MiniMC::VMT::Status::Ok;
	}

        else if constexpr (op == MiniMC::Model::InstructionCode::BoolSExt) {
          auto op1 = writeState.getValueLookup().lookupValue(*content.op1).template as<typename T::Bool>();
          switch (res.getType()->getTypeID()) {
            case MiniMC::Model::TypeID::I8:
              writeState.getValueLookup().saveValue(res, caster.template BoolSExt<MiniMC::Model::TypeID::I8>(op1));
              break;
            case MiniMC::Model::TypeID::I16:
              writeState.getValueLookup().saveValue(res, caster.template BoolSExt<MiniMC::Model::TypeID::I16>(op1));
              break;
            case MiniMC::Model::TypeID::I32:
              writeState.getValueLookup().saveValue(res, caster.template BoolSExt<MiniMC::Model::TypeID::I32>(op1));
              break;
            case MiniMC::Model::TypeID::I64:
              writeState.getValueLookup().saveValue(res, caster.template BoolSExt<MiniMC::Model::TypeID::I64>(op1));
              break;
            default:
              throw MiniMC::Support::Exception("Invalid Extenstion");
          }
        }

        else if constexpr (op == MiniMC::Model::InstructionCode::BoolZExt) {
          auto op1 = writeState.getValueLookup().lookupValue(*content.op1).template as<typename T::Bool>();
          switch (res.getType()->getTypeID()) {
            case MiniMC::Model::TypeID::I8:
              writeState.getValueLookup().saveValue(res, caster.template BoolZExt<MiniMC::Model::TypeID::I8>(op1));
              break;
            case MiniMC::Model::TypeID::I16:
              writeState.getValueLookup().saveValue(res, caster.template BoolZExt<MiniMC::Model::TypeID::I16>(op1));
              break;
            case MiniMC::Model::TypeID::I32:
              writeState.getValueLookup().saveValue(res, caster.template BoolZExt<MiniMC::Model::TypeID::I32>(op1));
              break;
            case MiniMC::Model::TypeID::I64:
              writeState.getValueLookup().saveValue(res, caster.template BoolZExt<MiniMC::Model::TypeID::I64>(op1));
              break;
            default:
              throw MiniMC::Support::Exception("Invalid Extenstion");
          }
        }

        else if constexpr (op == MiniMC::Model::InstructionCode::IntToPtr) {
          auto op1 = writeState.getValueLookup().lookupValue(*content.op1);
          T result = op1.visit (MiniMC::Support::Overload {
	      [&caster,&res]<typename K>(K& val) requires Integer<T,K> {
		if (res.getType ()->getTypeID () == MiniMC::Model::TypeID::Pointer) {
		  return T{caster.IntToPtr(val)};
		}
		else {
		  return T{caster.IntToPtr32(val)};
		}
	      },
	      [](auto&)->T { throw MiniMC::Support::Exception("Shouldn't get her");}
	    }
	    );
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
	  if constexpr (SimpStackControllable<State> || StackControllable<State> ) {
	    writeState.getStackControl().popNoReturn();
	    return Status::Ok;
	  }
	  else
	   throw NotImplemented<op> {};
	  
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
            case MiniMC::Model::TypeID::Aggregate:
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
            case MiniMC::Model::TypeID::Aggregate:
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

    template <class Operations, class Caster>
    template<class State>
    Status Engine<Operations, Caster>::execute(const MiniMC::Model::Instruction& instr,
						      State& wstate) {
      
      return instr.visit ([this,&wstate](auto& t) {return Impl<State,Operations,Caster>::template runInstruction (t, wstate, operations, caster, prgm);});

    }

    template <class Operations, class Caster>
    template<class State>
    Status Engine<Operations, Caster>::execute(const MiniMC::Model::InstructionStream& instr,
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
