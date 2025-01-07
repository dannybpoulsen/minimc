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
	Value load(const MemoryValue&, const typename Value::Pointer&, const MiniMC::Model::Type&) const;
        // First parameter is address to store at, second is the value to state
        MemoryValue store(const MemoryValue&, const Value::Pointer&, const Value::I8&) ;
	MemoryValue store(const MemoryValue&, const Value::Pointer&, const Value::I16&) ;
        MemoryValue store(const MemoryValue&, const Value::Pointer&, const Value::I32&) ;
        MemoryValue store(const MemoryValue&, const Value::Pointer&, const Value::I64&) ;
	MemoryValue store(const MemoryValue&, const Value::Pointer&, const Value::Aggregate&) ;
	MemoryValue store(const MemoryValue&, const Value::Pointer&, const Value::Pointer&) ;
	MemoryValue store(const MemoryValue&, const Value::Pointer&, const Value::Pointer32&) ;
	
	// PArameter is size to allocate
	MemoryValue allocate(const MemoryValue&, const Value::Pointer&,const Value::I64&) ;	
	Value::Pointer find_space(const MemoryValue&, const Value::I64&) ;
	
        MemoryValue free(const MemoryValue&, const Value::Pointer&);  
	
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
      using ConcreteVMState = MiniMC::CPA::Common::VMState<MiniMC::VMT::Concrete::Value,MiniMC::CPA::Common::EvaluationContext<MiniMC::VMT::Concrete::Value,MemoryValue,Memory>,MemoryValue,PathControl,ActivationStack>;
      
      //ConcreteVMState 
      using ConcreteEngine = MiniMC::VMT::Engine<Value, MemoryValue, MiniMC::VMT::Concrete::Operations, Memory>;
      
      
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


  
  
  
} // namespace std

#endif
