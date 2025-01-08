#ifndef _PATH_VM__
#define _PATH_VM__

#include "minimc/hash/hashing.hpp"

#include "cpa/common.hpp"
#include "minimc/vm/vmt.hpp"
#include "pathvm/value.hpp"
#include "pathvm/operations.hpp"

#include <memory>

namespace SMTLib {
  class TermBuilder;
  class Term;
  using Term_ptr = std::shared_ptr<Term>; 
}

namespace MiniMC {
  namespace VMT {
    namespace Pathformula {      
      
      //PathFormulaState, 
      
      
      

      
      class Memory  {
      public:
	Memory (SMTLib::TermBuilder& b);
	Memory (const Memory&) = default;
	Memory& operator= (Memory&& m) = default;
	Value load(const MemoryValue&, const typename Value::Pointer&, const MiniMC::Model::Type&) const ;
        // First parameter is address to store at, second is the value to state
        MemoryValue store(const MemoryValue&, const Value::Pointer&, const Value::I8&) const ;
	MemoryValue store(const MemoryValue&,const Value::Pointer&, const Value::I16&) const ;
        MemoryValue store(const MemoryValue&,const Value::Pointer&, const Value::I32&) const ;
        MemoryValue store(const MemoryValue&,const Value::Pointer&, const Value::I64&) const ;
	MemoryValue store(const MemoryValue&,const Value::Pointer&, const Value::Aggregate&) const ;
	MemoryValue store(const MemoryValue&,const Value::Pointer&, const Value::Pointer&) const ;
	MemoryValue store(const MemoryValue&,const Value::Pointer&, const Value::Pointer32&) const ;
	
	// PArameter is size to allocate
	MemoryValue allocate(const MemoryValue&,const Value::Pointer&, const Value::I64&) ;
	Value::Pointer  find_space(const MemoryValue&,const Value::I64&) ;
	
	
        MemoryValue free(const MemoryValue& m, const Value::Pointer&)  {return m;}
      private:
	SMTLib::TermBuilder* builder;
      };


      using PathFormulaEngine = MiniMC::VMT::Engine<Value,MemoryValue,Operations,Memory> ;
      
      using ActivationRecord = MiniMC::CPA::Common::ActivationRecord<MiniMC::VMT::Pathformula::Value>;
      using ActivationStack = MiniMC::CPA::Common::ActivationStack<MiniMC::VMT::Pathformula::Value>;

      
      class PathControl  {
      public:
	PathControl (SMTLib::TermBuilder& builder);
        TriBool addAssumption(const Value::Bool&);
        TriBool addAssert(const Value::Bool&);
	auto& getAssump () const {return assump;}
	auto& getAsserts () const {return asserts;}
	
      private:
	SMTLib::Term_ptr assump;
	SMTLib::Term_ptr asserts;
	
	SMTLib::TermBuilder& builder;
      };

      using PathFormulaState = MiniMC::CPA::Common::VMState<Value,CPA::Common::EvaluationContext<Value,MemoryValue,Memory>,MemoryValue,PathControl,ActivationStack>;
      
      
      
    } // namespace Concrete
    
  } // namespace VMT
} // namespace MiniMC


#endif
