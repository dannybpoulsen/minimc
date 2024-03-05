#ifndef _VM_VMT__
#define _VM_VMT__

#include "minimc/hash/hashing.hpp"
#include "minimc/vm/value.hpp"
#include "minimc/model/cfg.hpp"
#include "minimc/model/instructions.hpp"
#include "minimc/model/heaplayout.hpp"

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
    concept Evaluator = requires (const MiniMC::Model::Value& v, const MiniMC::Model::Register& reg, const Eval& ceval, Eval& eval,  T&& t,MiniMC::Model::Type& ty) {
      {ceval.lookupValue (v)} -> std::convertible_to<T>;
      {ceval.unboundValue (ty)}->std::convertible_to<T>;
      {ceval.defaultValue (ty)}->std::convertible_to<T>;
      {eval.saveValue (reg,std::move(t))};
    } ;

    
    
    
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
				const MiniMC::Model::HeapLayout& heapl
				) {
      {mem.storeValue (p,i8)};
      {mem.storeValue (p,i16)};
      {mem.storeValue (p,i32)};
      {mem.storeValue (p,i64)};
      {mem.storeValue (p,aggr)};
      {mem.storeValue (p,ptr)};
      {mem.storeValue (p,ptr32)};
      {mem.alloca(i64)}->std::convertible_to<typename T::Pointer>;
      {mem.createHeapLayout (heapl)};
      {mem.free (p)};
    };
			       
    template<class PathC,class T>
    concept PathControl = requires (const typename T::Bool& b,
                                    PathC& p)
    {
      {p.addAssumption (b)}->std::convertible_to<TriBool>;
      {p.addAssert (b)}->std::convertible_to<TriBool>;
      
    };

    template<class StackC,class T>
    concept StackControl = requires (
				     StackC& p,
				     MiniMC::Model::Location_ptr loc,
				     std::size_t s,
				     MiniMC::Model::Value_ptr value,
				     T&& t)
    {
      {p.push (loc,s,value)};
      {p.pop (std::move(t))};
      {p.popNoReturn ()};
    };

    template<class StackC>
    concept SimpStackControl = requires (
				     StackC& p,
				     MiniMC::Model::Location_ptr loc,
				     std::size_t s,
				     MiniMC::Model::Value_ptr value
					 )
    {
      {p.push (loc,s,value)};
      {p.popNoReturn ()};
    };
    
    
    
    template<class T,Evaluator<T> Eval, Memory<T> Mem,PathControl<T> PathC,StackControl<T> stackC>  
    struct VMState {
      using Domain = T;
      
      
      VMState (Mem& m, PathC& path, stackC& stack,Eval& vlook) : memory(m),control(path),scontrol(stack),lookup(vlook) {}
      auto& getValueLookup () {return lookup;}
      auto& getMemory () {return memory;}
      auto& getPathControl ()  {return control;}
      auto& getStackControl ()  {return scontrol;}
    private:
      Mem& memory;
      PathC& control;
      stackC& scontrol;
      Eval& lookup;
    };

    
    template<class T,Evaluator<T> Eval,Memory<T> Mem,PathControl<T> PathC>  
    struct VMInitState {
      using Domain = T;
      
      
      VMInitState (Mem& m, PathC& path, Eval& vlook) : memory(m),control(path),lookup(vlook) {}
      auto& getValueLookup () {return lookup;}
      auto& getPathControl () const {return control;}
    private:
      Mem& memory;
      PathC& control;
      Eval& lookup;
    };

    template<class State>
    concept StackControllable = requires (State& state) {
      {state.getStackControl ()} ->StackControl<typename State::Domain>;
    };

    template<class State>
    concept SimpStackControllable = requires (State& state) {
      {state.getStackControl ()} ->SimpStackControl;
    };



    template<class State>
    concept MemoryControllable = requires (State& state) {
      {state.getMemory ()} ->Memory<typename State::Domain>;
    };

    template<class State>
    concept PathControllable = requires (State& state) {
      {state.getPath ()} ->PathControl<typename State::Domain>;
    };
    
    template<class State>
    concept ValueLookupable = requires (State& state) {
      {state.getValueLookup ()} ->Evaluator<typename State::Domain>;
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
    concept AggregateCompatible = requires (Operation op, const Aggregate& aggr, MiniMC::BV64 index, const Int& insertee,size_t s) {
      {op.template ExtractBaseValue<Int> (aggr,index)} -> std::convertible_to<Int>;
      {op.ExtractAggregateValue (aggr,index,s)} -> std::convertible_to<Aggregate>;
      {op.template InsertBaseValue<Int> (aggr,index, insertee)} -> std::convertible_to<Aggregate>;
      {op.InsertAggregateValue (aggr,index,aggr)} -> std::convertible_to<Aggregate>;
    };

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
    
    class DummyOperations {
    public:
      using Domain = int;
    };
    
    
    template<class Value, class Operations>
    class Engine {
    public:
      Engine (Operations&& ops,const MiniMC::Model::Program& prgm);
      ~Engine ();
      
      template<class VState>
      Status execute (const MiniMC::Model::InstructionStream&, VState& ) ;

      template<class VState>
      Status execute (const MiniMC::Model::Instruction&, VState& ) ;
      
      using OperationsT = Operations;
    private:
      class Impl;
      std::unique_ptr<Impl> _impl;
    };
    
    
    
  }

		
}


#include "minimc/vm/engine_implementation.hpp"

#endif
