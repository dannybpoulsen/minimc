#include "minimc/values/pathformula/value.hpp"
#include "minimc/values/pathformula/operations.hpp"
#include "minimc/smt/smt.hpp"


namespace SMTLib {
  class Term;
  using Term_ptr = std::shared_ptr<Term>;
  class Solver;
  class TermBuilder;
} // namespace SMTLib


namespace MiniMC {
  namespace VMT {
    namespace Pathformula {
      class ConstraintSolver {
      public:
	ConstraintSolver (SMTLib::Solver& solver) : smtsolver(solver) {
	}
	void push () {smtsolver.push ();}
	void pop () {smtsolver.pop ();}	

	void addConstraint (Value::Bool b);
	MiniMC::VMT::Feasibility check () const;
	MiniMC::Model::Constant_ptr eval (const Value&) const;
      private:
	SMTLib::Solver& smtsolver;
      };
      
      class ValueDefinition {
      public:
	using Val = MiniMC::VMT::Pathformula::Value;
	
	ValueDefinition (MiniMC::Support::SMT::SMTDescr fact);
	ValueDefinition (const ValueDefinition& ) =default;
	
	Operations ops () const;
	Memory memops () const ;
	ConstraintSolver solver() const ;
      private:
	SMTLib::Context_ptr context;
      };
      
    }
  }
}
