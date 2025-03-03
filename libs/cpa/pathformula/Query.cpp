#include "minimc/cpa/pathformula.hpp"
#include "minimc/cpa/common.hpp"
#include "minimc/values/pathformula/pathformula.hpp"
#include "smt/context.hpp"
#include "state.hpp"
#include "minimc/smt/smt.hpp"
#include "minimc/support/overload.hpp"

namespace MiniMC {
  namespace CPA {
    namespace PathFormula {
      /*
      using PathFormulaEngine = MiniMC::VMT::Engine<MiniMC::VMT::Pathformula::Value,MiniMC::VMT::Pathformula::Operations,MiniMC::VMT::Pathformula::Memory> ;
      
      using ActivationRecord = MiniMC::CPA::Common::ActivationRecord<MiniMC::VMT::Pathformula::Value>;
      using ActivationStack = MiniMC::CPA::Common::ActivationStack<MiniMC::VMT::Pathformula::Value>;


      using PathFormulaState = MiniMC::CPA::Common::VMState<MiniMC::VMT::Pathformula::Value,MiniMC::CPA::Common::EvaluationContext<MiniMC::VMT::Pathformula::Value,MiniMC::VMT::Pathformula::Memory>,ActivationStack>;


      struct CPA : public ICPA {
	CPA (MiniMC::Support::SMT::SMTDescr fact) : context(fact.makeContext ()) {}
	MiniMC::CPA::State_ptr makeInitialState(const InitialiseDescr&) override;
	Transferer_ptr makeTransfer(const MiniMC::Model::Program& prgm) const { return std::make_shared<Transferer>(context,prgm); }
      private:
	SMTLib::Context_ptr context;
	};
      
      State_ptr CPA::makeInitialState(const InitialiseDescr& descr) {
	auto& termbuilder =  context->getBuilder ();
	
        
	return makeState<MiniMC::CPA::PathFormula::State>(MiniMC::CPA::Common::StateMixin<MiniMC::VMT::Pathformula::Value>::createInitialState(descr,
																	       MiniMC::VMT::Pathformula::Operations{termbuilder},
																	       MiniMC::VMT::Pathformula::Memory{termbuilder}),
							  *context);
      }

      struct Transferer::Internal {
	Internal (SMTLib::Context_ptr context,const MiniMC::Model::Program& prgm) : context(context),
										    engine(MiniMC::VMT::Pathformula::Operations{context->getBuilder()},
											   MiniMC::VMT::Pathformula::Memory{context->getBuilder()},
											   prgm),
										    metas(prgm.getMetaRegs().getTotalRegisters())
	{}
	SMTLib::Context_ptr context;
	PathFormulaEngine engine;
	MiniMC::Model::VariableMap<MiniMC::VMT::Pathformula::Value> metas;
	  
      };

      Transferer::Transferer (const SMTLib::Context_ptr& context,const MiniMC::Model::Program& prgm) : _internal(new Internal (context,prgm)) {}
      Transferer::~Transferer () {}
	
	
      MiniMC::CPA::State_ptr Transferer::doTransfer(const MiniMC::CPA::State& s, const MiniMC::CPA::Transition& trans) {
	const MiniMC::Model::Edge& e = *trans.edge;
	auto resstate = s.copy();
	auto& nstate = static_cast<MiniMC::CPA::PathFormula::State&>(*resstate);
	if (nstate.getProc(trans.proc).activeRecord ().getLocation () != e.getFrom ())
	  return nullptr;
	nstate.getProc(trans.proc ).activeRecord().setLocation (e.getTo ());
	
	
	PathFormulaState newvm {nstate.getProc(trans.proc),nstate.makeEvaluationContext(trans.proc)};
	auto& instr = e.getInstructions();
	auto res = _internal->engine.execute(instr,newvm);
	
	if (res.status == MiniMC::VMT::Status::Ok)  {
	  nstate.getMixin().setPathform (VMT::Pathformula::Operations{_internal->context->getBuilder()}.BoolAnd(nstate.getMixin().getPathform(), res.assumes));
	  return resstate;
	}
	else {	  
	  return nullptr;
	  
	}
      }
      */
      
   using CPA = MiniMC::CPA::Common::CPA<MiniMC::VMT::Pathformula::ValueDefinition>;
      
    
    } // namespace PathFormula
    template<>
    MiniMC::CPA::TCPA_ptr makeCPA<CPAType::Pathformula> (MiniMC::Support::SMT::SMTDescr fact) {
      return std::make_shared<PathFormula::CPA> (fact);
    }
    
  }   // namespace CPA
} // namespace MiniMC
