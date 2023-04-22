#ifndef _PATHFORMULA__
#define _PATHFORMULA__

#include "cpa/interface.hpp"
#include "model/checkers/HasInstruction.hpp"
#include "support/localisation.hpp"
#include "smt/smt.hpp"

#include <memory>

namespace MiniMC {
  namespace CPA {
    namespace PathFormula {
      
      struct Transferer : public MiniMC::CPA::TTransfer<DataState> {
	Transferer (const SMTLib::Context_ptr& context, const MiniMC::Model::Program& prgm);
	~Transferer ();
	MiniMC::CPA::State_ptr<DataState> doTransfer(const DataState& s, const MiniMC::Model::Edge&, proc_id);
      private:
	class Internal;
	std::unique_ptr<Internal> _internal;
      };
      
      struct Joiner : public MiniMC::CPA::TJoiner<DataState> {
	Joiner (const SMTLib::Context_ptr& context) : context(context) {}
	MiniMC::CPA::DataState_ptr doJoin(const DataState& l, const DataState& r);
	
        bool covers(const CommonState&, const CommonState&) {
          return false;
        }
      private:
	SMTLib::Context_ptr context;
      };
      
      struct CPA : public ICPA<DataState> {
	CPA (MiniMC::Support::SMT::SMTDescr fact) : context(fact.makeContext ()) {}
	MiniMC::CPA::DataState_ptr makeInitialState(const InitialiseDescr&) override;
	TTransferer_ptr<DataState> makeTransfer(const MiniMC::Model::Program& prgm) const { return std::make_shared<Transferer>(context,prgm); }
	TJoiner_ptr<DataState> makeJoin() const { return std::make_shared<Joiner>(context); }
      private:
	SMTLib::Context_ptr context;
      };

      } // namespace PathFormula
  }   // namespace CPA
} // namespace MiniMC
#endif
