#ifndef _PATHFORMULA__
#define _PATHFORMULA__

#include "cpa/interface.hpp"
#include "vm/pathformula/pathformua.hpp"
#include "model/checkers/HasInstruction.hpp"
#include "support/localisation.hpp"
#include "smt/smt.hpp"

namespace MiniMC {
  namespace CPA {
    namespace PathFormula {
      struct StateQuery : public MiniMC::CPA::StateQuery {
	StateQuery (const SMTLib::Context_ptr& context) : context(context) {}
        MiniMC::CPA::CommonState_ptr makeInitialState(const InitialiseDescr&) override;
      private:
	SMTLib::Context_ptr context;
      };

      struct Transferer : public MiniMC::CPA::Transferer {
	Transferer (const SMTLib::Context_ptr& context, const MiniMC::Model::Program& prgm) : context(context),
											      engine(MiniMC::VMT::Pathformula::PathFormulaEngine::OperationsT{context->getBuilder()},MiniMC::VMT::Pathformula::PathFormulaEngine::CasterT{context->getBuilder()},prgm) {}
	MiniMC::CPA::CommonState_ptr doTransfer(const CommonState_ptr& s, const MiniMC::Model::Edge*, proc_id);
      private:
	SMTLib::Context_ptr context;
	MiniMC::VMT::Pathformula::PathFormulaEngine engine;
      };
      
      struct Joiner : public MiniMC::CPA::Joiner {
	Joiner (const SMTLib::Context_ptr& context) : context(context)
	{}
	MiniMC::CPA::CommonState_ptr doJoin(const CommonState_ptr& l, const CommonState_ptr& r);

        bool covers(const CommonState_ptr&, const CommonState_ptr&) {
          return false;
        }
      private:
	SMTLib::Context_ptr context;
      };
      
      struct CPA : public ICPA {
	CPA (MiniMC::Support::SMT::SMTDescr fact) : context(fact.makeContext ()) {}
	StateQuery_ptr makeQuery() const { return std::make_shared<StateQuery>(context); }
	Transferer_ptr makeTransfer(const MiniMC::Model::Program& prgm) const { return std::make_shared<Transferer>(context,prgm); }
	Joiner_ptr makeJoin() const { return std::make_shared<Joiner>(context); }
      private:
	SMTLib::Context_ptr context;
      };

      } // namespace PathFormula
  }   // namespace CPA
} // namespace MiniMC
#endif
