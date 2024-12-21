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
      using PathFormulaEngine = MiniMC::VMT::Engine<Value,Operations> ;
      
      struct Inner {
	Inner (SMTLib::TermBuilder& b) : b(b) {}SMTLib::TermBuilder& b;
      };
      
      class Memory  {
      public:
	Memory (SMTLib::TermBuilder& b);
	Memory (const Memory&) = default;
	Memory& operator= (Memory&& m) = default;
	Value load(const typename Value::Pointer&, const MiniMC::Model::Type&) const ;
        // First parameter is address to store at, second is the value to state
        Memory store(const Value::Pointer&, const Value::I8&) ;
	Memory store(const Value::Pointer&, const Value::I16&) ;
        Memory store(const Value::Pointer&, const Value::I32&) ;
        Memory store(const Value::Pointer&, const Value::I64&) ;
	Memory store(const Value::Pointer&, const Value::Aggregate&) ;
	Memory store(const Value::Pointer&, const Value::Pointer&) ;
	Memory store(const Value::Pointer&, const Value::Pointer32&) ;
	
	// PArameter is size to allocate
	Memory allocate(const Value::Pointer&, const Value::I64&) ;
	Value::Pointer  find_space(const Value::I64&) ;
	
	
        void free(const Value::Pointer&)  {}
        void createHeapLayout(const MiniMC::Model::HeapLayout&, MiniMC::CPA::Common::StaticContext<Value>&) ;
      private:
	SMTLib::TermBuilder* builder;
	MiniMC::Model::base_t next_block = 0;
	SMTLib::Term_ptr mem_var{nullptr};
      };


      
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

      using PathFormulaState = MiniMC::CPA::Common::VMState<Value,CPA::Common::EvaluationContext<Value,Memory>,Memory,PathControl,ActivationStack>;
      
      
      
    } // namespace Concrete
    
  } // namespace VMT
} // namespace MiniMC


#endif
