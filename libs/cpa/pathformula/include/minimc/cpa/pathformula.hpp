#ifndef _PATHFORMULA__
#define _PATHFORMULA__

#include "minimc/cpa/interface.hpp"
#include "minimc/support/localisation.hpp"
#include "minimc/smt/smt.hpp"

#include <memory>

namespace MiniMC {
  namespace CPA {
    namespace PathFormula {
      
      struct Transferer : public MiniMC::CPA::Transfer {
	Transferer (const SMTLib::Context_ptr& context, const MiniMC::Model::Program& prgm);
	~Transferer ();
	MiniMC::CPA::State_ptr doTransfer(const DataState& s, const MiniMC::CPA::Transition&) override;
      private:
	struct Internal;
	std::unique_ptr<Internal> _internal;
      };
      

      struct CPA : public ICPA<DataState> {
	CPA (MiniMC::Support::SMT::SMTDescr fact) : context(fact.makeContext ()) {}
	MiniMC::CPA::State_ptr makeInitialState(const InitialiseDescr&) override;
	Transferer_ptr makeTransfer(const MiniMC::Model::Program& prgm) const { return std::make_shared<Transferer>(context,prgm); }
      private:
	SMTLib::Context_ptr context;
      };

      } // namespace PathFormula
  }   // namespace CPA
} // namespace MiniMC
#endif
