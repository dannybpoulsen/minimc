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

      struct ValidateInstructions : public MiniMC::Support::Sink<MiniMC::Model::Program> {
        ValidateInstructions(MiniMC::Support::Messager& ptr) : mess(ptr) {}
        virtual bool run(MiniMC::Model::Program& prgm) {
          for (auto& F : prgm.getEntryPoints()) {
            for (auto& E : F->getCFG()->getEdges()) {
              if (E->hasAttribute<MiniMC::Model::AttributeType::Instructions>()) {
                for (auto& I : E->getAttribute<MiniMC::Model::AttributeType::Instructions>()) {
                  if (I.getOpcode() == MiniMC::Model::InstructionCode::Call) {
                    MiniMC::Support::Localiser error_mess("This CPA does not support '%1%' instructions.");
                    mess.error(error_mess.format(I.getOpcode()));
                    return false;
                  }
                }
              }
            }
          }
          return true;
        }

      private:
        MiniMC::Support::Messager& mess;
      };

      struct PrevalidateSetup : MiniMC::CPA::PrevalidateSetup{
        bool validate(const MiniMC::Model::Program& prgm, MiniMC::Support::Messager& mess) {
          return MiniMC::Model::Checkers::HasNoInstruction<MiniMC::Model::InstructionCode::Call>(mess, "This CPA does not support '%1%' instructions.").run(prgm);
        }
      };


      struct CPA : public ICPA {
	CPA (const MiniMC::Support::SMT::SMTFactory_ptr& fact) : context(fact->construct ()) {}
	StateQuery_ptr makeQuery() const { return std::make_shared<StateQuery>(context); }
	Transferer_ptr makeTransfer() const { return std::make_shared<Transferer>(context); }
	Joiner_ptr makeJoin() const { return std::make_shared<Joiner>(context); }
	Storer_ptr makeStore() const { return std::make_shared<Storer>(std::make_shared<Joiner>(context)); }
	PrevalidateSetup_ptr makeValidate() const { return std::make_shared<PrevalidateSetup>(); }
      private:
	MiniMC::Support::SMT::SMTFactory_ptr factory;
	SMTLib::Context_ptr context;
      };

      } // namespace PathFormula
  }   // namespace CPA
} // namespace MiniMC
#endif
