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
      
      struct Transferer : public MiniMC::CPA::Transferer {
	Transferer (const SMTLib::Context_ptr& context, const MiniMC::Model::Program& prgm);
	~Transferer ();
	MiniMC::CPA::CommonState_ptr doTransfer(const CommonState& s, const MiniMC::Model::Edge&, proc_id);
      private:
	class Internal;
	std::unique_ptr<Internal> _internal;
      };
      
      struct Joiner : public MiniMC::CPA::Joiner {
	Joiner (const SMTLib::Context_ptr& context) : context(context) {}
	MiniMC::CPA::CommonState_ptr doJoin(const CommonState& l, const CommonState& r);
	
        bool covers(const CommonState&, const CommonState&) {
          return false;
        }
      private:
	SMTLib::Context_ptr context;
      };
      
      struct CPA : public ICPA {
	CPA (MiniMC::Support::SMT::SMTDescr fact) : context(fact.makeContext ()) {}
	MiniMC::CPA::CommonState_ptr makeInitialState(const InitialiseDescr&) override;
	Transferer_ptr makeTransfer(const MiniMC::Model::Program& prgm) const { return std::make_shared<Transferer>(context,prgm); }
	Joiner_ptr makeJoin() const { return std::make_shared<Joiner>(context); }
      private:
	SMTLib::Context_ptr context;
      };

      } // namespace PathFormula
  }   // namespace CPA
} // namespace MiniMC
#endif
