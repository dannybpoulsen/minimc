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
      using PathFormulaEngine = MiniMC::VMT::Engine<Value,Operations<Value>> ;
      
      class Memory  {
      public:
	Memory (SMTLib::TermBuilder& b);
	Memory (const Memory&) = default;
	
	Value load(const typename Value::Pointer&, const MiniMC::Model::Type_ptr&) const ;
        // First parameter is address to store at, second is the value to state
        void store(const Value::Pointer&, const Value::I8&) ;
	void store(const Value::Pointer&, const Value::I16&) ;
        void store(const Value::Pointer&, const Value::I32&) ;
        void store(const Value::Pointer&, const Value::I64&) ;
	void store(const Value::Pointer&, const Value::Aggregate&) ;
	void store(const Value::Pointer&, const Value::Pointer&) ;
	void store(const Value::Pointer&, const Value::Pointer32&) ;
	
	// PArameter is size to allocate
	Value::Pointer alloca(const Value::I64&) ;
	
        void free(const Value::Pointer&)  {}
        void createHeapLayout(const MiniMC::Model::HeapLayout& ) ;
      private:
	SMTLib::TermBuilder& builder;
	MiniMC::Model::base_t next_block = 0;
	SMTLib::Term_ptr mem_var{nullptr};
      };


      
      using ActivationRecord = MiniMC::CPA::Common::ActivationRecord<MiniMC::VMT::Pathformula::Value>;
      using ActivationStack = MiniMC::CPA::Common::ActivationStack<MiniMC::VMT::Pathformula::Value>;

      class ValueCreator {
      public:
	ValueCreator (SMTLib::TermBuilder& builder) : builder(builder) {}
	Value create (const MiniMC::Model::I8Integer& val)  const; 
	Value create (const MiniMC::Model::I16Integer& val) const ;
	Value create (const MiniMC::Model::I32Integer& val) const ;
	Value create (const MiniMC::Model::I64Integer& val) const ;
	Value create (const MiniMC::Model::Bool& val) const   ;
	Value create (const MiniMC::Model::Pointer& val) const ;
	Value create (const MiniMC::Model::Pointer32& val) const;
	Value create (const MiniMC::Model::AggregateConstant& val) const;
	Value create (const MiniMC::Model::Undef& und) const ;
	Value create(const MiniMC::Model::SymbolicConstant& ) const    {throw MiniMC::Support::Exception ("Cannot Evaluate Symbolic Constants");}
	Value unboundValue (const MiniMC::Model::Type&) const ;
	Value defaultValue(const MiniMC::Model::Type&) const ;
	
      private:
	SMTLib::TermBuilder& builder;
      };
      
      using ValueLookup = MiniMC::CPA::Common::ValueLookup<Value,ValueCreator,MiniMC::CPA::Common::RegisterStore<Value> >;
      using ValueLookupNoRegister = MiniMC::CPA::Common::ValueLookup<Value,ValueCreator >;
      
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

      using PathFormulaState = MiniMC::CPA::Common::VMState<Value,ValueLookup,Memory,PathControl,ActivationStack>;
      
      
      
    } // namespace Concrete
    
  } // namespace VMT
} // namespace MiniMC


#endif
