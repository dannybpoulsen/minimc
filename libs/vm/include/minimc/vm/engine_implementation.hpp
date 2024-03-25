#ifndef _ENGINE_IMPLE__
#define _ENGINE_IMPLE__

#include "minimc/model/cfg.hpp"
#include "minimc/model/valuevisitor.hpp"
#include "minimc/vm/value.hpp"
#include "minimc/vm/vmt.hpp"
#include "minimc/support/overload.hpp"

namespace MiniMC {
  namespace VMT {

    template<class T,class R>
    concept Integer = std::is_same_v<R,typename T::I8> || std::is_same_v<R,typename T::I16> || std::is_same_v<R,typename T::I32> || std::is_same_v<R,typename T::I64>;

    template<class T,class R>
    concept Boolean = std::is_same_v<R,typename T::Bool>;    
    
    template<class T,class R>
    concept Pointer = std::is_same_v<R,typename T::Pointer> || std::is_same_v<R,typename T::Pointer32>;
    
    template<class Value,RegisterStore<Value> RegStore,Ops<Value> Operations>
    class Evaluator {
      Evaluator (const Operations& ops, const RegStore& regstore) : ops(ops),regstore(regstore) {}

      Value Eval (MiniMC::Model::Value& v) {
	MiniMC::Model::visitValue(*this,v);
      }
      
      template<class T>
      Value operator() (const T& t) const requires (!std::is_same_v<T,MiniMC::Model::Register> && !std::is_same_v<T,MiniMC::Model::AddExpr>) {
	return ops.create(t);
      }

      Value operator() (const MiniMC::Model::AddExpr&) const  {
	throw MiniMC::Support::Exception ("H");
      }
      
      Value operator() (const MiniMC::Model::Register& reg) const  {
	return regstore.lookupValue (reg);
      }
      
      
    private:
      const Operations& ops;
      const RegStore& regstore;
    };
    
    template<typename T, Ops<T> Operations>
    struct Engine<T,Operations>::Impl {
    private:
      const MiniMC::Model::Program& prgm;
      Operations operations;
    public:
      Impl (Operations&& operations, const MiniMC::Model::Program& prgm) : prgm(prgm),operations(operations) {}
      template <class I,class State>
      static Status runInstruction(const I&, State&)  {
	throw NotImplemented<I::getOpcode()> ();
      }

      
      template <class Value>
      auto castPtrToAppropriateInteger(const Value& v) {
        if constexpr (std::is_same_v<Value, typename T::Pointer>) {
          return operations.template PtrToInt<typename T::I64>(v);
        } else if constexpr (std::is_same_v<Value, typename T::Pointer32>) {
          return operations.template Ptr32ToInt<typename T::I32>(v);
        } else {
          return v;
        }
      }
      

      
      template <class I,VMState<T> State>
      Status runInstruction(const I& instr, State& state) requires MiniMC::Model::isTAC_v<I> 

