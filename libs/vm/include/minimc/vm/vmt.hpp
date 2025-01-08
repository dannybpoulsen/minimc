#ifndef _VM_VMT__
#define _VM_VMT__

#include "minimc/hash/hashing.hpp"
#include "minimc/model/types.hpp"
#include "minimc/model/variables.hpp"
#include "minimc/vm/value.hpp"
#include "minimc/model/cfg.hpp"
#include "minimc/model/instructions.hpp"
#include "minimc/model/heaplayout.hpp"
#include "minimc/model/valuevisitor.hpp"
#include "minimc/support/overload.hpp"

#include <type_traits>
#include <iostream>
#include <utility>

namespace MiniMC {
  namespace Model {
    class Program;
  }
  namespace VMT {
    
    template<MiniMC::Model::VMInstructionCode c>
    class NotImplemented : public MiniMC::Support::Exception {
    public:
      NotImplemented () : MiniMC::Support::Exception (MiniMC::Support::Localiser{"Instruction '%1%' not supported."}.format (c)) {}
    };
    
    
    template<class Eval,class T>
    concept RegisterStore = requires (MiniMC::Model::Symbol s, const MiniMC::Model::Register& reg, const Eval& ceval, Eval& eval,  T&& t, const T::Pointer p,const T::Memory mem,const MiniMC::Model::Type& ty) {
      {ceval.lookupRegister (reg)} -> std::convertible_to<T>;
      {ceval.lookupSymbol (s)} -> std::convertible_to<T>;
      
      {eval.saveValue (reg,std::move(t))};
      {ceval.load(p,mem,ty)}->std::convertible_to<T>;
    } ;

    
    
    template<class Creato,class Res>
    concept Creator = requires (const Creato e,
				const MiniMC::Model::I8Integer& i8,
				const MiniMC::Model::I16Integer& i16,
				const MiniMC::Model::I32Integer& i32,
				const MiniMC::Model::I64Integer& i64,
				const MiniMC::Model::Bool& b,
				const MiniMC::Model::Pointer& ptr,
				const MiniMC::Model::Pointer32& ptr32,
				const MiniMC::Model::AggregateConstant& aggrc,
				const MiniMC::Model::Undef& und,
 				const MiniMC::Model::Type& ty) {
      {e.create(i8)}->std::convertible_to<Res>;
      {e.create(i16)}->std::convertible_to<Res>;
      {e.create(i32)}->std::convertible_to<Res>;
      {e.create(i64)}->std::convertible_to<Res>;
      {e.create(b)}->std::convertible_to<Res>;
      {e.create(ptr)}->std::convertible_to<Res>;
      {e.create(ptr32)}->std::convertible_to<Res>;
      {e.create(aggrc)}->std::convertible_to<Res>;
      {e.create(und)}->std::convertible_to<Res>;
      {e.defaultValue (ty)}->std::convertible_to<Res>;
    };
      
    enum class TriBool {
      True,
      False,
      Unk
    };
    
    
    template<class Mem,class Memory,class T>
    concept MemoryController = requires (Mem& memc,
					 const Memory& mem, 
					 const typename T::Pointer& p,
					 const typename T::I8& i8,
					 const typename T::I16& i16,
					 const typename T::I32& i32,
					 const typename T::I64& i64,
					 const typename T::Aggregate& aggr,
					 const typename T::Pointer& ptr,
					 const typename T::Pointer32& ptr32,
					 const MiniMC::Model::Type&ty
					 ) {
      {memc.store (mem,p,i8)}->std::convertible_to<Memory>;
      {memc.store (mem,p,i16)}->std::convertible_to<Memory>;
      {memc.store (mem,p,i32)}->std::convertible_to<Memory>;
      {memc.store (mem,p,i64)}->std::convertible_to<Memory>;
      {memc.store (mem,p,aggr)}->std::convertible_to<Memory>;
      {memc.store (mem,p,ptr)}->std::convertible_to<Memory>;
      {memc.store (mem,p,ptr32)}->std::convertible_to<Memory>;
      {memc.find_space(mem,i64)}->std::convertible_to<typename T::Pointer>;
      {memc.allocate(mem,ptr,i64)}->std::convertible_to<Memory>;
      {memc.free (mem,p)}->std::convertible_to<Memory>;
      {memc.load (mem,p,ty)}->std::convertible_to<T>;
    };

