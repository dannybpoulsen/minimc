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
        ConcreteVMVal loadValue(const typename ConcreteVMVal::Pointer&, const MiniMC::Model::Type_ptr&) const;
        // First parameter is address to store at, second is the value to state
        void storeValue(const ConcreteVMVal::Pointer&, const ConcreteVMVal::I8&) ;
	void storeValue(const ConcreteVMVal::Pointer&, const ConcreteVMVal::I16&) ;
        void storeValue(const ConcreteVMVal::Pointer&, const ConcreteVMVal::I32&) ;
        void storeValue(const ConcreteVMVal::Pointer&, const ConcreteVMVal::I64&) ;
	void storeValue(const ConcreteVMVal::Pointer&, const ConcreteVMVal::Aggregate&) ;
	void storeValue(const ConcreteVMVal::Pointer&, const ConcreteVMVal::Pointer&) ;
	void storeValue(const ConcreteVMVal::Pointer&, const ConcreteVMVal::Pointer32&) ;
        
	
	// PArameter is size to allocate
	ConcreteVMVal::Pointer alloca(const ConcreteVMVal::I64&) ;
	
        void free(const ConcreteVMVal::Pointer&);
        void createHeapLayout(const MiniMC::Model::HeapLayout& layout);
        MiniMC::Hash::hash_t hash() const;
      private:
        struct internal;
        std::unique_ptr<internal> _internal;
      };
       
      class ValueLookupBase  {
      public:
	ConcreteVMVal lookupValue (const MiniMC::Model::Value& v) const ;
	ConcreteVMVal unboundValue (const MiniMC::Model::Type&) const ;
	ConcreteVMVal defaultValue(const MiniMC::Model::Type&) const ;
	virtual void saveValue(const MiniMC::Model::Register&, ConcreteVMVal&& )  {throw MiniMC::Support::Exception ("Can't save values");}
	virtual ConcreteVMVal lookupRegisterValue (const MiniMC::Model::Register&) const {throw MiniMC::Support::Exception ("Can't lookupRegisters");}
      };
      
      class ValueLookup : public ValueLookupBase,
			  private MiniMC::CPA::Common::BaseValueLookup<ConcreteVMVal> {
      public:
	ValueLookup (MiniMC::CPA::Common::ActivationStack<ConcreteVMVal > & values,MiniMC::Model::VariableMap<ConcreteVMVal>& metas) : BaseValueLookup<ConcreteVMVal>(values,metas) {}
        void saveValue(const MiniMC::Model::Register& v, ConcreteVMVal&& value) override {
	  this->saveRegister (v,std::move(value));
	}
	
	ConcreteVMVal lookupRegisterValue (const MiniMC::Model::Register& r) const  override {return lookupRegister (r);}
	
	
      };
      
      class PathControl  {
      public:
        TriBool addAssumption(const ConcreteVMVal::Bool& b) {
	  return b.getValue () ? TriBool::True : TriBool::False;
	}
	
        TriBool addAssert(const ConcreteVMVal::Bool& b) {
	  return b.getValue () ? TriBool::True : TriBool::False;
	}

      };

      
      
      using ActivationRecord = MiniMC::CPA::Common::ActivationRecord<MiniMC::VMT::Concrete::ConcreteVMVal>;
      using ActivationStack = MiniMC::CPA::Common::ActivationStack<MiniMC::VMT::Concrete::ConcreteVMVal>;
      using StackControl = MiniMC::CPA::Common::StackControl<MiniMC::VMT::Concrete::ConcreteVMVal>;
      using ConcreteVMState = MiniMC::VMT::VMState<MiniMC::VMT::Concrete::ConcreteVMVal,ValueLookupBase,Memory,PathControl,StackControl>;
      using ConcreteVMInitState = MiniMC::VMT::VMInitState<MiniMC::VMT::Concrete::ConcreteVMVal,ValueLookupBase,Memory,PathControl>;
      
      //ConcreteVMState 
      using ConcreteEngine = MiniMC::VMT::Engine<ConcreteVMVal, MiniMC::VMT::Concrete::Operations >;
      
      
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
