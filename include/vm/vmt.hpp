#ifndef _VM_VMT__
#define _VM_VMT__

#include "hash/hashing.hpp"
#include "vm/value.hpp"
#include "model/instructions.hpp"
#include "model/heaplayout.hpp"
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
    
    class Value;
    using Value_ptr = std::shared_ptr<Value>;
    
    template<class T>
    struct ValueLookup {
    public:
      virtual ~ValueLookup ()  {}
      virtual T lookupValue (const MiniMC::Model::Value_ptr&) const = 0;
      virtual void saveValue (const MiniMC::Model::Register&, T&&)  = 0;
      virtual T unboundValue (const MiniMC::Model::Type_ptr&) const = 0;
      virtual T defaultValue(const MiniMC::Model::Type_ptr&) const  = 0;

      using Value = T;
      
    };
    
    template<class T>
    struct BaseValueLookup : ValueLookup<T> {
    public:
      BaseValueLookup (std::size_t i) : values(i) {}
      BaseValueLookup (const BaseValueLookup&) = default;
      virtual  ~BaseValueLookup () {}
      virtual T lookupValue (const MiniMC::Model::Value_ptr& v) const override = 0;
      void saveValue(const MiniMC::Model::Register& v, T&& value) override {
	values.set (v,std::move(value));
      }
      virtual T unboundValue(const MiniMC::Model::Type_ptr&) const override = 0;
      virtual T defaultValue(const MiniMC::Model::Type_ptr&) const override = 0;
      
      MiniMC::Hash::hash_t hash() const { return values.hash(); }
      using Value = T;
    protected:
      T lookupRegister (const MiniMC::Model::Register& reg) const  {return values[reg];}
    private:
      MiniMC::Model::VariableMap<T> values;
    };
    

    
    
    template<class T>
    struct Memory {
    public:
      virtual ~Memory ()  {}
      virtual T loadValue (const typename T::Pointer&, const MiniMC::Model::Type_ptr& ) const  = 0;
      //First parameter is address to store at, second is the value to state
      virtual void storeValue (const typename T::Pointer&, const typename T::I8&) = 0;
      virtual void storeValue (const typename T::Pointer&, const typename T::I16&) = 0;
      virtual void storeValue (const typename T::Pointer&, const typename T::I32&) = 0;
      virtual void storeValue (const typename T::Pointer&, const typename T::I64&) = 0;
      virtual void storeValue (const typename T::Pointer&, const typename T::Pointer&) = 0;
      virtual void storeValue (const typename T::Pointer&, const typename T::Pointer32&) = 0;
      
      virtual void free (const typename T::Pointer&) = 0;
      
      
      //PArameter is size to allocate
      virtual T alloca (const typename T::I64& ) = 0;
      
      
      virtual void createHeapLayout (const MiniMC::Model::HeapLayout& layout) = 0;
      using Value = T;
    };

    enum class TriBool {
      True,
      False,
      Unk
    };
    
    
    template<class  T>
    struct PathControl {
      virtual ~PathControl ()  {}
      virtual TriBool addAssumption (const typename T::Bool&) = 0;
      virtual TriBool addAssert (const typename T::Bool&) = 0;
      using Value = T;
    };

    
    template<class  T>
    struct StackControl {
      virtual ~StackControl ()  {}
      virtual void  push (std::size_t,  const MiniMC::Model::Value_ptr& ) = 0;
      virtual void pop (T&&) = 0;
      virtual void popNoReturn () = 0;
      virtual ValueLookup<T>& getValueLookup () = 0;
      using Value = T;
    };
    
    
    template<class T, bool cons = false>  
    struct VMState {
      using VLookup  = typename std::conditional<!cons,ValueLookup<T>,const ValueLookup<T>>::type;
      using MLookup  = typename std::conditional<!cons,Memory<T>,const Memory<T>>::type;
      using PControl  = typename std::conditional<!cons,PathControl<T>,const PathControl<T>>::type;
      using StControl  = typename std::conditional<!cons,StackControl<T>,const StackControl<T>>::type;
      
      
      VMState (MLookup& m, PControl& path, StControl& stack) : memory(m),control(path),scontrol(stack) {}
      //auto& getValueLookup () {return lookup;}
      auto& getMemory () {return memory;}
      //auto& getValueLookup () const {return lookup;}
      auto& getMemory () const {return memory;}
      auto& getPathControl () const {return control;}
      auto& getStackControl () const {return scontrol;}
    private:
      MLookup& memory;
      PControl& control;
      StControl& scontrol;
    };

    enum class  Status{
      Ok,
      AssumeViolated,
      AssertViolated,
      UnsupportedOperation
    };


    template<class Int, class Bool,class Operation>
    concept IntOperationCompatible = requires (Operation op, const Int&left) {
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

    template<class Int, class Pointer,class Bool,class Operation>
    concept PointerOperationCompatible = requires (Operation op, const Int&left, const Pointer& ptr) {
      {op.PtrAdd (ptr,left)} -> std::convertible_to<Pointer>;
      {op.PtrEq (ptr,ptr)} -> std::convertible_to<Bool>;
      
    };
    
    template<class Int, class Aggregate,class Operation>
    concept AggregateCompatible = requires (Operation op, const Aggregate& aggr, MiniMC::BV64 index, const Int& insertee,size_t s) {
      {op.template ExtractBaseValue<Int> (aggr,index)} -> std::convertible_to<Int>;
      {op.ExtractAggregateValue (aggr,index,s)} -> std::convertible_to<Aggregate>;
      {op.template InsertBaseValue<Int> (aggr,index, insertee)} -> std::convertible_to<Aggregate>;
      {op.InsertAggregateValue (aggr,index,aggr)} -> std::convertible_to<Aggregate>;
    };

    template<class I8, class I16,class I32,class I64, typename Bool, typename Pointer,class Pointer32,class Caster>
    concept CastCompatible = requires (Caster op, const I8& i8,const I16& i16, const I32& i32, const I64& i64, const Bool& b,  const Pointer& p, const Pointer32& p32) {
      {op.template ZExt<1> (i8)} -> std::convertible_to<I8>;
      {op.template ZExt<2> (i8)} -> std::convertible_to<I16>;
      {op.template ZExt<4> (i8)} -> std::convertible_to<I32>;
      {op.template ZExt<8> (i8)} -> std::convertible_to<I64>;
      {op.template ZExt<2> (i16)} -> std::convertible_to<I16>;
      {op.template ZExt<4> (i16)} -> std::convertible_to<I32>;
      {op.template ZExt<8> (i16)} -> std::convertible_to<I64>;
      {op.template ZExt<4> (i32)} -> std::convertible_to<I32>;
      {op.template ZExt<8> (i32)} -> std::convertible_to<I64>;
      {op.template ZExt<8> (i64)} -> std::convertible_to<I64>;

      {op.template SExt<1> (i8)} -> std::convertible_to<I8>;
      {op.template SExt<2> (i8)} -> std::convertible_to<I16>;
      {op.template SExt<4> (i8)} -> std::convertible_to<I32>;
      {op.template SExt<8> (i8)} -> std::convertible_to<I64>;
      {op.template SExt<2> (i16)} -> std::convertible_to<I16>;
      {op.template SExt<4> (i16)} -> std::convertible_to<I32>;
      {op.template SExt<8> (i16)} -> std::convertible_to<I64>;
      {op.template SExt<4> (i32)} -> std::convertible_to<I32>;
      {op.template SExt<8> (i32)} -> std::convertible_to<I64>;
      {op.template SExt<8> (i64)} -> std::convertible_to<I64>;
      
      {op.template Trunc<8> (i64)} -> std::convertible_to<I64>;
      {op.template Trunc<4> (i64)} -> std::convertible_to<I32>;
      {op.template Trunc<2> (i64)} -> std::convertible_to<I16>;
      {op.template Trunc<1> (i64)} -> std::convertible_to<I8>;
      {op.template Trunc<4> (i32)} -> std::convertible_to<I32>;
      {op.template Trunc<2> (i32)} -> std::convertible_to<I16>;
      {op.template Trunc<1> (i32)} -> std::convertible_to<I8>;
      {op.template Trunc<2> (i16)} -> std::convertible_to<I16>;
      {op.template Trunc<1> (i16)} -> std::convertible_to<I8>;
      {op.template Trunc<1> (i8)} -> std::convertible_to<I8>;


      {op.template BoolZExt<1> (b)} -> std::convertible_to<I8>;
      {op.template BoolZExt<2> (b)} -> std::convertible_to<I16>;
      {op.template BoolZExt<4> (b)} -> std::convertible_to<I32>;
      {op.template BoolZExt<8> (b)} -> std::convertible_to<I64>;

      {op.template BoolSExt<1> (b)} -> std::convertible_to<I8>;
      {op.template BoolSExt<2> (b)} -> std::convertible_to<I16>;
      {op.template BoolSExt<4> (b)} -> std::convertible_to<I32>;
      {op.template BoolSExt<8> (b)} -> std::convertible_to<I64>;

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


    template<class T,class Operations,class Caster>
    concept VMCompatible = requires {
      requires IntOperationCompatible<typename T::I8,typename T::Bool,Operations>;
      requires IntOperationCompatible<typename T::I16,typename T::Bool,Operations>;
      requires IntOperationCompatible<typename T::I32,typename T::Bool,Operations>;
      requires IntOperationCompatible<typename T::I64,typename T::Bool,Operations>;
      requires AggregateCompatible<typename T::I8,typename T::Aggregate,Operations>; 
      requires AggregateCompatible<typename T::I16,typename T::Aggregate,Operations>;
      requires AggregateCompatible<typename T::I32,typename T::Aggregate,Operations>;
      requires AggregateCompatible<typename T::I64,typename T::Aggregate,Operations>;
      requires PointerOperationCompatible<typename T::I8,typename T::Pointer,typename T::Bool,Operations>;
      requires PointerOperationCompatible<typename T::I16,typename T::Pointer,typename T::Bool,Operations>;
      requires PointerOperationCompatible<typename T::I32,typename T::Pointer,typename T::Bool,Operations>;
      requires PointerOperationCompatible<typename T::I64,typename T::Pointer,typename T::Bool,Operations>;
      requires CastCompatible<typename T::I8,typename T::I16, typename T::I32, typename T::I64, typename T::Bool,typename T::Pointer,typename T::Pointer32, Caster>;
    };

    template<class T,class Operations,class Caster>
    requires VMCompatible<T,Operations,Caster>
    class Engine {
    public:
      using State = VMState<T,false>;
      using ConstState = VMState<T,true>;
      
      Engine (Operations&& ops, Caster&& caster,const MiniMC::Model::Program& prgm)  : operations(std::move(ops)), caster(std::move(caster)),prgm(prgm){}
      ~Engine ()  {}
      Status execute (const MiniMC::Model::InstructionStream&, State& ) ;
      using OperationsT = Operations;
      using CasterT = Caster;
    private:
      Operations operations;
      Caster caster;
      const MiniMC::Model::Program& prgm;
    };
    
    
    
  }

		
}


#include "vm/engine_implementation.hpp"

#endif
