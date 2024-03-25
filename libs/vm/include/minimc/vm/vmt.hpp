#ifndef _VM_VMT__
#define _VM_VMT__

#include "minimc/hash/hashing.hpp"
#include "minimc/vm/value.hpp"
#include "minimc/model/cfg.hpp"
#include "minimc/model/instructions.hpp"
#include "minimc/model/heaplayout.hpp"
#include "minimc/model/valuevisitor.hpp"
#include "minimc/support/overload.hpp"

#include <type_traits>

namespace MiniMC {
  namespace Model {
    class Program;
  }
  namespace VMT {
    
    template<MiniMC::Model::InstructionCode c>
    class NotImplemented : public MiniMC::Support::Exception {
    public:
      NotImplemented () : MiniMC::Support::Exception (MiniMC::Support::Localiser{"Instruction '%1%' not supported."}.format (c)) {}
    };
    

    template<class Eval,class T >
    concept RegisterStore = requires (const MiniMC::Model::Register& reg, const Eval& ceval, Eval& eval,  T&& t) {
      {ceval.lookupRegister (reg)} -> std::convertible_to<T>;
      {eval.saveValue (reg,std::move(t))};
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
				const MiniMC::Model::SymbolicConstant& sc,
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
      {e.create(sc)}->std::convertible_to<Res>;
      {e.defaultValue (ty)}->std::convertible_to<Res>;
    };
      
    enum class TriBool {
      True,
      False,
      Unk
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
			       const MiniMC::Model::HeapLayout& heapl,
			       MiniMC::Model::Type_ptr ty
				) {
      {mem.store (p,i8)};
      {mem.store (p,i16)};
      {mem.store (p,i32)};
      {mem.store (p,i64)};
      {mem.store (p,aggr)};
      {mem.store (p,ptr)};
      {mem.store (p,ptr32)};
      {mem.alloca(i64)}->std::convertible_to<typename T::Pointer>;
      {mem.createHeapLayout (heapl)};
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
      ValueLookupable<State,V> &&
      HasMemory<State,V>; 

    template<class T,class R>
    concept Integer = std::is_same_v<R,typename T::I8> || std::is_same_v<R,typename T::I16> || std::is_same_v<R,typename T::I32> || std::is_same_v<R,typename T::I64>;

    template<class T,class R>
    concept Boolean = std::is_same_v<R,typename T::Bool>;    
    
    template<class T,class R>
    concept Pointer = std::is_same_v<R,typename T::Pointer> || std::is_same_v<R,typename T::Pointer32>;

    
    template<class Value,RegisterStore<Value> RegStore,Ops<Value> Operations>
    class Evaluator {
    public:
      Evaluator (Operations ops, const RegStore& regstore) : ops(ops),regstore(regstore) {}
      
      Value Eval (const MiniMC::Model::Value& v)  const {
	return MiniMC::Model::visitValue<Value>(*this,v);
      }
      
      template<class T>
      Value operator() (const T& t) const requires (!std::is_same_v<T,MiniMC::Model::Register> && !std::is_same_v<T,MiniMC::Model::AddExpr>) {
	return ops.create(t);
      }
      
      Value operator() (const MiniMC::Model::AddExpr& add) const  {
	auto l = Eval (add.getLeft ());
	auto r = Eval (add.getRight ());
	return Value::visit (MiniMC::Support::Overload {
	    [this]<typename T> (T& ll, T& rr) -> Value requires Integer<Value,T>  {
	      return ops.Add (ll,rr);},
	    [](auto&, auto& ) -> Value {throw MiniMC::Support::Exception ("Error");}
	  },l,r);
      }
      
      Value operator() (const MiniMC::Model::Register& reg) const  {
	return regstore.lookupRegister (reg);
      }
      
      
    private:
      Operations ops;
      const RegStore& regstore;
    };
    
    template<class Value, Ops<Value> Operations>
    class Engine {
    public:
      Engine (Operations&& ops,const MiniMC::Model::Program& prgm);
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
