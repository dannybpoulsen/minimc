#include <gsl/pointers>
#include <memory>

#include "cpa/concrete.hpp"
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

      struct StackFrame {
	StackFrame (std::size_t size, const MiniMC::Model::Value_ptr& ret)  : values(size),ret(ret) {}
	StackFrame (const StackFrame& ) = default;

	MiniMC::Hash::hash_t hash() const {
	  MiniMC::Hash::seed_t seed{0};
	  MiniMC::Hash::hash_combine(seed, values);
	  //MiniMC::Hash::hash_combine(seed,*ret.get());
          return seed;
	}

	MiniMC::VMT::Concrete::ValueLookup values;
	MiniMC::Model::Value_ptr ret{nullptr};
	
      };

      
      struct CallStack  {
	CallStack (StackFrame&& sf) {
	  frames.push_back (std::move(sf));
	}
	CallStack (const CallStack&) = default;
	
	auto pop () {	  
	  auto val = frames.back  ();
	  frames.pop_back ();
	  return val;
	}
	
	auto push (StackFrame&& frame) {
	  frames.push_back (std::move(frame));
	}

	auto& back () {return frames.back ();}
	auto& back () const {return frames.back ();}

	MiniMC::Hash::hash_t hash(MiniMC::Hash::seed_t seed = 0) const {
	  for (auto& vl : frames) {
	    MiniMC::Hash::hash_combine(seed, vl);
	  }
	  return seed;
	}
	
	std::vector<StackFrame> frames;
      };

      class StackControl : public  MiniMC::VMT::StackControl<MiniMC::VMT::Concrete::ConcreteVMVal> {
      public:
	StackControl (CallStack& s, const MiniMC::Model::Program& p) : stack (s),prgm(p) {}
	void  push (MiniMC::pointer_t funcpointer, std::vector<MiniMC::VMT::Concrete::ConcreteVMVal>& params, const MiniMC::Model::Value_ptr& ret) override {

	  auto func = prgm.getFunction(MiniMC::Support::getFunctionId(funcpointer));
	  auto& vstack = func->getRegisterStackDescr();
	  StackFrame sf {vstack.getTotalRegisters (),ret};
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
	
	
      private:
	CallStack& stack;
	const MiniMC::Model::Program& prgm;
	
      };
      
      class State : public MiniMC::CPA::State {
      public:
        State( const std::vector<CallStack>& var, MiniMC::VMT::Concrete::Memory& mem) :  proc_vars(var),heap(mem) {
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

	  std::vector<CallStack> proc_vars2{proc_vars};
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
        std::vector<CallStack> proc_vars;
	MiniMC::VMT::Concrete::Memory heap;
      };

      MiniMC::CPA::State_ptr StateQuery::makeInitialState(const InitialiseDescr& descr) {
        
        std::vector<CallStack> stack;
        for (auto& f : descr.getEntries()) {
          auto& vstack = f->getRegisterStackDescr();
	  StackFrame sf {vstack.getTotalRegisters (),nullptr};
	  for (auto& v : vstack.getRegisters()) {
            sf.values.saveValue  (*v,sf.values.unboundValue (v->getType ()));
          }
	  CallStack cs {std::move(sf)};
          stack.push_back(cs);
	  
        }
	MiniMC::VMT::Concrete::Memory heap;
	heap.createHeapLayout (descr.getHeap ());
	
        auto state = std::make_shared<State>(stack,heap);

	MiniMC::VMT::Concrete::ConcreteEngine engine{MiniMC::VMT::Concrete::Operations{},MiniMC::VMT::Concrete::Caster{}};
	MiniMC::VMT::Concrete::PathControl control;
	StackControl scontrol {state->getProc (0),descr.getProgram ()};
	decltype(engine)::State newvm {state->getProc (0).back().values,state->getHeap (),control,scontrol};
	decltype(engine)::ConstState oldvm {state->getProc (0).back().values,state->getHeap (),control,scontrol};
	engine.execute(descr.getInit (),newvm,oldvm);
	
        return state;
      }

      MiniMC::CPA::State_ptr Transferer::doTransfer(const MiniMC::CPA::State_ptr& s, const MiniMC::Model::Edge_ptr& e, proc_id id) {
	auto resstate = s->copy();
        auto& ostate = static_cast<const MiniMC::CPA::Concrete::State&>(*s);
        auto& nstate = static_cast<MiniMC::CPA::Concrete::State&>(*resstate);
	MiniMC::VMT::Status status  = MiniMC::VMT::Status::Ok;
	  
	MiniMC::VMT::Concrete::PathControl control;
	StackControl scontrol {nstate.getProc (id),e->getProgram ()};
	
	if (e->hasAttribute<MiniMC::Model::AttributeType::Instructions>()) {
	  decltype(engine)::State newvm {nstate.getProc (id).back().values,nstate.getHeap (),control,scontrol};
	  decltype(engine)::ConstState convm {nstate.getProc (id).back().values,nstate.getHeap (),control,scontrol};
          auto& instr = e->getAttribute<MiniMC::Model::AttributeType::Instructions>();
	  if (!instr.isPhi ()) {
	    status = engine.execute(instr,newvm,convm);
	    
	  }
	  else{
	    decltype(engine)::ConstState oldvm {ostate.getProc (id).back().values,ostate.getHeap (),control,scontrol};
	    status = engine.execute(instr,newvm,oldvm);
	    
	  }
	  
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
