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
       
      class ValueLookupBase  {
      public:
	Value lookupValue (const MiniMC::Model::Value& v) const ;
	Value unboundValue (const MiniMC::Model::Type&) const ;
	Value defaultValue(const MiniMC::Model::Type&) const ;
	virtual void saveValue(const MiniMC::Model::Register&, Value&& )  {throw MiniMC::Support::Exception ("Can't save values");}
	virtual Value lookupRegisterValue (const MiniMC::Model::Register&) const {throw MiniMC::Support::Exception ("Can't lookupRegisters");}
      };
      
      class ValueLookup : public ValueLookupBase,
			  private MiniMC::CPA::Common::BaseValueLookup<Value> {
      public:
	ValueLookup (MiniMC::CPA::Common::ActivationStack<Value > & values,MiniMC::Model::VariableMap<Value>& metas) : BaseValueLookup<Value>(values,metas) {}
        void saveValue(const MiniMC::Model::Register& v, Value&& value) override {
	  this->saveRegister (v,std::move(value));
	}
	
	Value lookupRegisterValue (const MiniMC::Model::Register& r) const  override {return lookupRegister (r);}
	
	
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
      using StackControl = MiniMC::CPA::Common::StackControl<MiniMC::VMT::Concrete::Value>;
      using ConcreteVMState = MiniMC::CPA::Common::VMState<MiniMC::VMT::Concrete::Value,ValueLookupBase,Memory,PathControl,StackControl>;
      using ConcreteVMInitState = MiniMC::CPA::Common::VMInitState<MiniMC::VMT::Concrete::Value,ValueLookupBase,Memory,PathControl>;
      
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