    template<class Mem,class T>
    concept Memory = requires (Mem& mem,
			       const typename T::Pointer& p,
			       const typename T::I8& i8,
			       const typename T::I16& i16,
			       const typename T::I32& i32,
			       const typename T::I64& i64,
			       const typename T::Aggregate& aggr,
			       const typename T::Pointer& ptr,
			       const typename T::Pointer32& ptr32,
			       const MiniMC::Model::Type&ty
				) {
      {mem.store (p,i8)};
      {mem.store (p,i16)};
      {mem.store (p,i32)};
      {mem.store (p,i64)};
      {mem.store (p,aggr)};
      {mem.store (p,ptr)};
      {mem.store (p,ptr32)};
      {mem.find_space(i64)}->std::convertible_to<typename T::Pointer>;
      {mem.allocate(ptr,i64)};
      {mem.free (p)};
      {mem.load (p,ty)}->std::convertible_to<T>;
    };

    
    template<class PathC,class T>
    concept PathControl = requires (const typename T::Bool& b,
                                    PathC& p)
    {
      {p.addAssumption (b)}->std::convertible_to<TriBool>;
      {p.addAssert (b)}->std::convertible_to<TriBool>;
    };

    template<class StackC>
    concept StackControl = requires (
				     StackC& p,
				     MiniMC::Model::Location_ptr loc,				     
				     MiniMC::Model::Register_ptr value
				     )
    {
      {p.push (loc,value)};
      {p.pop ()}->std::convertible_to<MiniMC::Model::Value_ptr>;
    };

    
    enum class  Status{
      Ok,
      AssumeViolated,
      AssertViolated,
      UnsupportedOperation
    };


    template<class Int, class Bool,class Operation>
    concept IntOperationCompatible_ = requires (Operation op, const Int&left) {
      {op.template Not<Int> (left)} -> std::convertible_to<Int>;
      {op.template Add<Int> (left,left)} -> std::convertible_to<Int>;
      {op.template Sub<Int> (left,left) } -> std::convertible_to<Int>;
      {op.template Mul<Int> (left,left) } -> std::convertible_to<Int>;
      {op.template UDiv<Int> (left,left) } -> std::convertible_to<Int>;
      {op.template SDiv<Int> (left,left) } -> std::convertible_to<Int>;
      {op.template LShl<Int> (left,left) } -> std::convertible_to<Int>;
      {op.template LShr<Int> (left,left) } -> std::convertible_to<Int>;
      {op.template AShr<Int> (left,left)  } -> std::convertible_to<Int>;
      {op.template And<Int> (left,left) } -> std::convertible_to<Int>;
      {op.template Or<Int> (left,left)  } -> std::convertible_to<Int>;
      {op.template Xor<Int> (left,left) } -> std::convertible_to<Int>;
      {op.template SGt<Int> (left,left)} -> std::convertible_to<Bool>;
      {op.template SGe<Int> (left,left) } -> std::convertible_to<Bool>;
      {op.template SLt<Int> (left,left) } -> std::convertible_to<Bool>;
      {op.template SLe<Int> (left,left) } -> std::convertible_to<Bool>;
      {op.template UGt<Int> (left,left) } -> std::convertible_to<Bool>;
      {op.template UGe<Int> (left,left) } -> std::convertible_to<Bool>;
      {op.template ULt<Int> (left,left) } -> std::convertible_to<Bool>;
      {op.template ULe<Int> (left,left)  } -> std::convertible_to<Bool>;
      {op.template Eq<Int> (left,left) } -> std::convertible_to<Bool>;
      {op.template NEq<Int> (left,left)  } -> std::convertible_to<Bool>;
    };

    template<class Value,class Operation>
    concept IntOperationCompatible =  (IntOperationCompatible_<typename Value::I8,typename Value::Bool,Operation> &&
				       IntOperationCompatible_<typename Value::I16,typename Value::Bool,Operation> &&
				       IntOperationCompatible_<typename Value::I32,typename Value::Bool,Operation> &&
				       IntOperationCompatible_<typename Value::I64,typename Value::Bool,Operation>
				       );
    
    template<class Int, class Pointer,class Bool,class Operation>
    concept PointerOperationCompatible_ = requires (Operation op, const Int&left, const Pointer& ptr) {
      {op.PtrAdd (ptr,left)} -> std::convertible_to<Pointer>;
      {op.PtrSub (ptr,left)} -> std::convertible_to<Pointer>;
      {op.PtrEq (ptr,ptr)} -> std::convertible_to<Bool>; 
    };

