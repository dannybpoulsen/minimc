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
      using PathFormulaState = MiniMC::VMT::VMState<PathFormulaVMVal>;
      using PathFormulaInitState = MiniMC::VMT::VMInitState<PathFormulaVMVal>;
      
      //PathFormulaState, 
      using PathFormulaEngine = MiniMC::VMT::Engine<PathFormulaVMVal,Operations<PathFormulaVMVal>> ;
      
      class Memory : public MiniMC::VMT::Memory<PathFormulaVMVal> {
      public:
	Memory (SMTLib::TermBuilder& b);
	Memory (const Memory&) = default;
	
	PathFormulaVMVal loadValue(const typename Value::Pointer&, const MiniMC::Model::Type_ptr&) const override;
        // First parameter is address to store at, second is the value to state
        void storeValue(const Value::Pointer&, const Value::I8&) override;
	void storeValue(const Value::Pointer&, const Value::I16&) override;
        void storeValue(const Value::Pointer&, const Value::I32&) override;
        void storeValue(const Value::Pointer&, const Value::I64&) override;
	void storeValue(const Value::Pointer&, const Value::Aggregate&) override;
	void storeValue(const Value::Pointer&, const Value::Pointer&) override;
	void storeValue(const Value::Pointer&, const Value::Pointer32&) override;
	
	// PArameter is size to allocate
        PathFormulaVMVal alloca(const Value::I64&) override;
	
        void free(const Value::Pointer&) override {}
        void createHeapLayout(const MiniMC::Model::HeapLayout& ) override;
	MiniMC::Hash::hash_t hash() const {return std::bit_cast<uint64_t> (this);}// }throw MiniMC::Support::Exception ("Not implemented");}
      private:
	SMTLib::TermBuilder& builder;
	MiniMC::Model::base_t next_block = 0;
	SMTLib::Term_ptr mem_var{nullptr};
      };


      
      using ActivationRecord = MiniMC::CPA::Common::ActivationRecord<MiniMC::VMT::Pathformula::PathFormulaVMVal>;
      using ActivationStack = MiniMC::CPA::Common::ActivationStack<MiniMC::VMT::Pathformula::PathFormulaVMVal>;
      
      
      class ValueLookupBase : public MiniMC::VMT::ValueLookup<PathFormulaVMVal> {
      public:
	ValueLookupBase (SMTLib::TermBuilder& b) : builder(b) {}
	ValueLookupBase (const ValueLookupBase&) = default;
        PathFormulaVMVal lookupValue (const MiniMC::Model::Value& ) const override;
        PathFormulaVMVal unboundValue(const MiniMC::Model::Type&) const override;
	PathFormulaVMVal defaultValue(const MiniMC::Model::Type&) const override;
      
	MiniMC::Hash::hash_t hash() const { return 0;}

	void saveValue(const MiniMC::Model::Register&, PathFormulaVMVal&&) override {
	  throw MiniMC::Support::Exception ("Can't save values");
	} 

	virtual PathFormulaVMVal lookupRegisterValue (const MiniMC::Model::Register&) const {
	  throw MiniMC::Support::Exception ("Can't lookupRegisters");
	}
	
	  
      
	SMTLib::TermBuilder& builder;
      };

      class ValueLookup : public ValueLookupBase,
			  private MiniMC::CPA::Common::BaseValueLookup<PathFormulaVMVal>
      {
      public:
	ValueLookup (MiniMC::CPA::Common::ActivationStack<PathFormulaVMVal>& values, MiniMC::Model::VariableMap<PathFormulaVMVal>& metas,SMTLib::TermBuilder& b) : ValueLookupBase(b),BaseValueLookup(values,metas) {}

	void saveValue(const MiniMC::Model::Register& v, PathFormulaVMVal&& value) override {
	  this->saveRegister (v,std::move(value));
	}

	PathFormulaVMVal lookupRegisterValue (const MiniMC::Model::Register& r) const  override {return lookupRegister (r);}
	
	
      };
      
      class PathControl : public MiniMC::VMT::PathControl<PathFormulaVMVal> {
      public:
	PathControl (SMTLib::TermBuilder& builder);
        TriBool addAssumption(const Value::Bool&) override;
        TriBool addAssert(const Value::Bool&) override;
	auto& getAssump () const {return assump;}
	auto& getAsserts () const {return asserts;}
	
      private:
	SMTLib::Term_ptr assump;
	SMTLib::Term_ptr asserts;
	
	SMTLib::TermBuilder& builder;
      };
      
      
      
    } // namespace Concrete
    
  } // namespace VMT
} // namespace MiniMC


#endif
