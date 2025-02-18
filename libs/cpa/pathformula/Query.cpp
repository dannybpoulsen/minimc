#include "minimc/cpa/pathformula.hpp"
#include "cpa/common.hpp"
#include "smt/context.hpp"
#include "state.hpp"
#include "minimc/smt/smt.hpp"
#include "minimc/support/overload.hpp"

namespace MiniMC {
  namespace CPA {
    namespace PathFormula {

      using PathFormulaEngine = MiniMC::VMT::Engine<MiniMC::VMT::Pathformula::Value,MiniMC::VMT::Pathformula::Operations,MiniMC::VMT::Pathformula::Memory> ;
      
      using ActivationRecord = MiniMC::CPA::Common::ActivationRecord<MiniMC::VMT::Pathformula::Value>;
      using ActivationStack = MiniMC::CPA::Common::ActivationStack<MiniMC::VMT::Pathformula::Value>;


      using PathFormulaState = MiniMC::CPA::Common::VMState<MiniMC::VMT::Pathformula::Value,MiniMC::CPA::Common::EvaluationContext<MiniMC::VMT::Pathformula::Value,MiniMC::VMT::Pathformula::Memory>,ActivationStack>;
      
      
      State_ptr CPA::makeInitialState(const InitialiseDescr& descr) {
	auto& termbuilder =  context->getBuilder ();
	auto term = termbuilder.makeBoolConst (true);
	
        
	return makeState<MiniMC::CPA::PathFormula::State>(MiniMC::CPA::Common::StateMixin<MiniMC::VMT::Pathformula::Value>::createInitialState(descr,
														     MiniMC::VMT::Pathformula::Operations{termbuilder},
														     MiniMC::VMT::Pathformula::Memory{termbuilder}),
							  std::move(term),
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
	  nstate.addConstraints (res.assumes.getTerm());
	  return resstate;
	}
	else {	  
	  return nullptr;
	  
	}
      }
	

      

    } // namespace PathFormula
  }   // namespace CPA
} // namespace MiniMC