    template<class Value, class Operation>
    concept PointerOperationCompatible = (PointerOperationCompatible_<typename Value::I8,typename Value::Pointer,typename Value::Bool,Operation>&&
					   PointerOperationCompatible_<typename Value::I16,typename Value::Pointer,typename Value::Bool,Operation>&&
					   PointerOperationCompatible_<typename Value::I32,typename Value::Pointer,typename Value::Bool,Operation>&&
					  PointerOperationCompatible_<typename Value::I64,typename Value::Pointer,typename Value::Bool,Operation>
					   );
    
    template<class Int, class Aggregate,class Operation>
    concept AggregateCompatible_ = requires (Operation op, const Aggregate& aggr, MiniMC::BV64 index, const Int& insertee,size_t s) {
      {op.template ExtractBaseValue<Int> (aggr,index)} -> std::convertible_to<Int>;
      {op.ExtractAggregateValue (aggr,index,s)} -> std::convertible_to<Aggregate>;
      {op.template InsertBaseValue<Int> (aggr,index, insertee)} -> std::convertible_to<Aggregate>;
      {op.InsertAggregateValue (aggr,index,aggr)} -> std::convertible_to<Aggregate>;
    };

    template<class Value,class Operation>
    concept AggregateCompatible = (AggregateCompatible_<typename Value::I8,typename Value::Aggregate,Operation> &&
				   AggregateCompatible_<typename Value::I16,typename Value::Aggregate,Operation> &&
				   AggregateCompatible_<typename Value::I32,typename Value::Aggregate,Operation> &&
				   AggregateCompatible_<typename Value::I64,typename Value::Aggregate,Operation>
				   ) ;

