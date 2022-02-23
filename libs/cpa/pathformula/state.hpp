#ifndef _pathSTATE__
#define _pathSTATE__


#include "cpa/interface.hpp"
#include "vm/pathformula/pathformua.hpp"
#include "smt/context.hpp"
#include "smt/solver.hpp"
#include "support/feedback.hpp"
#include "util/ssamap.hpp"
#include <memory>

namespace MiniMC {
  namespace CPA {
    namespace PathFormula {

      using ValueMap = MiniMC::Util::SSAMap<MiniMC::Model::Register,MiniMC::VMT::Pathformula::PathFormulaVMVal>;

      using SymbolicByteVectorExpr = TByteVectorExpr<MiniMC::VMT::Pathformula::PathFormulaVMVal>;

      struct StackFrame {
	StackFrame (ValueMap&& map, const MiniMC::Model::Value_ptr& ret) : values(std::move(map)),ret(ret) {} 
	ValueMap values;
	MiniMC::Model::Value_ptr ret;
      };

      struct CallStack  {
	explicit CallStack (ValueMap&& map){frames.emplace_back (std::move(map),nullptr);}

	void push  (ValueMap&& map, const MiniMC::Model::Value_ptr& ret) {
	  frames.emplace_back(std::move(map),ret);
	}

	auto pop () {	  
	  auto val = frames.back  ();
	  frames.pop_back ();
	  return val;
	}
	
	
	auto& back ()  {return frames.back ();}
	auto& back () const  {return frames.back ();}
	std::vector<StackFrame> frames;

	
	
      };



      class StackControl : public MiniMC::VMT::StackControl<MiniMC::VMT::Pathformula::PathFormulaVMVal> {
      public:
	StackControl (CallStack& stack, MiniMC::Model::Program& prgm,SMTLib::Context& context) : stack(stack),prgm(prgm),context(context) {}
	//StackControl API
	void  push (MiniMC::pointer_t funcpointer, std::vector<MiniMC::VMT::Pathformula::PathFormulaVMVal>& params, const MiniMC::Model::Value_ptr& ret) override {
	  
	  auto func = prgm.getFunction(MiniMC::Support::getFunctionId(funcpointer));
	  auto& vstack = func->getVariableStackDescr();
	  ValueMap sf {vstack->getTotalVariables ()};
	  MiniMC::VMT::Pathformula::ValueLookup values{sf,context.getBuilder ()};
	  for (auto& v : vstack->getVariables()) {
            values.saveValue  (v,values.unboundValue (v->getType ()));
          }

	  auto it = params.begin ();
	  for (auto& p : func->getParameters ()) {
	    values.saveValue  (p,std::move(*it));
	    ++it;
	      
	  }
	  
	  stack.push (std::move(sf),ret);
	}
	
	void pop (MiniMC::VMT::Pathformula::PathFormulaVMVal&& val) override {
	  auto ret = stack.back ().ret;
	  stack.pop ();
	  MiniMC::VMT::Pathformula::ValueLookup{stack.back().values,context.getBuilder  ()}.saveValue (std::static_pointer_cast<MiniMC::Model::Register> (ret),std::move(val));
	}
	
	void popNoReturn () override {
	  stack.pop ();
	}
      private:
	CallStack& stack;
	MiniMC::Model::Program& prgm;
	SMTLib::Context& context;
      };
      
      class State : public MiniMC::CPA::State
      {
      public:
	State (CallStack&& vals,  MiniMC::VMT::Pathformula::Memory&& memory, SMTLib::Term_ptr&& formula,SMTLib::Context& ctxt) : call_stack(std::move(vals)),
																memory(std::move(memory)),
																pathformula(std::move(formula)),
																context(ctxt)
	{}
	State(const State& oth) = default;
        virtual std::ostream& output(std::ostream& os) const override { return os << "\nPathformula:" << *pathformula; }
        MiniMC::Hash::hash_t hash(MiniMC::Hash::seed_t  = 0) const override { return reinterpret_cast<MiniMC::Hash::hash_t>(this); }
        virtual std::shared_ptr<MiniMC::CPA::State> copy() const override { return std::make_shared<State>(*this); }
        virtual bool need2Store() const override { return false; }
        virtual bool assertViolated() const override { return false; }
	
        
        const Solver_ptr getConcretizer() const override;
	auto& getStack () {return call_stack;}
	auto& getMemory () {return memory;}
	auto& getStack ()  const {return call_stack;}
	
	void addConstraints (SMTLib::Term_ptr& term) {
	  pathformula = context.getBuilder().buildTerm (SMTLib::Ops::And,{pathformula,term});
	}

	auto& getPathformula () const  {return pathformula;}
	
	ByteVectorExpr_ptr symbEvaluate (proc_id, const MiniMC::Model::Variable_ptr& v) const override  {
	  return std::make_unique<SymbolicByteVectorExpr> (getStack().back().values.get(*v),v->getType()->getSize ());
	}

		
      private:
	CallStack  call_stack;
	MiniMC::VMT::Pathformula::Memory memory;
        SMTLib::Term_ptr pathformula;
	SMTLib::Context& context;
      };

      class Concretizer : public MiniMC::CPA::Solver {
      public:
        Concretizer(const State& s, SMTLib::Solver_ptr&& solver) : state(s),solver(std::move(solver))  {
	  this->solver->assert_formula (state.getPathformula ());
        }

        virtual Feasibility isFeasible() const {
	  switch (solver->check_sat ()) {
	  case SMTLib::Result::Satis:
	    return  Feasibility::Feasible;
	  case SMTLib::Result::NSatis:
	    return  Feasibility::Infeasible;
	  default:
	    return Feasibility::Unknown;
	  }
	}
	
      private:
	const State& state;
	SMTLib::Solver_ptr solver;
      };

      const Solver_ptr State::getConcretizer() const {
	auto solver = context.makeSolver ();
	solver->assert_formula (getPathformula ());
	return std::make_shared<Concretizer>(*this,std::move(solver));
      }

    } // namespace PathFormula
  }   // namespace CPA
} // namespace MiniMC

#endif
