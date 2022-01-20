#ifndef _VM_VMT__
#define _VM_VMT__

#include "hash/hashing.hpp"
#include "vm/value.hpp"
#include "model/instructions.hpp"
#include "model/heaplayout.hpp"

#include <type_traits>

namespace MiniMC {
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
      virtual T lookupValue (const MiniMC::Model::Value_ptr&) const {throw MiniMC::Support::Exception ("Not implemented");};
      virtual void saveValue (const MiniMC::Model::Variable_ptr&, T&&) {throw MiniMC::Support::Exception ("Not implemented");};
      virtual T unboundValue (const MiniMC::Model::Type_ptr&) const {throw MiniMC::Support::Exception ("Not implemented");};
    };

    
    
    template<class T>
    struct Memory {
    public:
      virtual ~Memory ()  {}
      virtual T loadValue (const T::Pointer&, const MiniMC::Model::Type_ptr& ) const  {throw MiniMC::Support::Exception ("Not implemented");};
      //First parameter is address to store at, second is the value to state
      virtual void storeValue (const typename T::Pointer&, const typename T::I8&) {throw MiniMC::Support::Exception ("Not implemented");};
      virtual void storeValue (const typename T::Pointer&, const typename T::I16&) {throw MiniMC::Support::Exception ("Not implemented");};
      virtual void storeValue (const typename T::Pointer&, const typename T::I32&) {throw MiniMC::Support::Exception ("Not implemented");};
      virtual void storeValue (const typename T::Pointer&, const typename T::I64&) {throw MiniMC::Support::Exception ("Not implemented");};
      virtual void storeValue (const typename T::Pointer&, const typename T::Pointer&) {throw MiniMC::Support::Exception ("Not implemented");};
      
      //PArameter is size to allocate
      virtual T alloca (const typename T::I64& ) {throw MiniMC::Support::Exception ("Not implemented");};
      
      virtual void free (const typename T::Pointer&) {throw MiniMC::Support::Exception ("Not implemented");}
      virtual void createHeapLayout (const MiniMC::Model::HeapLayout& layout) = 0;
    };

    enum class TriBool {
      True,
      False,
      Unk
    };
    
    
    template<class  T>
    struct PathControl {
      virtual ~PathControl ()  {}
      virtual TriBool addAssumption (const typename T::Bool&) {throw MiniMC::Support::Exception ("Not implemented");}
      virtual TriBool addAssert (const typename T::Bool&) {throw MiniMC::Support::Exception ("Not implemented");}
      
    };
    
    
    
    template<class T, bool cons = false>  
    struct VMState {
      using VLookup  = std::conditional<!cons,ValueLookup<T>,const ValueLookup<T>>::type;
      using MLookup  = std::conditional<!cons,Memory<T>,const Memory<T>>::type;
      using PControl  = std::conditional<!cons,PathControl<T>,const PathControl<T>>::type;
      
      VMState (VLookup& p, MLookup& m, PControl& path) : lookup(p),memory(m),control(path) {}
      auto& getValueLookup () {return lookup;}
      auto& getMemory () {return memory;}
      auto& getValueLookup () const {return lookup;}
      auto& getMemory () const {return memory;}
      auto& getPathControl () const {return control;}
    private:
      VLookup& lookup;
      MLookup& memory;
      PControl& control;
    };

    enum class  Status{
      Ok,
      AssumeViolated,
      AssertViolated,
      UnsupportedOperation
    };

    template<class T,class Caster>
    class Engine {
    public:
      using State = VMState<T,false>;
      using ConstState = VMState<T,true>;
      
      Engine () {}
      ~Engine ()  {}
      virtual Status execute (const MiniMC::Model::InstructionStream&, State&, ConstState&  ) ;
    private:
      Caster caster;
    };
    
    
    
  }

		
}


#include "vm/engine_implementation.hpp"

#endif