    template<class I8, class I16,class I32,class I64, typename Bool, typename Pointer,class Pointer32,class Caster>
    concept CastCompatible_ = requires (Caster op, const I8& i8,const I16& i16, const I32& i32, const I64& i64, const Bool& b,  const Pointer& p, const Pointer32& p32) {
      {op.template ZExt<MiniMC::Model::TypeID::I8> (i8)} -> std::convertible_to<I8>;
      {op.template ZExt<MiniMC::Model::TypeID::I16> (i8)} -> std::convertible_to<I16>;
      {op.template ZExt<MiniMC::Model::TypeID::I32> (i8)} -> std::convertible_to<I32>;
      {op.template ZExt<MiniMC::Model::TypeID::I64> (i8)} -> std::convertible_to<I64>;
      {op.template ZExt<MiniMC::Model::TypeID::I16> (i16)} -> std::convertible_to<I16>;
      {op.template ZExt<MiniMC::Model::TypeID::I32> (i16)} -> std::convertible_to<I32>;
      {op.template ZExt<MiniMC::Model::TypeID::I64> (i16)} -> std::convertible_to<I64>;
      {op.template ZExt<MiniMC::Model::TypeID::I32> (i32)} -> std::convertible_to<I32>;
      {op.template ZExt<MiniMC::Model::TypeID::I64> (i32)} -> std::convertible_to<I64>;
      {op.template ZExt<MiniMC::Model::TypeID::I64> (i64)} -> std::convertible_to<I64>;

      {op.template SExt<MiniMC::Model::TypeID::I8> (i8)} -> std::convertible_to<I8>;
      {op.template SExt<MiniMC::Model::TypeID::I16> (i8)} -> std::convertible_to<I16>;
      {op.template SExt<MiniMC::Model::TypeID::I32> (i8)} -> std::convertible_to<I32>;
      {op.template SExt<MiniMC::Model::TypeID::I64> (i8)} -> std::convertible_to<I64>;
      {op.template SExt<MiniMC::Model::TypeID::I16> (i16)} -> std::convertible_to<I16>;
      {op.template SExt<MiniMC::Model::TypeID::I32> (i16)} -> std::convertible_to<I32>;
      {op.template SExt<MiniMC::Model::TypeID::I64> (i16)} -> std::convertible_to<I64>;
      {op.template SExt<MiniMC::Model::TypeID::I32> (i32)} -> std::convertible_to<I32>;
      {op.template SExt<MiniMC::Model::TypeID::I64> (i32)} -> std::convertible_to<I64>;
      {op.template SExt<MiniMC::Model::TypeID::I64> (i64)} -> std::convertible_to<I64>;
      
      {op.template Trunc<MiniMC::Model::TypeID::I64> (i64)} -> std::convertible_to<I64>;
      {op.template Trunc<MiniMC::Model::TypeID::I32> (i64)} -> std::convertible_to<I32>;
      {op.template Trunc<MiniMC::Model::TypeID::I16> (i64)} -> std::convertible_to<I16>;
      {op.template Trunc<MiniMC::Model::TypeID::I8> (i64)} -> std::convertible_to<I8>;
      {op.template Trunc<MiniMC::Model::TypeID::I32> (i32)} -> std::convertible_to<I32>;
      {op.template Trunc<MiniMC::Model::TypeID::I16> (i32)} -> std::convertible_to<I16>;
      {op.template Trunc<MiniMC::Model::TypeID::I8> (i32)} -> std::convertible_to<I8>;
      {op.template Trunc<MiniMC::Model::TypeID::I16> (i16)} -> std::convertible_to<I16>;
      {op.template Trunc<MiniMC::Model::TypeID::I8> (i16)} -> std::convertible_to<I8>;
      {op.template Trunc<MiniMC::Model::TypeID::I8> (i8)} -> std::convertible_to<I8>;


      {op.template BoolZExt<MiniMC::Model::TypeID::I8> (b)} -> std::convertible_to<I8>;
      {op.template BoolZExt<MiniMC::Model::TypeID::I16> (b)} -> std::convertible_to<I16>;
      {op.template BoolZExt<MiniMC::Model::TypeID::I32> (b)} -> std::convertible_to<I32>;
      {op.template BoolZExt<MiniMC::Model::TypeID::I64> (b)} -> std::convertible_to<I64>;

      {op.template BoolSExt<MiniMC::Model::TypeID::I8> (b)} -> std::convertible_to<I8>;
      {op.template BoolSExt<MiniMC::Model::TypeID::I16> (b)} -> std::convertible_to<I16>;
      {op.template BoolSExt<MiniMC::Model::TypeID::I32> (b)} -> std::convertible_to<I32>;
      {op.template BoolSExt<MiniMC::Model::TypeID::I64> (b)} -> std::convertible_to<I64>;
      {op.template BoolNegate (b)} -> std::convertible_to<Bool>;
      {op.template IntToBool<I8> (i8)} -> std::convertible_to<Bool>;
      {op.template IntToBool<I16> (i16)} -> std::convertible_to<Bool>;
      {op.template IntToBool<I32> (i32)} -> std::convertible_to<Bool>;
      {op.template IntToBool<I64> (i64)} -> std::convertible_to<Bool>;

      {op.template IntToPtr<I8> (i8)} -> std::convertible_to<Pointer>;
      {op.template IntToPtr<I16> (i16)} -> std::convertible_to<Pointer>;
      {op.template IntToPtr<I32> (i32)} -> std::convertible_to<Pointer>;
      {op.template IntToPtr<I64> (i64)} -> std::convertible_to<Pointer>;
      {op.template IntToPtr32<I8> (i8)} -> std::convertible_to<Pointer32>;
      {op.template IntToPtr32<I16> (i16)} -> std::convertible_to<Pointer32>;
      {op.template IntToPtr32<I32> (i32)} -> std::convertible_to<Pointer32>;
      {op.template IntToPtr32<I64> (i64)} -> std::convertible_to<Pointer32>;

      {op.template PtrToInt<I8> (p)} -> std::convertible_to<I8>;
      {op.template PtrToInt<I16> (p)} -> std::convertible_to<I16>;
      {op.template PtrToInt<I32> (p)} -> std::convertible_to<I32>;
      {op.template PtrToInt<I64> (p)} -> std::convertible_to<I64>;
      {op.template Ptr32ToInt<I8> (p32)} -> std::convertible_to<I8>;
      {op.template Ptr32ToInt<I16> (p32)} -> std::convertible_to<I16>;
      {op.template Ptr32ToInt<I32> (p32)} -> std::convertible_to<I32>;
      {op.template Ptr32ToInt<I64> (p32)} -> std::convertible_to<I64>;
      
      
      {op.template PtrToPtr32 (p)} -> std::convertible_to<Pointer32>;
      {op.template Ptr32ToPtr (p32)} -> std::convertible_to<Pointer>;
      
    };

    template<class T,class Operation>
    concept CastCompatible = CastCompatible_<typename T::I8,typename T::I16, typename T::I32, typename T::I64,
					     typename T::Bool,typename T::Pointer,typename T::Pointer32,Operation>;
    

    
    template<class Operation,class Value>
    concept Ops = CastCompatible<Value,Operation> &&
                  IntOperationCompatible<Value,Operation> &&
                  PointerOperationCompatible<Value,Operation> &&
                  AggregateCompatible<Value,Operation> &&
                  Creator<Operation,Value>
      ;


