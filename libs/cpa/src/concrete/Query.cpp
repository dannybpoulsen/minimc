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
      class QExpr : public MiniMC::CPA::QueryExpr {
      public:
	QExpr (MiniMC::VMT::Concrete::ConcreteVMVal&& val) : value(std::move(val)) {}
	std::ostream& output (std::ostream& os) const override {
	  return os << value; 
	}

	auto getValue () const {return value;}
	
      private:
	MiniMC::VMT::Concrete::ConcreteVMVal value;
      };

      class MConcretizer : public MiniMC::CPA::Solver {
      public:
        MConcretizer()  {}
	MiniMC::CPA::Solver::Feasibility isFeasible() const override { return Feasibility::Feasible; }
	MiniMC::VMT::Concrete::ConcreteVMVal evaluate (const QueryExpr& expr) const override {
	  auto& ref = static_cast<const QExpr&> (expr);
	  return ref.getValue ();
	}
      
	
      };

      
      using ActivationRecord = MiniMC::CPA::Common::ActivationRecord<MiniMC::VMT::Concrete::ConcreteVMVal,MiniMC::VMT::Concrete::ValueLookup>;

      using ActivationStack = MiniMC::CPA::Common::ActivationStack<ActivationRecord>;
      
      class StackControl : public  MiniMC::VMT::StackControl<MiniMC::VMT::Concrete::ConcreteVMVal> {
      public:
	StackControl (ActivationStack& s) : stack (s) {}
	void  push (std::size_t registers, const MiniMC::Model::Value_ptr& ret) override {
	  ActivationRecord sf {{registers},ret};
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

	MiniMC::VMT::ValueLookup<MiniMC::VMT::Concrete::ConcreteVMVal>& getValueLookup () override {return stack.back().values;}

	
      private:
	ActivationStack& stack;
	
      };
      
      class State : public MiniMC::CPA::DataState,
		    private MiniMC::CPA::QueryBuilder
      {
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
	  MiniMC::Hash::Hasher hash;
	  for (auto& vl : proc_vars) {
	    hash << vl;
	  }
	  hash << heap;
	  return hash;
	}
	
        virtual std::shared_ptr<MiniMC::CPA::CommonState> copy() const override {

	  std::vector<ActivationStack> proc_vars2{proc_vars};
	  MiniMC::VMT::Concrete::Memory heap2(heap);
	  auto copy = std::make_shared<State>(proc_vars2,heap2); 
	  
	  return copy;
        }
	
        auto& getProc(std::size_t i) { return proc_vars[i]; }
        auto& getHeap() { return heap; }
	
        auto& getProc(std::size_t i) const { return proc_vars[i]; }
        auto& getHeap() const { return heap; }

        virtual const Solver_ptr getConcretizer() const override { return std::make_shared<MConcretizer> ();}

	//QueryBuilder
	QueryExpr_ptr buildValue (MiniMC::proc_t p, const MiniMC::Model::Value_ptr& val) const override {
	  if (p >= proc_vars.size ()) {
	    throw MiniMC::Support::Exception ("Not enough processes");
	  }
	  return std::make_unique<QExpr> (proc_vars.at(p).back ().values.lookupValue (val));
	}

	const QueryBuilder& getBuilder () const override  {return *this;}
	
	
	
      private:
        std::vector<ActivationStack> proc_vars;
	MiniMC::VMT::Concrete::Memory heap;
      };

      MiniMC::CPA::CommonState_ptr StateQuery::makeInitialState(const InitialiseDescr& descr) {

	MiniMC::VMT::Concrete::Memory heap;
	heap.createHeapLayout (descr.getHeap ());
	
	
        std::vector<ActivationStack> stack;
        for (auto& f : descr.getEntries()) {
          auto& vstack = f->getRegisterStackDescr();
	  ActivationRecord sf {{vstack.getTotalRegisters ()},
			 nullptr};
	  for (auto& v : vstack.getRegisters()) {
            sf.values.saveValue  (*v,sf.values.defaultValue (v->getType ()));
          }
	  ActivationStack cs {std::move(sf)};
          stack.push_back(cs);
	  
        }
	
        auto state = std::make_shared<State>(stack,heap);

	  MiniMC::VMT::Concrete::ConcreteEngine engine{MiniMC::VMT::Concrete::ConcreteEngine::OperationsT{},MiniMC::VMT::Concrete::ConcreteEngine::CasterT{},descr.getProgram ()};
	MiniMC::VMT::Concrete::PathControl control;
	StackControl scontrol {state->getProc (0)};
	decltype(engine)::State newvm {state->getHeap (),control,scontrol};
	engine.execute(descr.getInit (),newvm);
	
        return state;
      }

      MiniMC::CPA::CommonState_ptr Transferer::doTransfer(const MiniMC::CPA::CommonState_ptr& s, const MiniMC::Model::Edge* e, proc_id id) {
	auto resstate = s->copy();
        auto& nstate = static_cast<MiniMC::CPA::Concrete::State&>(*resstate);
	MiniMC::VMT::Status status  = MiniMC::VMT::Status::Ok;
	  
	MiniMC::VMT::Concrete::PathControl control;
	StackControl scontrol {nstate.getProc (id)};
	
	decltype(engine)::State newvm {nstate.getHeap (),control,scontrol};
	auto& instr = e->getInstructions();
	status = engine.execute(instr,newvm);
	
	if (status ==MiniMC::VMT::Status::Ok)
	  return resstate;
	else {
	  
	  return nullptr;

	}
      }

    } // namespace Concrete
  }   // namespace CPA
} // namespace MiniMC
