#include <gsl/pointers>
#include <memory>

#include "cpa/concrete.hpp"
#include "cpa/common.hpp"
#include "hash/hashing.hpp"
//#include "heap.hpp"
//#include "instructionimpl.hpp"
//#include "util/vm.hpp"
#include "vm/concrete/concrete.hpp"

namespace MiniMC {
  namespace CPA {
    namespace Concrete {
      class MConcretizer : public MiniMC::CPA::Solver {
      public:
        MConcretizer()  {}
        virtual MiniMC::CPA::Solver::Feasibility isFeasible() const override { return Feasibility::Feasible; }
	
      };

      using ConcreteByteVectorExpr = TByteVectorExpr<MiniMC::VMT::Concrete::ConcreteVMVal>;

      using ActivationRecord = MiniMC::CPA::Common::ActivationRecord<MiniMC::VMT::Concrete::ConcreteVMVal,MiniMC::VMT::Concrete::ValueLookup>;

      using ActivationStack = MiniMC::CPA::Common::ActivationStack<ActivationRecord>;
      
      class StackControl : public  MiniMC::VMT::StackControl<MiniMC::VMT::Concrete::ConcreteVMVal> {
      public:
	StackControl (ActivationStack& s, const MiniMC::Model::Program& p) : stack (s),prgm(p) {}
	void  push (MiniMC::pointer_t funcpointer, std::vector<MiniMC::VMT::Concrete::ConcreteVMVal>& params, const MiniMC::Model::Value_ptr& ret) override {
	  
	  auto& cur = stack.back ();
	  auto func = prgm.getFunction(MiniMC::Support::getFunctionId(funcpointer));
	  auto& vstack = func->getRegisterStackDescr();
	  ActivationRecord sf {{vstack.getTotalRegisters ()},ret,cur.next_stack_alloc};
	  for (auto& v : vstack.getRegisters()) {
            sf.values.saveValue  (*v,sf.values.unboundValue (v->getType ()));
          }

	  auto it = params.begin ();
	  for (auto& p : func->getParameters ()) {
	    sf.values.saveValue  (*p,std::move(*it));
	    ++it;
	      
	  }
	  
	  stack.push (std::move(sf));
	}
	
	void pop (MiniMC::VMT::Concrete::ConcreteVMVal&& val) override {
	  auto ret = stack.back ().ret;
	  stack.pop ();
	  stack.back().values.saveValue (*std::static_pointer_cast<MiniMC::Model::Register> (ret),std::move(val));
	}
	
	void popNoReturn () override {
	  stack.pop ();
	}

        virtual typename MiniMC::VMT::Concrete::ConcreteVMVal::Pointer alloc(const MiniMC::VMT::Concrete::ConcreteVMVal::I64& size) override {
	  auto ret = stack.back().next_stack_alloc;
	  stack.back().next_stack_alloc = MiniMC::Support::ptradd (ret.getValue (),size.getValue ());
	  return ret;
	}
	
      private:
	ActivationStack& stack;
	const MiniMC::Model::Program& prgm;
	
      };
      
      class State : public MiniMC::CPA::State {
      public:
        State( const std::vector<ActivationStack>& var, MiniMC::VMT::Concrete::Memory& mem) :  proc_vars(var),heap(mem) {
        }

	virtual std::ostream& output(std::ostream& os) const override {
          /*for (auto& vl : proc_vars) {
            os << "===\n";
            os << vl << "\n";
	    }*/
          return os << "==\n";
        }

        virtual MiniMC::Hash::hash_t hash() const override {
	  MiniMC::Hash::hash_t seed{0};
	  for (auto& vl : proc_vars) {
	    MiniMC::Hash::hash_combine(seed, vl);
	  }
	  MiniMC::Hash::hash_combine(seed, heap);
	  return seed;
	}
	
        virtual std::shared_ptr<MiniMC::CPA::State> copy() const override {

	  std::vector<ActivationStack> proc_vars2{proc_vars};
	  MiniMC::VMT::Concrete::Memory heap2(heap);
	  auto copy = std::make_shared<State>(proc_vars2,heap2); 
	  
	  return copy;
        }

	ByteVectorExpr_ptr symbEvaluate (proc_id id, const MiniMC::Model::Register_ptr& val) const override {
	  auto& lookup = proc_vars.at(id).back().values;
	  return std::make_unique<ConcreteByteVectorExpr> (lookup.lookupValue(val),val->getType()->getSize ());
	}
      
	
        auto& getProc(std::size_t i) { return proc_vars[i]; }
        auto& getHeap() { return heap; }
	
        auto& getProc(std::size_t i) const { return proc_vars[i]; }
        auto& getHeap() const { return heap; }

        virtual const Solver_ptr getConcretizer() const override { return std::make_shared<MConcretizer> ();}

      private:
        std::vector<ActivationStack> proc_vars;
	MiniMC::VMT::Concrete::Memory heap;
      };

      MiniMC::CPA::State_ptr StateQuery::makeInitialState(const InitialiseDescr& descr) {

	MiniMC::VMT::Concrete::Memory heap;
	heap.createHeapLayout (descr.getHeap ());
	
	
        std::vector<ActivationStack> stack;
        for (auto& f : descr.getEntries()) {
          auto& vstack = f->getRegisterStackDescr();
	  ActivationRecord sf {{vstack.getTotalRegisters ()},
			 nullptr,
			 heap.alloca (MiniMC::VMT::Concrete::ConcreteVMVal::I64{100}).as<MiniMC::VMT::Concrete::ConcreteVMVal::Pointer> ()
	  };
	  for (auto& v : vstack.getRegisters()) {
            sf.values.saveValue  (*v,sf.values.unboundValue (v->getType ()));
          }
	  ActivationStack cs {std::move(sf)};
          stack.push_back(cs);
	  
        }
	
        auto state = std::make_shared<State>(stack,heap);

	MiniMC::VMT::Concrete::ConcreteEngine engine{MiniMC::VMT::Concrete::Operations{},MiniMC::VMT::Concrete::Caster{}};
	MiniMC::VMT::Concrete::PathControl control;
	StackControl scontrol {state->getProc (0),descr.getProgram ()};
	decltype(engine)::State newvm {state->getProc (0).back().values,state->getHeap (),control,scontrol};
	engine.execute(descr.getInit (),newvm);
	
        return state;
      }

      MiniMC::CPA::State_ptr Transferer::doTransfer(const MiniMC::CPA::State_ptr& s, const MiniMC::Model::Edge_ptr& e, proc_id id) {
	auto resstate = s->copy();
        auto& nstate = static_cast<MiniMC::CPA::Concrete::State&>(*resstate);
	MiniMC::VMT::Status status  = MiniMC::VMT::Status::Ok;
	  
	MiniMC::VMT::Concrete::PathControl control;
	StackControl scontrol {nstate.getProc (id),e->getProgram ()};
	
	if (e->hasAttribute<MiniMC::Model::AttributeType::Instructions>()) {
	  decltype(engine)::State newvm {nstate.getProc (id).back().values,nstate.getHeap (),control,scontrol};
	  auto& instr = e->getAttribute<MiniMC::Model::AttributeType::Instructions>();
	  status = engine.execute(instr,newvm);
	  
	}
	if (status ==MiniMC::VMT::Status::Ok)
	  return resstate;
	else {
	  
	  return nullptr;

	}
      }

    } // namespace Concrete
  }   // namespace CPA
} // namespace MiniMC