    template<class State>
    concept StackControllable = requires (State& state) {
      {state.getStackControl ()} ->StackControl;
    };

    


    template<class State,typename T>
    concept HasMemory = requires (State& state) {
      {state.getMemory ()} ->Memory<T>;
      
    };

    template<class State,typename Memory>
    concept HasMemoryNew = requires (State& state,Memory&& m) {
      {state.getMemory ()} ->std::convertible_to<Memory&>;
      {state.setMemory (m)};
      
    };

    template<class State,typename T>
    concept HasPathcontrol = requires (State& state) {
      {state.getPathControl ()} ->PathControl<T>;
    };
    
    template<class State,typename T>
    concept ValueLookupable = requires (State& state) {
      {state.getValueLookup ()} ->RegisterStore<T>;
    };

    
    template<class State,class V>
    concept VMState =  StackControllable<State> &&
                       ValueLookupable<State,V>;
    
    template<class T,class R>
    concept Integer = std::is_same_v<R,typename T::I8> || std::is_same_v<R,typename T::I16> || std::is_same_v<R,typename T::I32> || std::is_same_v<R,typename T::I64>;

    template<class T,class R>
    concept Boolean = std::is_same_v<R,typename T::Bool>;    

    template<class T,class R>
    concept MemoryC = std::is_same_v<R,typename T::Memory>;    
    
    
    template<class T,class R>
    concept Pointer = std::is_same_v<R,typename T::Pointer> || std::is_same_v<R,typename T::Pointer32>;

    
    template<class Value,RegisterStore<Value> RegStore,Ops<Value> Operations>
    class Evaluator {
    public:
      Evaluator (Operations ops, const RegStore regstore) : ops(ops),regstore(std::move(regstore)) {}
      
      Value Eval (const MiniMC::Model::Value& v)  const {
	return MiniMC::Model::visitValue<Value>(*this,v);
      }

      template <class Castee>
      auto castPtrToAppropriateInteger(Castee&& v) const   {
        if constexpr (std::is_same_v<Castee, typename Value::Pointer>) {
          return ops.template PtrToInt<typename Value::I64>(v);
        } else if constexpr (std::is_same_v<Castee, typename Value::Pointer32>) {
          return ops.template Ptr32ToInt<typename Value::I32>(v);
        } else {
          return v;
        }
      }

      template<class T>
      Value operator() (const T&) const  {
	throw MiniMC::Support::Exception ("Not implemented");
      }
      
      
      template<class T>
      Value operator() (const T& t) const requires (MiniMC::Model::is_root<T>) {
	return  ops.create(t);
      }

      
      
      template<typename T, MiniMC::Model::TypeID To>
      Value ExecTrunc (T from) const {
	if constexpr (T::intbitsize () >= MiniMC::Model::BitWidth<To>) {
	  return ops.template Trunc<To> (from);
	}
	else {
	  throw MiniMC::Support::Exception ("Invalid Truncation");
	}
      }

      Value operator() (const MiniMC::Model::TruncExpr& trunc) const  {
	Value res = Value::visit (MiniMC::Support::Overload {
	    [&trunc,this]<typename T>(const T& b) ->Value requires Integer<Value,T> {
	      switch (trunc.getToType()->getTypeID ()) {
	      case MiniMC::Model::TypeID::I8:
	      return ExecTrunc<T,MiniMC::Model::TypeID::I8> (b);
	      case MiniMC::Model::TypeID::I16:
	      return ExecTrunc<T,MiniMC::Model::TypeID::I16> (b);
	      case MiniMC::Model::TypeID::I32:
	      return ExecTrunc<T,MiniMC::Model::TypeID::I32> (b);
	      case MiniMC::Model::TypeID::I64:
	      return ExecTrunc<T,MiniMC::Model::TypeID::I64> (b);
	      default:
	      std::unreachable();
	      }
	      
	    },
	      MiniMC::Support::Error<Value> {}
	  },
	  Eval(trunc.getFrom ())
	  );

	return res;
	
      }


      template<typename T, MiniMC::Model::TypeID To>
      Value ExecZExt (T from) const {
	if constexpr (T::intbitsize () <= MiniMC::Model::BitWidth<To>) {
	  return ops.template ZExt<To> (from);
	}
	else {
	  throw MiniMC::Support::Exception ("Invalid Truncation");
	}
      }

