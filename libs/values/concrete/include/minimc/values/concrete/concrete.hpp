#ifndef _CONCRETE_VM__
#define _CONCRETE_VM__

#include "minimc/hash/hashing.hpp"
#include "minimc/host/casts.hpp"
#include "minimc/host/operataions.hpp"

#include "minimc/vm/vmt.hpp"
#include "minimc/values/concrete/value.hpp"
#include "minimc/values/concrete/operations.hpp"

#include <memory>

namespace MiniMC {
  namespace VMT {
    namespace Concrete {
      
      class Memory  {
      public:
	Memory ();
	std::generator<Value::I8> loadBytes(const MemoryValue& mvalue, const typename Value::Pointer& p, std::size_t bytes) const;	

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
      
      
      class ConstraintSolver {
      public:
	void push () {}
	void pop () {}
	
	void addConstraint (Value::Bool constraint) {constraints = constraints.getValue() && constraint.getValue();}
	MiniMC::VMT::Feasibility check () const {
	  return constraints.getValue() ? MiniMC::VMT::Feasibility::Feasible : MiniMC::VMT::Feasibility::Infeasible; 
	}
	
	MiniMC::Model::Constant_ptr eval (const Value& ) const;
	
      private:
	Value::Bool constraints{true};
      };

      class ValueDefinition  {
      public:
	using Val = Value;
	Operations ops () const {return Operations{};}
	Memory memops () const {return Memory{};}
	ConstraintSolver solver () const  {return ConstraintSolver{};}
	ConstraintSolver solver (MiniMC::VMT::SolverOptions) const  {return ConstraintSolver{};}
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