      {
	constexpr auto op = I::getOpcode ();
        auto& content = instr.getOps ();
        auto& res = content.res->asRegister ();

        auto lval = state.getValueLookup().lookupValue(*content.op1);
        auto rval = state.getValueLookup().lookupValue(*content.op2);
	T::visit (MiniMC::Support::Overload {
	    [&state,this,&res]<typename R>(R& lval, R& rval) requires Integer<T,R> {
	      
	      if constexpr (op == MiniMC::Model::InstructionCode::Add)
	         state.getValueLookup().saveValue(res, operations.Add(lval, rval));
	      else if constexpr (op == MiniMC::Model::InstructionCode::Sub)
	         state.getValueLookup().saveValue(res, operations.Sub(lval, rval));
	      else if constexpr (op == MiniMC::Model::InstructionCode::Mul)
	        state.getValueLookup().saveValue(res, operations.Mul(lval, rval));
	      else if constexpr (op == MiniMC::Model::InstructionCode::UDiv)
	        state.getValueLookup().saveValue(res, operations.UDiv(lval, rval));
	      else if constexpr (op == MiniMC::Model::InstructionCode::SDiv)
	        state.getValueLookup().saveValue(res, operations.SDiv(lval, rval));
	      else if constexpr (op == MiniMC::Model::InstructionCode::Shl)
	        state.getValueLookup().saveValue(res, operations.LShl(lval, rval));
	      else if constexpr (op == MiniMC::Model::InstructionCode::LShr)
	        state.getValueLookup().saveValue(res, operations.LShr(lval, rval));
	      else if constexpr (op == MiniMC::Model::InstructionCode::AShr)
	        state.getValueLookup().saveValue(res, operations.AShr(lval, rval));
	      else if constexpr (op == MiniMC::Model::InstructionCode::And)
	        state.getValueLookup().saveValue(res, operations.And(lval, rval));
	      else if constexpr (op == MiniMC::Model::InstructionCode::Or)
	        state.getValueLookup().saveValue(res, operations.Or(lval, rval));
	      else if constexpr (op == MiniMC::Model::InstructionCode::Xor)
	        state.getValueLookup().saveValue(res, operations.Xor(lval, rval));
	      else
		throw NotImplemented<op>();
	    },
	      [] (auto&, auto&) {
		throw NotImplemented<op>();
	      }
	      },
	  lval,
	  rval
	  );
	return Status::Ok;
	  
      }
	  