      Value operator() (const MiniMC::Model::ZExtExpr& zext) const  {
	Value res = Value::visit (MiniMC::Support::Overload {
	    [&zext,this]<typename T>(const T& b) ->Value requires Integer<Value,T> {
	      switch (zext.getToType()->getTypeID ()) {
	      case MiniMC::Model::TypeID::I8:
	      return ExecZExt<T,MiniMC::Model::TypeID::I8> (b);
	      case MiniMC::Model::TypeID::I16:
	      return ExecZExt<T,MiniMC::Model::TypeID::I16> (b);
	      case MiniMC::Model::TypeID::I32:
	      return ExecZExt<T,MiniMC::Model::TypeID::I32> (b);
	      case MiniMC::Model::TypeID::I64:
	      return ExecZExt<T,MiniMC::Model::TypeID::I64> (b);
	      default:
	      std::unreachable();
	      }
	      
	    },
	      
	    [&zext,this](const typename Value::Bool& b) ->Value  {
	      switch (zext.getToType()->getTypeID()) {
	      case MiniMC::Model::TypeID::I8:
		return ops.template BoolZExt<MiniMC::Model::TypeID::I8>(b);
	      case MiniMC::Model::TypeID::I16:
		return ops.template BoolZExt<MiniMC::Model::TypeID::I16>(b);
		break;
	      case MiniMC::Model::TypeID::I32:
		return ops.template BoolZExt<MiniMC::Model::TypeID::I32>(b);
		break;
	      case MiniMC::Model::TypeID::I64:
		return ops.template BoolZExt<MiniMC::Model::TypeID::I64>(b);
		break;
	      default:
		std::unreachable();
	      }
	    },
	    MiniMC::Support::Error<Value> {}
	  },
	  Eval(zext.getFrom ())
	  );

	return res;
	
      }

      template<typename T, MiniMC::Model::TypeID To>
      Value ExecSExt (T from) const {
	if constexpr (T::intbitsize () <= MiniMC::Model::BitWidth<To>) {
	  return ops.template SExt<To> (from);
	}
	else {
	  throw MiniMC::Support::Exception ("Invalid Truncation");
	}
      }
      
      Value operator() (const MiniMC::Model::SExtExpr& sext) const  {
	Value res = Value::visit (MiniMC::Support::Overload {
	    [&sext,this]<typename T>(const T& b) ->Value requires Integer<Value,T> {
	      switch (sext.getToType()->getTypeID ()) {
	      case MiniMC::Model::TypeID::I8:
	      return ExecSExt<T,MiniMC::Model::TypeID::I8> (b);
	      case MiniMC::Model::TypeID::I16:
	      return ExecSExt<T,MiniMC::Model::TypeID::I16> (b);
	      case MiniMC::Model::TypeID::I32:
	      return ExecSExt<T,MiniMC::Model::TypeID::I32> (b);
	      case MiniMC::Model::TypeID::I64:
	      return ExecSExt<T,MiniMC::Model::TypeID::I64> (b);
	      default:
	      std::unreachable();
	      }
	      
	    },
	      
	    [&sext,this](const typename Value::Bool& b) ->Value  {
	      switch (sext.getToType()->getTypeID()) {
	      case MiniMC::Model::TypeID::I8:
		return ops.template BoolSExt<MiniMC::Model::TypeID::I8>(b);
	      case MiniMC::Model::TypeID::I16:
		return ops.template BoolSExt<MiniMC::Model::TypeID::I16>(b);
		break;
	      case MiniMC::Model::TypeID::I32:
		return ops.template BoolSExt<MiniMC::Model::TypeID::I32>(b);
		break;
	      case MiniMC::Model::TypeID::I64:
		return ops.template BoolSExt<MiniMC::Model::TypeID::I64>(b);
		break;
	      default:
		std::unreachable();
	      }
	    },
	    MiniMC::Support::Error<Value> {}
	  },
	  Eval(sext.getFrom ())
	  );

	return res;
	
      }

      Value operator() (const MiniMC::Model::IntToBoolExpr& sext) const  {
	return Value::visit (  MiniMC::Support::Overload {
	    [this]<typename T> (const T v)->Value requires Integer<Value,T> {return ops.IntToBool (v);},
	    MiniMC::Support::Error<Value>{}
	  },
	  Eval (sext.getFrom ())
	  );
      }
      
      Value operator() (const MiniMC::Model::IntToPtrExpr& sext) const  {
	return Value::visit (  MiniMC::Support::Overload {
	    [this]<typename T> (const T v)->Value requires Integer<Value,T> {
	      return ops.IntToPtr (v);
	    },
	    MiniMC::Support::Error<Value>{}
	  },
	  Eval (sext.getFrom ())
	  );
      }

