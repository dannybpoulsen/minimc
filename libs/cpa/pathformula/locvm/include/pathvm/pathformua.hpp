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
      using PathFormulaEngine = MiniMC::VMT::Engine<PathFormulaVMVal,Operations<PathFormulaVMVal>> ;
      
      class Memory  {
      public:
	Memory (SMTLib::TermBuilder& b);
	Memory (const Memory&) = default;
	
	PathFormulaVMVal loadValue(const typename PathFormulaVMVal::Pointer&, const MiniMC::Model::Type_ptr&) const ;
        // First parameter is address to store at, second is the value to state
        void storeValue(const PathFormulaVMVal::Pointer&, const PathFormulaVMVal::I8&) ;
	void storeValue(const PathFormulaVMVal::Pointer&, const PathFormulaVMVal::I16&) ;
        void storeValue(const PathFormulaVMVal::Pointer&, const PathFormulaVMVal::I32&) ;
        void storeValue(const PathFormulaVMVal::Pointer&, const PathFormulaVMVal::I64&) ;
	void storeValue(const PathFormulaVMVal::Pointer&, const PathFormulaVMVal::Aggregate&) ;
	void storeValue(const PathFormulaVMVal::Pointer&, const PathFormulaVMVal::Pointer&) ;
	void storeValue(const PathFormulaVMVal::Pointer&, const PathFormulaVMVal::Pointer32&) ;
	
	// PArameter is size to allocate
	PathFormulaVMVal::Pointer alloca(const PathFormulaVMVal::I64&) ;
	
        void free(const PathFormulaVMVal::Pointer&)  {}
        void createHeapLayout(const MiniMC::Model::HeapLayout& ) ;
	MiniMC::Hash::hash_t hash() const {return std::bit_cast<uint64_t> (this);}// }throw MiniMC::Support::Exception ("Not implemented");}
      private:
	SMTLib::TermBuilder& builder;
	MiniMC::Model::base_t next_block = 0;
	SMTLib::Term_ptr mem_var{nullptr};
      };


      
      using ActivationRecord = MiniMC::CPA::Common::ActivationRecord<MiniMC::VMT::Pathformula::PathFormulaVMVal>;
      using ActivationStack = MiniMC::CPA::Common::ActivationStack<MiniMC::VMT::Pathformula::PathFormulaVMVal>;
      
      
      class ValueLookupBase  {
      public:
	ValueLookupBase (SMTLib::TermBuilder& b) : builder(b) {}
	ValueLookupBase (const ValueLookupBase&) = default;
        PathFormulaVMVal lookupValue (const MiniMC::Model::Value& ) const ;
        PathFormulaVMVal unboundValue(const MiniMC::Model::Type&) const ;
	PathFormulaVMVal defaultValue(const MiniMC::Model::Type&) const ;
      
	MiniMC::Hash::hash_t hash() const { return 0;}

	virtual void saveValue(const MiniMC::Model::Register&, PathFormulaVMVal&&)  {
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
      
      class PathControl  {
      public:
	PathControl (SMTLib::TermBuilder& builder);
        TriBool addAssumption(const PathFormulaVMVal::Bool&);
        TriBool addAssert(const PathFormulaVMVal::Bool&);
	auto& getAssump () const {return assump;}
	auto& getAsserts () const {return asserts;}
	
      private:
	SMTLib::Term_ptr assump;
	SMTLib::Term_ptr asserts;
	
	SMTLib::TermBuilder& builder;
      };

      using StackControl = MiniMC::CPA::Common::StackControl<MiniMC::VMT::Pathformula::PathFormulaVMVal>;
      using PathFormulaState = MiniMC::VMT::VMState<PathFormulaVMVal,ValueLookupBase,Memory,PathControl,StackControl>;
      using PathFormulaInitState = MiniMC::VMT::VMInitState<PathFormulaVMVal,ValueLookupBase,Memory,PathControl>;
      
      
      
    } // namespace Concrete
    
  } // namespace VMT
} // namespace MiniMC


#endif
