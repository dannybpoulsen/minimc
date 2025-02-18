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
        MemoryValue store(const MemoryValue&, const Value::Pointer&, const Value::I8&) const  ;
	MemoryValue store(const MemoryValue&, const Value::Pointer&, const Value::I16&)const  ;
        MemoryValue store(const MemoryValue&, const Value::Pointer&, const Value::I32&) const ;
        MemoryValue store(const MemoryValue&, const Value::Pointer&, const Value::I64&) const ;
	MemoryValue store(const MemoryValue&, const Value::Pointer&, const Value::Aggregate&) const ;
	MemoryValue store(const MemoryValue&, const Value::Pointer&, const Value::Pointer&) const ;
	MemoryValue store(const MemoryValue&, const Value::Pointer&, const Value::Pointer32&) const ;
	
	// PArameter is size to allocate
	MemoryValue allocate(const MemoryValue&, const Value::Pointer&,const Value::I64&) ;	
	Value::Pointer find_space(const MemoryValue&, const Value::I64&) ;
	
        MemoryValue free(const MemoryValue&, const Value::Pointer&);  
	
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
  struct hash<MiniMC::VMT::Concrete::Value> {
    auto operator()(const MiniMC::VMT::Concrete::Value& t) { return t.hash(); }
  };


  
  
  
} // namespace std

#endif
