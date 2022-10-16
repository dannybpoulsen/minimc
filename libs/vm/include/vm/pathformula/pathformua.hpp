#ifndef _PATH_VM__
#define _PATH_VM__

#include "hash/hashing.hpp"

#include "vm/vmt.hpp"
#include "vm/pathformula/value.hpp"
#include "vm/pathformula/operations.hpp"

#include <memory>

namespace SMTLib {
  class TermBuilder;
  class Term;
  using Term_ptr = std::shared_ptr<Term>; 
}

namespace MiniMC {
  namespace VMT {
    namespace Pathformula {      
      
      using PathFormulaEngine = MiniMC::VMT::Engine<PathFormulaVMVal, Operations<PathFormulaVMVal>, Casts<PathFormulaVMVal>> ;
      
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
        void storeValue(const Value::Pointer&, const Value::Pointer&) override;
	void storeValue(const Value::Pointer&, const Value::Pointer32&) override;
	
	// PArameter is size to allocate
        PathFormulaVMVal alloca(const Value::I64&) override;
	
        void free(const Value::Pointer&) override {}
        void createHeapLayout(const MiniMC::Model::HeapLayout& ) override;
	MiniMC::Hash::hash_t hash() const {return std::bit_cast<uint64_t> (this);}// }throw MiniMC::Support::Exception ("Not implemented");}
      private:
	SMTLib::TermBuilder& builder;
	MiniMC::base_t next_block = 0;
	SMTLib::Term_ptr mem_var{nullptr};
      };

      class ValueLookup : public BaseValueLookup<PathFormulaVMVal> {
      public:
	ValueLookup (std::size_t size, SMTLib::TermBuilder& b) : BaseValueLookup(size),builder(b) {}
	ValueLookup (const ValueLookup&) = default;
        PathFormulaVMVal lookupValue (const MiniMC::Model::Value_ptr& ) const override;
        PathFormulaVMVal unboundValue(const MiniMC::Model::Type_ptr&) const override;
	PathFormulaVMVal defaultValue(const MiniMC::Model::Type_ptr&) const override;
      
	MiniMC::Hash::hash_t hash() const { return 0;}
	
      private:
	SMTLib::TermBuilder& builder;
      };
      
      class PathControl : public MiniMC::VMT::PathControl<PathFormulaVMVal> {
      public:
	PathControl (SMTLib::TermBuilder& builder);
        TriBool addAssumption(const Value::Bool&) override;
        TriBool addAssert(const Value::Bool&) override;
	auto& getAssump () {return assump;}
	auto& getAsserts () {return asserts;}
	
      private:
	SMTLib::Term_ptr assump;
	SMTLib::Term_ptr asserts;
	
	SMTLib::TermBuilder& builder;
	};
      
      
      
    } // namespace Concrete
    
  } // namespace VMT
} // namespace MiniMC


#endif
