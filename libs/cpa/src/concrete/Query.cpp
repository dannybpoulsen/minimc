#include <memory>

#include "cpa/concrete.hpp"
#include "cpa/common.hpp"
#include "hash/hashing.hpp"
//#include "heap.hpp"
//#include "instructionimpl.hpp"
//#include "util/vm.hpp"
#include "concvm/concrete.hpp"
#include "concvm/value.hpp"
#include "concvm/operations.hpp"


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
	MiniMC::Model::Constant_ptr evaluate (const QueryExpr& expr) const override {
	  auto& ref = static_cast<const QExpr&> (expr);
	  return ref.getValue ().visit (MiniMC::VMT::Overload {
	        [](MiniMC::VMT::Concrete::ConcreteVMVal::I8& val) ->MiniMC::Model::Constant_ptr {return std::make_shared<MiniMC::Model::I8Integer> (val.getValue ());},
		[](MiniMC::VMT::Concrete::ConcreteVMVal::I16& val) ->MiniMC::Model::Constant_ptr {return std::make_shared<MiniMC::Model::I16Integer> (val.getValue ());},
		[](MiniMC::VMT::Concrete::ConcreteVMVal::I32& val) ->MiniMC::Model::Constant_ptr {return std::make_shared<MiniMC::Model::I32Integer> (val.getValue ());},
		[](MiniMC::VMT::Concrete::ConcreteVMVal::I64& val) ->MiniMC::Model::Constant_ptr{return std::make_shared<MiniMC::Model::I64Integer> (val.getValue ());},
		[](MiniMC::VMT::Concrete::ConcreteVMVal::Pointer& val) ->MiniMC::Model::Constant_ptr{return std::make_shared<MiniMC::Model::Pointer> (val.getValue ());},
		[](MiniMC::VMT::Concrete::ConcreteVMVal::Pointer32& val) ->MiniMC::Model::Constant_ptr{return std::make_shared<MiniMC::Model::Pointer32> (val.getValue ());},
		[](MiniMC::VMT::Concrete::ConcreteVMVal::Bool& val) ->MiniMC::Model::Constant_ptr{return std::make_shared<MiniMC::Model::Bool> (val.getValue ());},
		[](MiniMC::VMT::Concrete::ConcreteVMVal::Aggregate& val) ->MiniMC::Model::Constant_ptr {return std::make_shared<MiniMC::Model::AggregateConstant> (val.getValue ());},
		});
	}
	
	
      };

      
      class StackControl : public  MiniMC::VMT::StackControl<MiniMC::VMT::Concrete::ConcreteVMVal> {
      public:
	StackControl (MiniMC::VMT::Concrete::ActivationStack& s) : stack (s) {}
	void  push (std::size_t registers, const MiniMC::Model::Value_ptr& ret) override {
	  MiniMC::VMT::Concrete::ActivationRecord sf {{registers},ret};
	  stack.push (std::move(sf));
	}
	
	void pop (MiniMC::VMT::Concrete::ConcreteVMVal&& val) override {
	  auto ret = stack.back ().ret;
	  stack.pop ();
	  stack.back().values.set (*std::static_pointer_cast<MiniMC::Model::Register> (ret),std::move(val));
	}
	
	void popNoReturn () override {
	  stack.pop ();
	}

	
      private:
	MiniMC::VMT::Concrete::ActivationStack& stack;
	
      };
      
      struct Transferer::Internal {
	Internal (const MiniMC::Model::Program& prgm) : engine(MiniMC::VMT::Concrete::ConcreteEngine::OperationsT{},MiniMC::VMT::Concrete::ConcreteEngine::CasterT{},prgm) {}
	MiniMC::VMT::Concrete::ConcreteEngine engine;
      };
      
      Transferer::Transferer (const MiniMC::Model::Program& p) : _internal(new Internal (p)) {}
      Transferer::~Transferer () {}
      
      class State : public MiniMC::CPA::DataState,
		    private MiniMC::CPA::QueryBuilder
      {
      public:
        State( const std::vector<MiniMC::VMT::Concrete::ActivationStack>& var, MiniMC::VMT::Concrete::Memory& mem) :  proc_vars(var),heap(mem) {
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
	
        virtual std::shared_ptr<MiniMC::CPA::DataState> copy() const override {

	  std::vector<MiniMC::VMT::Concrete::ActivationStack> proc_vars2{proc_vars};
	  MiniMC::VMT::Concrete::Memory heap2(heap);
	  return std::make_shared<State>(proc_vars2,heap2); 
	}
	
        auto& getProc(std::size_t i) { return proc_vars.at(i); }
        auto& getHeap() { return heap; }
	
        auto& getProc(std::size_t i) const { return proc_vars.at(i); }
        auto& getHeap() const { return heap; }

        virtual const Solver_ptr getConcretizer() const override { return std::make_shared<MConcretizer> ();}

	//QueryBuilder
	QueryExpr_ptr buildValue (MiniMC::proc_t p, const MiniMC::Model::Value_ptr& val) const override {
	  if (p >= proc_vars.size ()) {
	    throw MiniMC::Support::Exception ("Not enough processes");
	  }
	  
	  MiniMC::VMT::Concrete::ValueLookup lookup{const_cast<MiniMC::VMT::Concrete::ActivationStack&> (proc_vars.at(p))};
	  return std::make_unique<QExpr> (lookup.lookupValue(*val));
	  
	}
	
	const QueryBuilder& getBuilder () const override  {return *this;}
	
	
	
      private:
        std::vector<MiniMC::VMT::Concrete::ActivationStack> proc_vars;
	MiniMC::VMT::Concrete::Memory heap;
      };

      MiniMC::CPA::DataState_ptr CPA::makeInitialState(const InitialiseDescr& descr) {

	MiniMC::VMT::Concrete::Memory heap;
	heap.createHeapLayout (descr.getHeap ());
	
	
        std::vector<MiniMC::VMT::Concrete::ActivationStack> stack;
	for (auto& f : descr.getEntries()) {
          auto& vstack = f->getRegisterDescr();
	  MiniMC::Model::VariableMap<MiniMC::VMT::Concrete::ConcreteVMVal> gvalues {descr.getProgram().getCPURegs().getTotalRegisters ()};
	  MiniMC::Model::VariableMap<MiniMC::VMT::Concrete::ConcreteVMVal> values{vstack.getTotalRegisters ()};
	  MiniMC::VMT::Concrete::ActivationRecord sf {std::move(values),nullptr};
	  MiniMC::VMT::Concrete::ActivationStack cs {std::move(gvalues),std::move(sf)};
	  MiniMC::VMT::Concrete::ValueLookup lookup {cs};
	  for (auto& v : vstack.getRegisters()) {
            lookup.saveValue  (*v,lookup.defaultValue (*v->getType ()));
	    
	  }
	  
          stack.push_back(cs);
	  
        }
	
	auto state = std::make_shared<State>(stack,heap);

	MiniMC::VMT::Concrete::ConcreteEngine engine{MiniMC::VMT::Concrete::ConcreteEngine::OperationsT{},MiniMC::VMT::Concrete::ConcreteEngine::CasterT{},descr.getProgram ()};
	MiniMC::VMT::Concrete::PathControl control;
	StackControl scontrol {state->getProc (0)};
	MiniMC::VMT::Concrete::ValueLookup lookup (state->getProc (0));
	decltype(engine)::State newvm {state->getHeap (),control,scontrol,lookup};
	engine.execute(descr.getInit (),newvm);
	
        return state;
      }

      MiniMC::CPA::State_ptr<DataState> Transferer::doTransfer(const MiniMC::CPA::DataState& s, const MiniMC::Model::Edge& e, proc_id id) {
	auto resstate = s.copy();
        auto& nstate = static_cast<MiniMC::CPA::Concrete::State&>(*resstate);
	MiniMC::VMT::Status status  = MiniMC::VMT::Status::Ok;
	  
	MiniMC::VMT::Concrete::PathControl control;
	StackControl scontrol {nstate.getProc (id)};
	MiniMC::VMT::Concrete::ValueLookup lookup (nstate.getProc (id));
	
	
	decltype(_internal->engine)::State newvm {nstate.getHeap (),control,scontrol,lookup};
	auto& instr = e.getInstructions();
	status = _internal->engine.execute(instr,newvm);
	
	if (status ==MiniMC::VMT::Status::Ok)
	  return resstate;
	else {
	  
	  return nullptr;

	}
      }

    } // namespace Concrete
  }   // namespace CPA
} // namespace MiniMC
