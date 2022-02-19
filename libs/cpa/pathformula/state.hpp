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
      
      class State : public MiniMC::CPA::State {
      public:
	State (ValueMap&& vals,  MiniMC::VMT::Pathformula::Memory&& memory, SMTLib::Term_ptr&& formula,SMTLib::Context& ctxt) : values(std::move(vals)),
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
	
        
        const Concretizer_ptr getConcretizer() const override;
	auto& getValues () {return values;}
	auto& getMemory () {return memory;}
	auto& getValues () const {return values;}
	
	void addConstraints (SMTLib::Term_ptr& term) {
	  pathformula = context.getBuilder().buildTerm (SMTLib::Ops::And,{pathformula,term});
	}

	auto& getPathformula () const  {return pathformula;}
	
      private:
	ValueMap values;
	MiniMC::VMT::Pathformula::Memory memory;
        SMTLib::Term_ptr pathformula;
	SMTLib::Context& context;
      };

      class Concretizer : public MiniMC::CPA::Concretizer {
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
	
        virtual std::ostream& evaluate_str(proc_id, const MiniMC::Model::Variable_ptr& v, std::ostream& os) {
	  return os << state.getValues().get(*v);
	}
      private:
	const State& state;
	SMTLib::Solver_ptr solver;
      };

      const Concretizer_ptr State::getConcretizer() const {
	auto solver = context.makeSolver ();
	solver->assert_formula (getPathformula ());
	return std::make_shared<Concretizer>(*this,std::move(solver));
      }

    } // namespace PathFormula
  }   // namespace CPA
} // namespace MiniMC

#endif