      Value operator() (const MiniMC::Model::PtrToIntExpr& sext) const  {
	return Value::visit (  MiniMC::Support::Overload {
	    [this,&sext](const typename Value::Pointer& val)->Value  {
	      switch (sext.getToType ()->getTypeID ()) {
	      case MiniMC::Model::TypeID::I8:
		return ops.template PtrToInt<typename Value::I8> (val);
	      case MiniMC::Model::TypeID::I16:
		return ops.template PtrToInt<typename Value::I16> (val);
	      case MiniMC::Model::TypeID::I32:
		return ops.template PtrToInt<typename Value::I32> (val);
	      case MiniMC::Model::TypeID::I64:
		return ops.template PtrToInt<typename Value::I64> (val);
	      default:
		std::unreachable();
	      }
	    },
	      [this,&sext](const typename Value::Pointer32& val)->Value  {
	      switch (sext.getToType ()->getTypeID ()) {
	      case MiniMC::Model::TypeID::I8:
		return ops.template Ptr32ToInt<typename Value::I8> (val);
	      case MiniMC::Model::TypeID::I16:
		return ops.template Ptr32ToInt<typename Value::I16> (val);
	      case MiniMC::Model::TypeID::I32:
		return ops.template Ptr32ToInt<typename Value::I32> (val);
	      case MiniMC::Model::TypeID::I64:
		return ops.template Ptr32ToInt<typename Value::I64> (val);
	      default:
		std::unreachable();
	      }
	      },
	  
	  MiniMC::Support::Error<Value>{}
	  },
	  Eval (sext.getFrom ())
	  );
      }
      
      Value operator() (const MiniMC::Model::Register& reg) const  {
	return regstore.lookupRegister (reg);
      }
      
      Value operator() (const MiniMC::Model::SymbolicConstant& s) const  {
	return std::visit (
		    MiniMC::Support::Overload {
		      [this](const MiniMC::Model::Register_wptr& r)->Value {return regstore.lookupRegister (*r.lock());},
		      [this,&s](const MiniMC::Model::HeapBlock_wptr&)->Value {return regstore.lookupSymbol (s.getValue());},
		      MiniMC::Support::Error<Value>{}	 
		    },
		    
		    s.getValue().getUserData()
			   );
      }
      
      
      
#define OPSI					\
      X(AddExpr,Add)				\
      X(SubExpr,Sub)				\
      X(MulExpr, Mul)				\
      X(UDivExpr, UDiv)				\
      X(SDivExpr, SDiv)				\
      X(ShlExpr, LShl)				\
      X(AShrExpr, AShr)				\
      X(LShrExpr, LShr)				\
      X(AndExpr, And)				\
      X(OrExpr, Or)				\
      X(XorExpr, Xor)				\
      
#define X(CC,op)							\
      Value operator() (const MiniMC::Model::CC& cc) const  {		\
	auto l = Eval (cc.op1());					\
	auto r = Eval (cc.op2());					\
      return Value::visit (MiniMC::Support::Overload {			\
      [this]<typename T> (T& ll, T& rr) -> Value requires Integer<Value,T>  { \
	return ops.op (ll,rr);},					\
      [](auto&, auto& ) -> Value {throw MiniMC::Support::Exception ("Error");} \
    },l,r);								\
}									
      OPSI
#undef X
#undef OPSI

#define OPSI					\
      X(SGtExpr, SGt)			\
      X(UGtExpr, UGt)			\
      X(SGEExpr, SGe)			\
      X(UGEExpr, UGe)			\
      X(SLtExpr, SLt)			\
      X(ULtExpr, ULt)			\
      X(SLEExpr, SLe)			\
      X(ULEExpr, ULe)			\
      X(EqExpr, Eq)			\
      X(NEqExpr, NEq)			\

#define X(CC,op)							\
      Value operator() (const MiniMC::Model::CC& cc) const  {		\
	auto l = Eval (cc.op1 ());					\
	auto r = Eval (cc.op2 ());					\
	return Value::visit (MiniMC::Support::Overload {		\
	    [this]<typename T> (T& ll, T& rr) -> Value requires Integer<Value,T>  { \
	      return ops.op (ll,rr);},					\
	      [this]<typename T> (T& ll, T& rr) -> Value requires Pointer<Value,T>  { \
		return ops.op (castPtrToAppropriateInteger<T>(std::forward<T>(ll)),castPtrToAppropriateInteger<T>(std::forward<T>(rr)));}, \
	      [](auto&, auto& ) -> Value {throw MiniMC::Support::Exception ("Error");} \
	      },l,r);							\
      }									
      OPSI
#undef X
#undef OPSI     
      
      
#define OPSI								\
      X(LogNotExpr, BoolNegate)						
      
#define X(CC,op)							\
      Value operator() (const MiniMC::Model::CC& cc) const  {		\
	auto l = Eval (cc.op1());					\
	return Value::visit (MiniMC::Support::Overload {		\
	  [this] (typename Value::Bool& ll) -> Value    {		\
	    return ops.op (ll);					\
	  },								\
	  [](auto&) -> Value {throw MiniMC::Support::Exception ("Error");} \
	    },l								\
	  );								\
      }									\
      
OPSI
#undef X
#undef OPSI   
      
