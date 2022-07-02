#ifndef _CONCRETE_VM__
#define _CONCRETE_VM__

#include "hash/hashing.hpp"
#include "host/casts.hpp"
#include "host/operataions.hpp"
#include "support/pointer.hpp"

#include "vm/vmt.hpp"
#include "vm/concrete/value.hpp"
#include "vm/concrete/operations.hpp"

#include <memory>

namespace MiniMC {
  namespace VMT {
    namespace Concrete {


      
      
      using ConcreteEngine = MiniMC::VMT::Engine<ConcreteVMVal, Operations, Caster >;
      
      class Memory : public MiniMC::VMT::Memory<ConcreteVMVal> {
      public:
	Memory ();
	Memory (const Memory&);
	~Memory ();
        ConcreteVMVal loadValue(const typename ConcreteVMVal::Pointer&, const MiniMC::Model::Type_ptr&) const override;
        // First parameter is address to store at, second is the value to state
        void storeValue(const ConcreteVMVal::Pointer&, const ConcreteVMVal::I8&) override;
	void storeValue(const ConcreteVMVal::Pointer&, const ConcreteVMVal::I16&) override;
        void storeValue(const ConcreteVMVal::Pointer&, const ConcreteVMVal::I32&) override;
        void storeValue(const ConcreteVMVal::Pointer&, const ConcreteVMVal::I64&) override;
        void storeValue(const ConcreteVMVal::Pointer&, const ConcreteVMVal::Pointer&) override;
        
	// PArameter is size to allocate
        ConcreteVMVal alloca(const ConcreteVMVal::I64&) override;

        void free(const ConcreteVMVal::Pointer&) override;
        void createHeapLayout(const MiniMC::Model::HeapLayout& layout) override;
        MiniMC::Hash::hash_t hash() const;
	
      private:
        struct internal;
        std::unique_ptr<internal> _internal;
      };
      
      class ValueLookup : public MiniMC::VMT::BaseValueLookup<ConcreteVMVal> {
      public:
	ValueLookup (std::size_t i) : BaseValueLookup<ConcreteVMVal>(i) {}
        ConcreteVMVal lookupValue (const MiniMC::Model::Value_ptr& v) const override;
	ConcreteVMVal unboundValue (const MiniMC::Model::Type_ptr&) const override;
	ConcreteVMVal defaultValue(const MiniMC::Model::Type_ptr&) const override;
	
      };
      
      class PathControl : public MiniMC::VMT::PathControl<ConcreteVMVal> {
      public:
        TriBool addAssumption(const ConcreteVMVal::Bool& b) override{
	  return b.getValue () ? TriBool::True : TriBool::False;
	}
        TriBool addAssert(const ConcreteVMVal::Bool& b) override {
	    return b.getValue () ? TriBool::True : TriBool::False;
	}
      };

      
      
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
  struct hash<MiniMC::VMT::Concrete::ValueLookup> {
    auto operator()(const MiniMC::VMT::Concrete::ValueLookup& t) { return t.hash(); }
  };

  template <>
  struct hash<MiniMC::VMT::Concrete::Memory> {
    auto operator()(const MiniMC::VMT::Concrete::Memory& t) { return t.hash(); }
  };
  
} // namespace std

#endif
