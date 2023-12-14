#ifndef _PATHFORMULA__
#define _PATHFORMULA__

#include "cpa/interface.hpp"
#include "support/localisation.hpp"
#include "smt/smt.hpp"

#include <memory>

namespace MiniMC {
  namespace CPA {
    namespace PathFormula {
      
      struct Transferer : public MiniMC::CPA::TTransfer<DataState> {
	Transferer (const SMTLib::Context_ptr& context, const MiniMC::Model::Program& prgm);
	~Transferer ();
	MiniMC::CPA::State_ptr<DataState> doTransfer(const DataState& s, const MiniMC::CPA::Transition&) override;
      private:
	struct Internal;
	std::unique_ptr<Internal> _internal;
      };
      

      struct CPA : public ICPA<DataState> {
	CPA (MiniMC::Support::SMT::SMTDescr fact) : context(fact.makeContext ()) {}
	MiniMC::CPA::DataState_ptr makeInitialState(const InitialiseDescr&) override;
	TTransferer_ptr<DataState> makeTransfer(const MiniMC::Model::Program& prgm) const { return std::make_shared<Transferer>(context,prgm); }
      private:
	SMTLib::Context_ptr context;
      };

      } // namespace PathFormula
  }   // namespace CPA
} // namespace MiniMC
#endif
