#ifndef _pathSTATE__
#define _pathSTATE__

#include "cpa/common.hpp"
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


      using SymbolicByteVectorExpr = TByteVectorExpr<MiniMC::VMT::Pathformula::PathFormulaVMVal>;
      using ActivationRecord = MiniMC::CPA::Common::ActivationRecord<MiniMC::VMT::Pathformula::PathFormulaVMVal,MiniMC::VMT::Pathformula::ValueLookup>;

      using ActivationStack = MiniMC::CPA::Common::ActivationStack<ActivationRecord>;

      
      class StackControl : public MiniMC::VMT::StackControl<MiniMC::VMT::Pathformula::PathFormulaVMVal> {
      public:
	StackControl (ActivationStack& stack,const MiniMC::Model::Program& prgm,SMTLib::Context& context,SMTLib::TermBuilder& builder) : stack(stack),prgm(prgm),context(context),builder(builder) {}
	//StackControl API
	void  push (MiniMC::pointer_t funcpointer, std::vector<MiniMC::VMT::Pathformula::PathFormulaVMVal>& params, const MiniMC::Model::Value_ptr& ret) override {
	  
	  auto func = prgm.getFunction(MiniMC::Support::getFunctionId(funcpointer));
	  auto& vstack = func->getRegisterStackDescr();
	  MiniMC::VMT::Pathformula::ValueLookup values{vstack.getTotalRegisters (),context.getBuilder ()};
	  for (auto& v : vstack.getRegisters()) {
            values.saveValue  (*v,values.unboundValue (v->getType ()));
          }

	  auto it = params.begin ();
	  for (auto& p : func->getParameters ()) {
	    values.saveValue  (*p,std::move(*it));
	    ++it;
	      
	  }
	  
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
	
	
      private:
	ActivationStack& stack;
	const MiniMC::Model::Program& prgm;
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
	
	ByteVectorExpr_ptr symbEvaluate (proc_id, const MiniMC::Model::Register_ptr&) const override  {
	  //return std::make_unique<SymbolicByteVectorExpr> (getStack().back().values.get(*v),v->getType()->getSize ());
	  return nullptr;
	}

		
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