      Value operator() (const MiniMC::Model::NotExpr& notex) const  {
	return Value::visit (  MiniMC::Support::Overload {
	    [this]<typename T> (const  T& t) ->Value requires Integer<Value,T>{
	      return ops.Not (t);
	    },
	    MiniMC::Support::Error<Value>{}
	  },
	  Eval (notex.op1 ())
	  );
      }

      
      Value operator() (const MiniMC::Model::LoadExpr& load) const  {
	return Value::visit (  MiniMC::Support::Overload {
	    [this,&load] (const typename Value::Memory& m,const  typename Value::Pointer& p) {
	      return regstore.load (p,m,*load.getToType());
	    },
	    [this,&load] (const typename Value::Memory& m,const  typename Value::Pointer32& p) {
	      return regstore.load (ops.Ptr32ToPtr (p),m,*load.getToType());
	    },
	      MiniMC::Support::Error<Value>{}
	  },
	  Eval (load.mem()),
	  Eval (load.addr ())
	  );
      }

      

      Value operator() (const MiniMC::Model::PtrAddExpr& load) const  {
	auto visitor = MiniMC::Support::Overload {
	  [this,&load]<typename ValT>(Value::Pointer& ptr,ValT& skipsize,ValT& nbskips)->Value requires Integer<Value,ValT> {
	    auto totalskip = ops.Mul(skipsize, nbskips);
	    return  ops.PtrAdd(ptr, totalskip);
	    
	  },
	  MiniMC::Support::Error<Value>{}
	};
	
	return Value::visit (visitor,Eval(load.ptr()),Eval(load.skipsize()),Eval(load.nbSkips()));
      }

      Value operator() (const MiniMC::Model::PtrSubExpr& load) const  {
	auto visitor = MiniMC::Support::Overload {
	  [this]<typename ValT>(Value::Pointer& ptr,ValT& skipsize,ValT& nbskips)->Value requires Integer<Value,ValT> {
	    auto totalskip = ops.Mul(skipsize, nbskips);
	    return ops.PtrSub(ptr, totalskip);
	    
	  },
	  MiniMC::Support::Error<Value>{}
	};
	
	return Value::visit (visitor,Eval(load.ptr()),Eval(load.skipsize()),Eval(load.nbSkips()));
      }
      
      template<class T>
      Value operator() (const T&) const requires (MiniMC::Model::is_bin_arith<T> || MiniMC::Model::is_bin_cmp<T>) {
	throw MiniMC::Support::Exception ("Not Implemented");
      }
      
      
      
      
    private:
      Operations ops;
      const RegStore regstore;
    };

    template<class Value,RegisterStore<Value> RegStore,Ops<Value> Operations>
    Evaluator<Value,RegStore,Operations> makeEvaluator (RegStore reg, Operations ops) {return Evaluator<Value,RegStore,Operations> (ops,std::move(reg));}
    
    
    template<class Value, class Memory, Ops<Value> Operations, MemoryController<Memory,Value> MemControl>
    class Engine {
    public:
      Engine (Operations&& ops,MemControl&& memcontrol, const MiniMC::Model::Program& prgm);
      ~Engine ();
      
      template<VMState<Value> VState>
      Status execute (const MiniMC::Model::InstructionStream&, VState& ) ;

      template<VMState<Value> VState>
      Status execute (const MiniMC::Model::Instruction&, VState& ) ;
      
    private:
      class Impl;
      std::unique_ptr<Impl> _impl;
    };
    
    
    
  }

		
}


#include "minimc/vm/engine_implementation.hpp"

#endif
