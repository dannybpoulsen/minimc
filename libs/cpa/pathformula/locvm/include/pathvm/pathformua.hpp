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
      
      
      class ValueLookupBase  {
      public:
	ValueLookupBase (SMTLib::TermBuilder& b) : builder(b) {}
	ValueLookupBase (const ValueLookupBase&) = default;
        Value lookupValue (const MiniMC::Model::Value& ) const ;
        Value unboundValue(const MiniMC::Model::Type&) const ;
	Value defaultValue(const MiniMC::Model::Type&) const ;
      
	virtual void saveValue(const MiniMC::Model::Register&, Value&&)  {
	  throw MiniMC::Support::Exception ("Can't save values");
	} 

	virtual Value lookupRegisterValue (const MiniMC::Model::Register&) const {
	  throw MiniMC::Support::Exception ("Can't lookupRegisters");
	}
	
	SMTLib::TermBuilder& builder;
      };

      class ValueLookup : public ValueLookupBase,
			  private MiniMC::CPA::Common::BaseValueLookup<Value>
      {
      public:
	ValueLookup (MiniMC::CPA::Common::ActivationStack<Value>& values, MiniMC::Model::VariableMap<Value>& metas,SMTLib::TermBuilder& b) : ValueLookupBase(b),BaseValueLookup(values,metas) {}

	void saveValue(const MiniMC::Model::Register& v, Value&& value) override {
	  this->saveRegister (v,std::move(value));
	}
	
	Value lookupRegisterValue (const MiniMC::Model::Register& r) const  override {return lookupRegister (r);}
	
	
      };
      
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

      using StackControl = MiniMC::CPA::Common::StackControl<MiniMC::VMT::Pathformula::Value>;
      using PathFormulaState = MiniMC::CPA::Common::VMState<Value,ValueLookupBase,Memory,PathControl,StackControl>;
      using PathFormulaInitState = MiniMC::CPA::Common::VMInitState<Value,ValueLookupBase,Memory,PathControl>;
      
      
      
    } // namespace Concrete
    
  } // namespace VMT
} // namespace MiniMC


#endif
