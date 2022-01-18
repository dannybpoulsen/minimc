#include <gsl/pointers>
#include <memory>

#include "cpa/concrete.hpp"
#include "hash/hashing.hpp"
#include "heap.hpp"
#include "instructionimpl.hpp"
//#include "util/vm.hpp"
#include "vm/vm.hpp"
#include "vm/concrete.hpp"

namespace MiniMC {
  namespace CPA {
    namespace Concrete {
      class MConcretizer : public MiniMC::CPA::Concretizer {
      public:
        MConcretizer(const std::vector<MiniMC::VM::ValueLookup_ptr>& v) :  vars(v) {}
        virtual MiniMC::CPA::Concretizer::Feasibility isFeasible() const override { return Feasibility::Feasible; }

        virtual std::ostream& evaluate_str(proc_id id, const MiniMC::Model::Variable_ptr& var, std::ostream& os) {
	  return os <<"";
	}

        virtual MiniMC::Util::Array evaluate(proc_id id, const MiniMC::Model::Variable_ptr& var) override {
	  return MiniMC::Util::Array{};
	}

      private:
        const std::vector<MiniMC::VM::ValueLookup_ptr>& vars;
      };

      class State : public MiniMC::CPA::State, public MiniMC::CPA::Concretizer {
      public:
        State( const std::vector<MiniMC::VM::ValueLookup_ptr>& var, MiniMC::VM::Memory_ptr& mem) :  proc_vars(var),heap(mem) {
        }
        virtual std::ostream& output(std::ostream& os) const {
          for (auto& vl : proc_vars) {
            os << "===\n";
            os << vl << "\n";
          }
          return os << "==\n";
        }

        virtual MiniMC::Hash::hash_t hash(MiniMC::Hash::seed_t seed = 0) const override {
          if (!hash_val) {
            for (auto& vl : proc_vars) {
              MiniMC::Hash::hash_combine(seed, *vl);
            }
            MiniMC::Hash::hash_combine(seed, *heap);
            //uncommnented the update of this buffered hash value. It
            //disables the buffering as it might be incorrect
            //The State is really just a container and the parts
            //making up its hash-values can actually change outside
            //its knowledge....making it impossible to keep
            //precomputed hash_value up to date
            //hash_val = seed;
            return seed;
          }
          return hash_val;
        }

        virtual std::shared_ptr<MiniMC::CPA::State> copy() const override {

	  std::vector<MiniMC::VM::ValueLookup_ptr> proc_vars2;
	  MiniMC::VM::Memory_ptr heap2 = heap->copy ();
	  proc_vars2.reserve (proc_vars.size  ());
	  auto insert  = std::back_inserter (proc_vars2);
	  std::for_each (proc_vars.begin(), proc_vars.end  (),[&insert](auto& a) {insert = a->copy ();}); 
	  auto copy = std::make_shared<State>(proc_vars2,heap2); 
	  
	  return copy;
        }

        auto& getProc(std::size_t i) { return proc_vars[i]; }
        auto& getHeap() { return heap; }
	
        auto& getProc(std::size_t i) const { return proc_vars[i]; }
        auto& getHeap() const { return heap; }

        virtual bool need2Store() const override { return false; }
        virtual bool ready2explore() const override  { return true; }
        virtual bool assertViolated() const override  { return false; }

        virtual const Concretizer_ptr getConcretizer() const override { return std::make_shared<MConcretizer> (proc_vars);}

      private:
        std::vector<MiniMC::VM::ValueLookup_ptr> proc_vars;
	MiniMC::VM::Memory_ptr heap;
        mutable MiniMC::Hash::hash_t hash_val = 0;
      };

      MiniMC::CPA::State_ptr StateQuery::makeInitialState(const MiniMC::Model::Program& p) {
        
        std::vector<MiniMC::VM::ValueLookup_ptr> stack;
        for (auto& f : p.getEntryPoints()) {
          auto& vstack = f->getVariableStackDescr();
          stack.push_back(MiniMC::VM::Concrete::makeLookup (vstack->getTotalVariables ()));
	  for (auto& v : vstack->getVariables()) {
            stack.back()->saveValue  (v,stack.back ()->unboundValue (v->getType ()));
          }
        }
	auto heap = MiniMC::VM::Concrete::makeMemory ();
	heap->createHeapLayout (p.getHeapLayout ());
	
        auto state = std::make_shared<State>(stack,heap);
        
        return state;
      }

      MiniMC::CPA::State_ptr Transferer::doTransfer(const MiniMC::CPA::State_ptr& s, const MiniMC::Model::Edge_ptr& e, proc_id id) {
	MiniMC::VM::Engine engine;
        auto resstate = s->copy();
        auto& ostate = static_cast<const MiniMC::CPA::Concrete::State&>(*s);
        auto& nstate = static_cast<MiniMC::CPA::Concrete::State&>(*resstate);
	MiniMC::VM::Status status  = MiniMC::VM::Status::Ok;
	  
	auto control = MiniMC::VM::Concrete::makePathControl ();
        if (e->hasAttribute<MiniMC::Model::AttributeType::Instructions>()) {
	  MiniMC::VM::VMState newvm {nstate.getProc (id),nstate.getHeap (),control};
          auto& instr = e->getAttribute<MiniMC::Model::AttributeType::Instructions>();
	  if (!instr.isPhi ()) {
	    status = engine.execute(instr,newvm,newvm);
	    
	  }
	  else{
	    MiniMC::VM::VMState oldvm {nstate.getProc (id),nstate.getHeap (),control};
	    status = engine.execute(instr,newvm,oldvm);
	    
	  }
	  
	}
	if (status ==MiniMC::VM::Status::Ok)
	  return resstate;
	else {
	  
	  return nullptr;

	}
      }

    } // namespace Concrete
  }   // namespace CPA
} // namespace MiniMC
