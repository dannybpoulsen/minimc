#ifndef _CONCRETE_VM__
#define _CONCRETE_VM__

#include "minimc/hash/hashing.hpp"
#include "minimc/host/casts.hpp"
#include "minimc/host/operataions.hpp"

#include "cpa/common.hpp"
#include "minimc/vm/vmt.hpp"
#include "concvm/value.hpp"
#include "concvm/operations.hpp"

#include <memory>

namespace MiniMC {
  namespace VMT {
    namespace Concrete {
      
      class Memory  {
      public:
	Memory ();
	Memory (const Memory&);
	~Memory ();
        Value load(const typename Value::Pointer&, const MiniMC::Model::Type_ptr&) const;
        // First parameter is address to store at, second is the value to state
        void store(const Value::Pointer&, const Value::I8&) ;
	void store(const Value::Pointer&, const Value::I16&) ;
        void store(const Value::Pointer&, const Value::I32&) ;
        void store(const Value::Pointer&, const Value::I64&) ;
	void store(const Value::Pointer&, const Value::Aggregate&) ;
	void store(const Value::Pointer&, const Value::Pointer&) ;
	void store(const Value::Pointer&, const Value::Pointer32&) ;
	MiniMC::Hash::hash_t hash () const;
	
	// PArameter is size to allocate
	Value::Pointer alloca(const Value::I64&) ;
	
        void free(const Value::Pointer&);
        void createHeapLayout(const MiniMC::Model::HeapLayout& layout);
	
      private:
        struct internal;
        std::unique_ptr<internal> _internal;
      };

      struct ValueCreator {
      public:
	Value create (const MiniMC::Model::I8Integer& val)  const  { return Value::I8{val.getValue()}; }
	Value create (const MiniMC::Model::I16Integer& val) const { return Value::I16{val.getValue()}; }
	Value create (const MiniMC::Model::I32Integer& val) const  { return Value::I32{val.getValue()}; }
	Value create (const MiniMC::Model::I64Integer& val) const  { return Value::I64{val.getValue()}; }
	Value create (const MiniMC::Model::Bool& val) const   { return Value::Bool{val.getValue()}; }
	Value create (const MiniMC::Model::Pointer& val) const  { return Value::Pointer{val.getValue()}; }
	Value create (const MiniMC::Model::Pointer32& val) const   { return Value::Pointer32{val.getValue()}; }
	Value create (const MiniMC::Model::AggregateConstant& val) const   {return AggregateValue(val.getData());}
	Value create (const MiniMC::Model::Undef& und) const { return this->unboundValue (*und.getType ()); }
	Value create(const MiniMC::Model::SymbolicConstant& ) const    {throw MiniMC::Support::Exception ("Cannot Evaluate Symbolic Constants");}
	Value unboundValue (const MiniMC::Model::Type&) const ;
	Value defaultValue(const MiniMC::Model::Type&) const ;
	
      };

      
      
      
      class PathControl  {
      public:
        TriBool addAssumption(const Value::Bool& b) {
	  return b.getValue () ? TriBool::True : TriBool::False;
	}
	
        TriBool addAssert(const Value::Bool& b) {
	  return b.getValue () ? TriBool::True : TriBool::False;
	}

      };

      
      
      using ActivationRecord = MiniMC::CPA::Common::ActivationRecord<MiniMC::VMT::Concrete::Value>;
      using ActivationStack = MiniMC::CPA::Common::ActivationStack<MiniMC::VMT::Concrete::Value>;
      using ConcreteVMState = MiniMC::CPA::Common::VMState<MiniMC::VMT::Concrete::Value,MiniMC::CPA::Common::RegisterStore<Value>,Memory,PathControl,ActivationStack>;
      
      //ConcreteVMState 
      using ConcreteEngine = MiniMC::VMT::Engine<Value, MiniMC::VMT::Concrete::Operations >;
      
      
    } // namespace Concrete
    
  } // namespace VMT
} // namespace MiniMC

namespace std {
  template <>
  struct hash<MiniMC::VMT::Concrete::PointerValue> {
    auto operator()(const MiniMC::VMT::Concrete::PointerValue& t) { return t.hash(); }
  };

  template <typename T>
  struct hash<MiniMC::VMT::Concrete::TValue<T>> {
    auto operator()(const MiniMC::VMT::Concrete::TValue<T>& t) { return t.hash(); }
  };

  template <>
  struct hash<MiniMC::VMT::Concrete::BoolValue> {
    auto operator()(const MiniMC::VMT::Concrete::BoolValue& t) { return t.hash(); }
  };

  template <>
  struct hash<MiniMC::VMT::Concrete::AggregateValue> {
    auto operator()(const MiniMC::VMT::Concrete::AggregateValue& t) { return t.hash(); }
  };
  
  template <>
  struct hash<MiniMC::VMT::Concrete::Value> {
    auto operator()(const MiniMC::VMT::Concrete::Value& t) { return t.hash(); }
  };
  
  template <>
  struct hash<MiniMC::VMT::Concrete::Memory> {
    auto operator()(const MiniMC::VMT::Concrete::Memory& t) { return t.hash(); }
  };
  
} // namespace std

#endif
