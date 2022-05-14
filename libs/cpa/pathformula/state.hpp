#ifndef _pathSTATE__
#define _pathSTATE__

#include "cpa/common.hpp"
#include "cpa/interface.hpp"
#include "vm/pathformula/pathformua.hpp"
#include "smt/context.hpp"
#include "smt/solver.hpp"
#include "support/feedback.hpp"
#include <memory>

namespace MiniMC {
  namespace CPA {
    namespace PathFormula {


      using ActivationRecord = MiniMC::CPA::Common::ActivationRecord<MiniMC::VMT::Pathformula::PathFormulaVMVal,MiniMC::VMT::Pathformula::ValueLookup>;

      using ActivationStack = MiniMC::CPA::Common::ActivationStack<ActivationRecord>;

      
      class StackControl : public MiniMC::VMT::StackControl<MiniMC::VMT::Pathformula::PathFormulaVMVal> {
      public:
	StackControl (ActivationStack& stack,SMTLib::Context& context,SMTLib::TermBuilder& builder) : stack(stack),context(context),builder(builder) {}
	//StackControl API
	void  push (std::size_t registers,  const MiniMC::Model::Value_ptr& ret) override {
	  MiniMC::VMT::Pathformula::ValueLookup values{registers,context.getBuilder ()};
	  stack.push ({std::move(values),ret,stack.back().next_stack_alloc});
	}
	
	void pop (MiniMC::VMT::Pathformula::PathFormulaVMVal&& val) override {
	  auto ret = stack.back ().ret;
	  stack.pop ();
	  stack.back().values.saveValue (*std::static_pointer_cast<MiniMC::Model::Register> (ret),std::move(val));
	}
	
	void popNoReturn () override {
	  stack.pop ();
	}
	
	virtual typename MiniMC::VMT::Pathformula::PathFormulaVMVal::Pointer alloc(const MiniMC::VMT::Pathformula::PathFormulaVMVal::I64& size) override {
	  
	  auto ret = stack.back().next_stack_alloc;
	  stack.back().next_stack_alloc = builder.buildTerm(SMTLib::Ops::BVAdd,{ret.getTerm (),size.getTerm ()});
	  
	  
	  return ret;
	}

	MiniMC::VMT::Pathformula::PathFormulaVMVal::Pointer StackPointer () override
	{
	  return stack.back().next_stack_alloc;
	}
	
	void RestoreStackPointer (MiniMC::VMT::Pathformula::PathFormulaVMVal::Pointer&& next) override {
	  stack.back ().next_stack_alloc = std::move(next);
	}
	
	MiniMC::VMT::ValueLookup<MiniMC::VMT::Pathformula::PathFormulaVMVal>& getValueLookup () override {return stack.back().values;}
	
	
      private:
	ActivationStack& stack;
	SMTLib::Context& context;
	SMTLib::TermBuilder& builder;
      };
      
      class State : public MiniMC::CPA::State
      {
      public:
	State (ActivationStack&& vals,  MiniMC::VMT::Pathformula::Memory&& memory, SMTLib::Term_ptr&& formula,SMTLib::Context& ctxt) : call_stack(std::move(vals)),
																memory(std::move(memory)),
																pathformula(std::move(formula)),
																context(ctxt)
	{}
	State(const State& oth) : call_stack(oth.call_stack),memory(oth.memory),pathformula(oth.pathformula),context(oth.context),_hash(0) {}
        virtual std::ostream& output(std::ostream& os) const override { return os << "\nPathformula:" << *pathformula; }
        MiniMC::Hash::hash_t hash() const override {
	  //Hashes for pathformula states makes no sense 
	  //Since they should be usable by all algorithms we just makes a fake hash that increments with each invocation of the hash function
	  static MiniMC::Hash::hash_t nextHash {0};
	  if (!_hash)
	    _hash = ++nextHash;
	  std::cerr << _hash << std::endl;
	  return _hash;
	}
        virtual std::shared_ptr<MiniMC::CPA::State> copy() const override { return std::make_shared<State>(*this); }
        
        
        const Solver_ptr getConcretizer() const override;
	auto& getStack () {return call_stack;}
	auto& getMemory () {return memory;}
	auto& getStack ()  const {return call_stack;}
	
	void addConstraints (SMTLib::Term_ptr& term) {
	  pathformula = context.getBuilder().buildTerm (SMTLib::Ops::And,{pathformula,term});
	}

	auto& getPathformula () const  {return pathformula;}
	
		
      private:
	ActivationStack  call_stack;
	MiniMC::VMT::Pathformula::Memory memory;
        SMTLib::Term_ptr pathformula;
	SMTLib::Context& context;
	mutable MiniMC::Hash::hash_t _hash = 0;
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