      template <class I, VMState<T> State>
      Status runInstruction(const I& instr, State& state) requires MiniMC::Model::isComparison_v<I>
      {
	constexpr auto op = I::getOpcode ();
        auto& content = instr.getOps ();
        auto& res = content.res->asRegister ();

        auto lval = state.getValueLookup().lookupValue(*content.op1);
        auto rval = state.getValueLookup().lookupValue(*content.op2);
	T::visit (MiniMC::Support::Overload {
	    [&state,this,&res]<typename R>(R& l, R& r) requires Integer<T,R> || Pointer<T,R> {
	      auto lval = Impl::castPtrToAppropriateInteger (l);
	      auto rval = Impl::castPtrToAppropriateInteger (r);
	      
	      if constexpr (op == MiniMC::Model::InstructionCode::ICMP_SGT)
		state.getValueLookup().saveValue(res, operations.SGt(lval, rval));
	      else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_SGE) 
		state.getValueLookup().saveValue(res, operations.SGe(lval, rval));
	      else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_SLE)
	         state.getValueLookup().saveValue(res, operations.SLe(lval, rval));
	      else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_SLT)
	         state.getValueLookup().saveValue(res, operations.SLt(lval, rval));
	      else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_UGT)
	         state.getValueLookup().saveValue(res, operations.UGt(lval, rval));
	      else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_UGE)
	         state.getValueLookup().saveValue(res, operations.UGe(lval, rval));
	      else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_ULE)
	         state.getValueLookup().saveValue(res, operations.ULe(lval, rval));
	      else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_ULT)
	         state.getValueLookup().saveValue(res, operations.ULt(lval, rval));
	      else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_EQ)
	         state.getValueLookup().saveValue(res, operations.Eq(lval, rval));
	      else if constexpr (op == MiniMC::Model::InstructionCode::ICMP_NEQ)
	         state.getValueLookup().saveValue(res, operations.NEq(lval, rval));
	      
	      else
		throw NotImplemented<op>();
		},
	     [] (auto&, auto&) {
	       throw NotImplemented<op>();
	     }
	      },
	  lval
	  ,rval);
	return Status::Ok;
      }

      template <class I,VMState<T> State>
      Status runInstruction(const I& instr, State& state) requires MiniMC::Model::isPointer_v<I> 
      {
	constexpr auto op = I::getOpcode ();
        auto& content = instr.getOps ();
        auto& res = content.res->asRegister ();

        auto addrConverter = MiniMC::Support::Overload {
	  [](typename T::Pointer& addrVal) {
	    return addrVal;
	  },
	  [this] (typename T::Pointer32& addrVal) {
	    return operations.Ptr32ToPtr (addrVal);
	  },
	  [](auto& )->T::Pointer { throw MiniMC::Support::Exception("SHouldn't get here");
	  }

	};
	
        if constexpr (op == MiniMC::Model::InstructionCode::PtrAdd) {
          auto ptr = T::visit (addrConverter,state.getValueLookup().lookupValue(*content.ptr));;
	  auto visitor = MiniMC::Support::Overload {
	    [this,&state,&ptr,&res]<typename ValT>(ValT& skipsize,ValT& nbskips) requires Integer<T,ValT> {
	      auto totalskip = operations.Mul(skipsize, nbskips);
	      state.getValueLookup().saveValue(res, operations.PtrAdd(ptr, totalskip));
	      return Status::Ok;
	    },
	    [](auto&,  auto& )->Status {      throw MiniMC::Support::Exception("Invalid Skip-type type");
	    }
	  };
	  return T::visit (visitor,
			   state.getValueLookup ().lookupValue(*content.skipsize),
			   state.getValueLookup().lookupValue(*content.nbSkips)
										    );
	  
        }
        if constexpr (op == MiniMC::Model::InstructionCode::PtrSub) {
          auto ptr = T::visit (addrConverter,state.getValueLookup().lookupValue(*content.ptr));
	  auto visitor = MiniMC::Support::Overload {
	    [this,&state,&ptr,&res]<typename ValT>(ValT& skipsize,ValT& nbskips) requires Integer<T,ValT> {
	      auto totalskip = operations.Mul(skipsize, nbskips);
	      state.getValueLookup().saveValue(res, operations.PtrSub(ptr, totalskip));
	      return Status::Ok;
	    },
	    [](auto&,  auto& )->Status {      throw MiniMC::Support::Exception("Invalid Skip-type type");
	    }
	  };
	  return T::visit (visitor,
			   state.getValueLookup ().lookupValue(*content.skipsize),
			   state.getValueLookup().lookupValue(*content.nbSkips)
										    ); 
        } else if constexpr (op == MiniMC::Model::InstructionCode::PtrEq) {
          auto lval = T::visit (addrConverter,state.getValueLookup().lookupValue(*content.op1));
          auto rval = T::visit (addrConverter,state.getValueLookup().lookupValue(*content.op2));
          state.getValueLookup().saveValue(res, operations.PtrEq(lval, rval));
          return Status::Ok;
        }
        throw NotImplemented<op>();
      }
      
      template <class  I, VMState<T> State>
      Status runInstruction(const I& instr, State& state)
        requires MiniMC::Model::isMemory_v<I> {
	constexpr auto op = I::getOpcode ();
        auto& content = instr.getOps();

	auto addrConverter = MiniMC::Support::Overload {
	  [](typename T::Pointer& addrVal) {
	    return addrVal;
	  },
	  [this] (typename T::Pointer32& addrVal) {
	    return operations.Ptr32ToPtr (addrVal);
	  },
	  [](auto& )->T::Pointer { throw MiniMC::Support::Exception("SHouldn't get here");
	  }

	};
	
        if constexpr (op == MiniMC::Model::InstructionCode::Load ) {
	  auto& res = content.res->asRegister ();
	  auto addr = T::visit (addrConverter,state.getValueLookup().lookupValue(*content.addr));
	  state.getValueLookup().saveValue(res, state.getMemory().load(addr, res.getType()));
	  return Status::Ok;
	  
	  
	}
	
        else if constexpr (op == MiniMC::Model::InstructionCode::Store) {
	  if constexpr (HasMemory<State,T>) {
	    auto value = state.getValueLookup().lookupValue(*content.storee);
	    auto addr = T::visit(addrConverter,state.getValueLookup().lookupValue(*content.addr));
	    
	    T::visit([&state, &addr](const auto& t) {
	      if constexpr (!std::is_same_v<const typename T::Bool&, decltype(t)>)
		state.getMemory().store(addr, t);
	      else {
		throw MiniMC::Support::Exception("Cannot Store this type");
            }
	    },
	      value
	      );
	    
	  }
	  return Status::Ok;
	  
	}
	  
	else {
	  throw NotImplemented<op>();
	}
	  
      }
      
      template <class I,VMState<T> State>
      Status runInstruction(const I& instr, State& state) requires MiniMC::Model::isAssertAssume_v<I>       {
	constexpr auto op = instr.getOpcode ();
	auto& content = instr.getOps();
	  
	auto obj = T::visit (MiniMC::Support::Overload {
	    [](const T::Bool& b) {return b;},
	    [](const auto&) -> typename T::Bool {throw MiniMC::Support::Exception ("Should be a boolean");}
	  },
	  state.getValueLookup().lookupValue(*content.expr)
	  );
	
	auto& pathcontrol = state.getPathControl();
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
      static T doCastOp(const LeftOp& op, Operations& ops) {
	constexpr auto bw = MiniMC::Model::BitWidth<to>; 
	if constexpr (opc == MiniMC::Model::InstructionCode::Trunc) {
          if constexpr (bw  > LeftOp::intbitsize()) {
            throw MiniMC::Support::Exception("Invalid Truntion");
          } else
            return ops.template Trunc<to, LeftOp>(op);
        } else if constexpr (opc == MiniMC::Model::InstructionCode::ZExt) {
          if constexpr (bw  < LeftOp::intbitsize()) {
            throw MiniMC::Support::Exception("Invalid Extension");
          } else
            return ops.template ZExt<to, LeftOp>(op);
        } else if constexpr (opc == MiniMC::Model::InstructionCode::SExt) {
          if constexpr (bw < LeftOp::intbitsize()) {
            throw MiniMC::Support::Exception("Invalid Extension");
          } else
            return ops.template SExt<to, LeftOp>(op);
        } else {
          []<bool b = false>() { static_assert(b); }
          ();
        }
      }

     
      template <class I,VMState<T> State>
      Status runInstruction(const I& instr, State& state) requires MiniMC::Model::isCast_v<I>
      {
	constexpr auto op = I::getOpcode ();
        auto& content = instr.getOps ();
        auto& res = content.res->asRegister ();

        if constexpr (op == MiniMC::Model::InstructionCode::Trunc ||
                      op == MiniMC::Model::InstructionCode::ZExt ||
                      op == MiniMC::Model::InstructionCode::SExt) {
          auto op1 = state.getValueLookup().lookupValue(*content.op1);
	  
	  auto result = T::visit (MiniMC::Support::Overload {
	      [&op1,&res,this,&state]<typename K>(K& val) -> T requires Integer<T,K> {
		switch (res.getType ()->getTypeID ()) {
		case MiniMC::Model::TypeID::I8:
		return doCastOp<op, K, MiniMC::Model::TypeID::I8>(val, operations);
		case MiniMC::Model::TypeID::I16:
		return doCastOp<op, K, MiniMC::Model::TypeID::I16>(val, operations);
		case MiniMC::Model::TypeID::I32:
		return doCastOp<op, K, MiniMC::Model::TypeID::I32>(val, operations);
		case MiniMC::Model::TypeID::I64:
		return doCastOp<op, K, MiniMC::Model::TypeID::I64>(val, operations);
		default:
		throw MiniMC::Support::Exception("Error");
		}
	      },
		[](auto& ) -> T {throw MiniMC::Support::Exception("Invalid Trunc/Extenstion");}
		
		},
	    op1
	    );
	  state.getValueLookup().saveValue(res, std::move(result));
	    
	  return MiniMC::VMT::Status::Ok;
	}

        else if constexpr (op == MiniMC::Model::InstructionCode::BoolSExt) {
          auto op1 = T::visit (MiniMC::Support::Overload {
	      [](const typename T::Bool& b) {return b;},
	      [](const auto& ) -> T::Bool {throw MiniMC::Support::Exception ("Must be bool");},
		},
	    state.getValueLookup().lookupValue(*content.op1)
	    );
	  switch (res.getType()->getTypeID()) {
            case MiniMC::Model::TypeID::I8:
              state.getValueLookup().saveValue(res, operations.template BoolSExt<MiniMC::Model::TypeID::I8>(op1));
              break;
            case MiniMC::Model::TypeID::I16:
              state.getValueLookup().saveValue(res, operations.template BoolSExt<MiniMC::Model::TypeID::I16>(op1));
              break;
            case MiniMC::Model::TypeID::I32:
              state.getValueLookup().saveValue(res, operations.template BoolSExt<MiniMC::Model::TypeID::I32>(op1));
              break;
            case MiniMC::Model::TypeID::I64:
              state.getValueLookup().saveValue(res, operations.template BoolSExt<MiniMC::Model::TypeID::I64>(op1));
              break;
            default:
              throw MiniMC::Support::Exception("Invalid Extenstion");
          }
        }

        else if constexpr (op == MiniMC::Model::InstructionCode::BoolZExt) {
          auto op1 = T::visit (MiniMC::Support::Overload {
	      [](const typename T::Bool& b) {return b;},
		[](const auto& ) -> T::Bool {throw MiniMC::Support::Exception ("Must be bool");},
		},
	    state.getValueLookup().lookupValue(*content.op1)
	    );
	  
	  switch (res.getType()->getTypeID()) {
            case MiniMC::Model::TypeID::I8:
              state.getValueLookup().saveValue(res, operations.template BoolZExt<MiniMC::Model::TypeID::I8>(op1));
              break;
            case MiniMC::Model::TypeID::I16:
              state.getValueLookup().saveValue(res, operations.template BoolZExt<MiniMC::Model::TypeID::I16>(op1));
              break;
            case MiniMC::Model::TypeID::I32:
              state.getValueLookup().saveValue(res, operations.template BoolZExt<MiniMC::Model::TypeID::I32>(op1));
              break;
            case MiniMC::Model::TypeID::I64:
              state.getValueLookup().saveValue(res, operations.template BoolZExt<MiniMC::Model::TypeID::I64>(op1));
              break;
            default:
              throw MiniMC::Support::Exception("Invalid Extenstion");
          }
        }

        else if constexpr (op == MiniMC::Model::InstructionCode::IntToPtr) {
          auto op1 = state.getValueLookup().lookupValue(*content.op1);
          T result = T::visit (MiniMC::Support::Overload {
	      [this,&res]<typename K>(K& val) requires Integer<T,K> {
		if (res.getType ()->getTypeID () == MiniMC::Model::TypeID::Pointer) {
		  return T{operations.IntToPtr(val)};
		}
		else {
		  return T{operations.IntToPtr32(val)};
		}
	      },
	      [](auto&)->T { throw MiniMC::Support::Exception("Shouldn't get her");}
		},
	    op1
	    );
	  state.getValueLookup().saveValue(res, std::move(result));
	  
        }

        else if constexpr (op == MiniMC::Model::InstructionCode::IntToBool) {
	  auto resVal = T::visit (  MiniMC::Support::Overload {
	      [this](const typename T::I8 v)->T::Bool {return operations.IntToBool (v);},
		[this](const typename T::I16 v)->T::Bool {return operations.IntToBool (v);},
		[this](const typename T::I32 v)->T::Bool {return operations.IntToBool (v);},
		[this](const typename T::I64 v)->T::Bool {return operations.IntToBool (v);},
		[](auto& ) ->T::Bool {throw MiniMC::Support::Exception ("Must be integer");}
		},
	    state.getValueLookup().lookupValue (*content.op1)
										   
										   );
	  state.getValueLookup().saveValue(res, resVal);
	  
	}
	
        else
          throw NotImplemented<op>();
        return Status::Ok;
      }

      template <class I,VMState<T> State>
      Status runInstruction(const I& instr, State& state)
        requires MiniMC::Model::isInternal_v<I>
      {
	constexpr auto op = I::getOpcode ();
        

        if constexpr (op == MiniMC::Model::InstructionCode::Assign ) {
	  auto& content = instr.getOps();
	  auto& res = content.res->asRegister ();
          auto op1 = state.getValueLookup().lookupValue(*content.op1);
          state.getValueLookup().saveValue(res, std::move(op1));
          return Status::Ok;
        }

        else if constexpr (op == MiniMC::Model::InstructionCode::Call) {
	  auto& content = instr.getOps();
	  auto& scontrol = state.getStackControl();
          assert(content.function->isConstant());
	  
          auto func = MiniMC::Model::visitValue<MiniMC::Model::Function_ptr>(
									     MiniMC::Support::Overload{
									       [this](const MiniMC::Model::Pointer& t) -> MiniMC::Model::Function_ptr {
										 auto loadPtr = t.getValue();
										 auto func = prgm.getFunction(loadPtr.base);
										 return func;
									       },
										 [this](const MiniMC::Model::Pointer32& t) -> MiniMC::Model::Function_ptr {
										   auto loadPtr = t.getValue();
										   auto func = prgm.getFunction(loadPtr.base);
										   return func;
										 },
										 [this](const MiniMC::Model::SymbolicConstant& t) -> MiniMC::Model::Function_ptr {
										   auto symb = t.getValue();
										   auto func = prgm.getFunction(symb);
										   return func;
										 },
										 [](const auto&) -> MiniMC::Model::Function_ptr {
										   throw MiniMC::Support::Exception("Shouldn't happen");
										 }},
									     *content.function
									     );

	  std::vector<T> params;
	  if (func->isVarArgs()) {
	    throw MiniMC::Support::Exception("Vararg functions are not supported");
	  }
	  
	    
	  auto inserter = std::back_inserter(params);
	  std::for_each(content.params.begin(), content.params.end(), [&inserter, &state](auto& v) { inserter = state.getValueLookup().lookupValue(*v); });
	  
	  
	  auto res = content.res;
	  scontrol.push(func->getCFA().getInitialLocation(),  res);
	  
	  auto it = params.begin();
	  for (auto& p : func->getParameters()) {
	    state.getValueLookup().saveValue(*p, std::move(*it));
	    ++it;
	  }

	
	  return Status::Ok;
	}

        else if constexpr (op == MiniMC::Model::InstructionCode::Ret) {
	  auto& content = instr.getOps();
	  auto ret = state.getValueLookup().lookupValue(*content.value);
	  auto ret_reg = state.getStackControl().pop();
	  if (ret_reg)
	    state.getValueLookup().saveValue (ret_reg->asRegister (),std::move(ret));
	  	  
	  
	  return Status::Ok;
	  
	}

        else if constexpr (op == MiniMC::Model::InstructionCode::RetVoid) {
	  state.getStackControl().pop();
	  return Status::Ok;
	  
        }

        else if constexpr (op == MiniMC::Model::InstructionCode::Skip) {
          return Status::Ok;
        }

        else if constexpr (op == MiniMC::Model::InstructionCode::NonDet) {
	  auto& content = instr.getOps();
          auto& res = content.res->asRegister ();
	  MiniMC::Model::Undef val;
	  val.setType (res.getType());
          auto ret = state.getValueLookup().lookupValue(val);
          state.getValueLookup().saveValue(res, std::move(ret));
          return Status::Ok;
        }

        else {
	  throw NotImplemented<op>();
        }
      }

      template <class I,VMState<T> State>
      Status runInstruction(const I& instr, State& state)
        requires MiniMC::Model::isAggregate_v<I> 
      {
	constexpr auto op = I::getOpcode ();
        auto& content = instr.getOps ();
        auto& res = content.res->asRegister ();
        assert(content.offset->isConstant());
        MiniMC::BV64 offset{0};

        offset = MiniMC::Model::visitValue<MiniMC::BV64>(MiniMC::Support::Overload{
	      [](const MiniMC::Model::I16Integer& value) -> MiniMC::BV64 { return value.getValue(); },
	      [](const MiniMC::Model::I32Integer& value) -> MiniMC::BV64 { return value.getValue(); },
	      [](const MiniMC::Model::I64Integer& value) -> MiniMC::BV64 { return value.getValue(); },
	      [](const auto&) -> MiniMC::BV64 { throw MiniMC::Support::Exception("Invalid aggregate offset"); }},
	  *content.offset);
	
        if constexpr (op == MiniMC::Model::InstructionCode::InsertValue) {
          auto val_v = content.insertee;

          auto aggr = state.getValueLookup().lookupValue(*content.aggregate);
          auto value = state.getValueLookup().lookupValue(*content.insertee);

	  T::visit (MiniMC::Support::Overload {
	      [this,&state,&res,&offset](const typename T::Aggregate& aggr,const typename T::Aggregate& value) {
		state.getValueLookup().saveValue(res, operations.template InsertAggregateValue(aggr, offset, value));
	      },
	      [this,&state,&res,&offset](const typename T::Aggregate& aggr,const auto& value) {
		state.getValueLookup().saveValue(res, operations.template InsertBaseValue(aggr, offset, value));
	      },
	      [](const auto&,const auto&){
		throw MiniMC::Support::Exception ("Incompatible insert aggregate");
	      }
		}
 	    ,
	    aggr,
	    value);
	    
        }

        else if constexpr (op == MiniMC::Model::InstructionCode::ExtractValue) {
          auto aggr = T::visit (MiniMC::Support::Overload {
	      [](const typename T::Aggregate& aggr) {return aggr;},
	      [](const auto& )->T::Aggregate {throw MiniMC::Support::Exception ("Not an aggregate");},
	      	

	    },
	    state.getValueLookup().lookupValue(*content.aggregate)
	    );
	  
          switch (res.getType()->getTypeID()) {
            case MiniMC::Model::TypeID::I8:
              state.getValueLookup().saveValue(res, operations.template ExtractBaseValue<typename T::I8>(aggr, offset));
              break;
            case MiniMC::Model::TypeID::I16:
              state.getValueLookup().saveValue(res, operations.template ExtractBaseValue<typename T::I16>(aggr, offset));
              break;
            case MiniMC::Model::TypeID::I32:
              state.getValueLookup().saveValue(res, operations.template ExtractBaseValue<typename T::I32>(aggr, offset));
              break;
            case MiniMC::Model::TypeID::I64:
              state.getValueLookup().saveValue(res, operations.template ExtractBaseValue<typename T::I64>(aggr, offset));
              break;

            case MiniMC::Model::TypeID::Pointer:
              state.getValueLookup().saveValue(res, operations.template ExtractBaseValue<typename T::Pointer>(aggr, offset));
              break;
            case MiniMC::Model::TypeID::Aggregate:
              state.getValueLookup().saveValue(res, operations.ExtractAggregateValue(aggr, offset, res.getType()->getSize()));
              break;
            default:
              throw MiniMC::Support::Exception("Invalid Extract");
          }
        }
	else
          throw NotImplemented<op>();
        return Status::Ok;
      }
      
      };

    template <class Value,Ops<Value> Operations>
    template<VMState<Value> State>
    Status Engine<Value,Operations>::execute(const MiniMC::Model::Instruction& instr,
				       State& wstate) {
      
      return instr.visit ([this,&wstate](auto& t) {return _impl->template runInstruction (t, wstate);});
      
    }

    template <class Value,Ops<Value> Operations>
    template<VMState<Value> State>
    Status Engine<Value,Operations>::execute(const MiniMC::Model::InstructionStream& instr,
					     State& wstate) {
      auto end = instr.end();
      Status status = Status::Ok;
      auto it = instr.begin();
      for (it = instr.begin(); it != end && status == Status::Ok; ++it) {
	status = execute (*it,wstate);
      }
      return status;
    }
    
    template<class Value,Ops<Value> Operations>
    Engine<Value,Operations>::Engine (Operations&& ops,const MiniMC::Model::Program& prgm)   {
      _impl = std::make_unique<Impl> (std::move(ops),prgm);
    }
     
    template<class Value,Ops<Value> Operations>
    Engine<Value,Operations>::~Engine () {}
    
  } // namespace VMT
} // namespace MiniMC

#endif
