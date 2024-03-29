#ifndef _CONCRETE_VM__
#define _CONCRETE_VM__

#include "hash/hashing.hpp"
#include "host/casts.hpp"
#include "host/operataions.hpp"

#include "cpa/common.hpp"
#include "vm/vmt.hpp"
#include "concvm/value.hpp"
#include "concvm/operations.hpp"

#include <memory>

namespace MiniMC {
  namespace VMT {
    namespace Concrete {
      
      class Memory : public MiniMC::VMT::Memory<ConcreteVMVal> {
      public:
	Memory ();
	Memory (const Memory&);
	~Memory ();
        ConcreteVMVal loadValue(const typename ConcreteVMVal::Pointer&, const MiniMC::Model::Type_ptr&) const override;
        // First parameter is address to store at, second is the value to state
        void storeValue(const Value::Pointer&, const ConcreteVMVal::I8&) override;
	void storeValue(const Value::Pointer&, const Value::I16&) override;
        void storeValue(const Value::Pointer&, const Value::I32&) override;
        void storeValue(const Value::Pointer&, const Value::I64&) override;
	void storeValue(const Value::Pointer&, const Value::Aggregate&) override;
	void storeValue(const Value::Pointer&, const Value::Pointer&) override;
	void storeValue(const Value::Pointer&, const Value::Pointer32&) override;
        
	
	// PArameter is size to allocate
        Value alloca(const Value::I64&) override;
	
        void free(const Value::Pointer&) override;
        void createHeapLayout(const MiniMC::Model::HeapLayout& layout) override;
        MiniMC::Hash::hash_t hash() const;
      private:
        struct internal;
        std::unique_ptr<internal> _internal;
      };
       
      class ValueLookupBase : public MiniMC::VMT::ValueLookup<ConcreteVMVal> {
      public:
	ConcreteVMVal lookupValue (const MiniMC::Model::Value& v) const override;
	ConcreteVMVal unboundValue (const MiniMC::Model::Type&) const override;
	ConcreteVMVal defaultValue(const MiniMC::Model::Type&) const override;
	void saveValue(const MiniMC::Model::Register&, ConcreteVMVal&& ) override {throw MiniMC::Support::Exception ("Can't save values");}
	virtual ConcreteVMVal lookupRegisterValue (const MiniMC::Model::Register&) const {throw MiniMC::Support::Exception ("Can't lookupRegisters");}
      };
      
      class ValueLookup : public ValueLookupBase,
			  private MiniMC::CPA::Common::BaseValueLookup<ConcreteVMVal> {
      public:
	ValueLookup (MiniMC::CPA::Common::ActivationStack<ConcreteVMVal > & values) : BaseValueLookup<ConcreteVMVal>(values) {}
        void saveValue(const MiniMC::Model::Register& v, ConcreteVMVal&& value) override {
	  this->saveRegister (v,std::move(value));
	}
	
	ConcreteVMVal lookupRegisterValue (const MiniMC::Model::Register& r) const  override {return lookupRegister (r);}
      
	
      };
      
      class PathControl : public MiniMC::VMT::PathControl<ConcreteVMVal> {
      public:
        TriBool addAssumption(const Value::Bool& b) override{
	  return b.getValue () ? TriBool::True : TriBool::False;
	}
        TriBool addAssert(const Value::Bool& b) override {
	  return b.getValue () ? TriBool::True : TriBool::False;
	}

      };

      
      
      using ActivationRecord = MiniMC::CPA::Common::ActivationRecord<MiniMC::VMT::Concrete::ConcreteVMVal>;
      using ActivationStack = MiniMC::CPA::Common::ActivationStack<MiniMC::VMT::Concrete::ConcreteVMVal>;
      using ConcreteVMState = MiniMC::VMT::VMState<MiniMC::VMT::Concrete::ConcreteVMVal>;
      using ConcreteVMInitState = MiniMC::VMT::VMInitState<MiniMC::VMT::Concrete::ConcreteVMVal>;
      
      //ConcreteVMState 
      using ConcreteEngine = MiniMC::VMT::Engine<MiniMC::VMT::Concrete::Operations, MiniMC::VMT::Concrete::Caster >;
      
      
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
  struct hash<MiniMC::VMT::Concrete::ConcreteVMVal> {
    auto operator()(const MiniMC::VMT::Concrete::ConcreteVMVal& t) { return t.hash(); }
  };
  
  template <>
  struct hash<MiniMC::VMT::Concrete::Memory> {
    auto operator()(const MiniMC::VMT::Concrete::Memory& t) { return t.hash(); }
  };
  
} // namespace std

#endif
