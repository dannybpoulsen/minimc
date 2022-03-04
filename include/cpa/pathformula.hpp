#ifndef _PATHFORMULA__
#define _PATHFORMULA__

#include "cpa/interface.hpp"
#include "vm/pathformula/pathformua.hpp"
#include "model/checkers/HasInstruction.hpp"
#include "support/localisation.hpp"
#include "support/smt.hpp"

namespace MiniMC {
  namespace CPA {
    namespace PathFormula {
      struct StateQuery : public MiniMC::CPA::StateQuery {
	StateQuery (const SMTLib::Context_ptr& context) : context(context) {}
        MiniMC::CPA::State_ptr makeInitialState(const MiniMC::Model::Program&);
      private:
	SMTLib::Context_ptr context;
      };

      struct Transferer : public MiniMC::CPA::Transferer {
	Transferer (const SMTLib::Context_ptr& context) : context(context),
							  engine(MiniMC::VMT::Pathformula::Operations{context->getBuilder()},MiniMC::VMT::Pathformula::Casts{context->getBuilder()}) {}
	MiniMC::CPA::State_ptr doTransfer(const State_ptr& s, const MiniMC::Model::Edge_ptr&, proc_id);
      private:
	SMTLib::Context_ptr context;
		MiniMC::VMT::Pathformula::PathFormulaEngine engine;
      };
      
      struct Joiner : public MiniMC::CPA::Joiner {
	Joiner (const SMTLib::Context_ptr& context) : context(context)
	{}
	MiniMC::CPA::State_ptr doJoin(const State_ptr& l, const State_ptr& r);

        bool covers(const State_ptr&, const State_ptr&) {
          return false;
        }
      private:
	SMTLib::Context_ptr context;

      };

      struct CPA : public ICPA {
	CPA (MiniMC::Support::SMT::SMTDescr fact) : context(fact.makeContext ()) {}
	StateQuery_ptr makeQuery() const { return std::make_shared<StateQuery>(context); }
	Transferer_ptr makeTransfer() const { return std::make_shared<Transferer>(context); }
	Joiner_ptr makeJoin() const { return std::make_shared<Joiner>(context); }
	Storer_ptr makeStore() const { return std::make_shared<Storer>(std::make_shared<Joiner>(context)); }
      private:
	SMTLib::Context_ptr context;
      };

      } // namespace PathFormula
  }   // namespace CPA
} // namespace MiniMC
#endif
