#ifndef _PATH_VM__
#define _PATH_VM__

#include "hash/hashing.hpp"
#include "support/casts.hpp"
#include "support/operataions.hpp"
#include "support/pointer.hpp"

#include "vm/vmt.hpp"
#include "vm/pathformula/value.hpp"
#include "vm/pathformula/operations.hpp"
#include "util/ssamap.hpp"

#include <memory>

namespace SMTLib {
  class TermBuilder;
  class Term;
  using Term_ptr = std::shared_ptr<Term>; 
}

namespace MiniMC {
  namespace VMT {
    namespace Pathformula {      
      
      using PathFormulaEngine = MiniMC::VMT::Engine<PathFormulaVMVal, Operations, Casts >;
      
      class Memory : public MiniMC::VMT::Memory<PathFormulaVMVal> {
      public:
	Memory (SMTLib::TermBuilder& b) : builder(b) {} 
	PathFormulaVMVal loadValue(const typename PathFormulaVMVal::Pointer&, const MiniMC::Model::Type_ptr&) const override;
        // First parameter is address to store at, second is the value to state
        void storeValue(const PathFormulaVMVal::Pointer&, const PathFormulaVMVal::I8&) override {}
	void storeValue(const PathFormulaVMVal::Pointer&, const PathFormulaVMVal::I16&) override {}
        void storeValue(const PathFormulaVMVal::Pointer&, const PathFormulaVMVal::I32&) override {}
        void storeValue(const PathFormulaVMVal::Pointer&, const PathFormulaVMVal::I64&) override  {}
        void storeValue(const PathFormulaVMVal::Pointer&, const PathFormulaVMVal::Pointer&) override {}
        
	// PArameter is size to allocate
        PathFormulaVMVal alloca(const PathFormulaVMVal::I64&) override;
	
        void free(const PathFormulaVMVal::Pointer&) override {}
        void createHeapLayout(const MiniMC::Model::HeapLayout& ) override {}
	MiniMC::Hash::hash_t hash() const {return std::bit_cast<uint64_t> (this);}// }throw MiniMC::Support::Exception ("Not implemented");}
      private:
	SMTLib::TermBuilder& builder;
      };

      class ValueLookup : public MiniMC::VMT::ValueLookup<PathFormulaVMVal> {
      public:
	ValueLookup (MiniMC::Util::SSAMap<MiniMC::Model::Register,MiniMC::VMT::Pathformula::PathFormulaVMVal>& val, SMTLib::TermBuilder& b) : values(val),builder(b) {}
	ValueLookup (const ValueLookup&) = default;
        PathFormulaVMVal lookupValue (const MiniMC::Model::Value_ptr& ) const override;
	
        void saveValue(const MiniMC::Model::Register& v, PathFormulaVMVal&& value) override {
	  values.set(v,std::move(value));
        }
        PathFormulaVMVal unboundValue(const MiniMC::Model::Type_ptr&) const override;
        MiniMC::Hash::hash_t hash() const { return 0;}
	
      private:
	MiniMC::Util::SSAMap<MiniMC::Model::Register,MiniMC::VMT::Pathformula::PathFormulaVMVal>& values;
	SMTLib::TermBuilder& builder;
      };
      
      class PathControl : public MiniMC::VMT::PathControl<PathFormulaVMVal> {
      public:
	PathControl (SMTLib::TermBuilder& builder);
        TriBool addAssumption(const PathFormulaVMVal::Bool&) override;
        TriBool addAssert(const PathFormulaVMVal::Bool&) override;
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
