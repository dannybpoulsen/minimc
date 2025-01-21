#include "minimc/cpa/pathformula.hpp"
#include "cpa/common.hpp"
#include "smt/context.hpp"
#include "state.hpp"
#include "minimc/smt/smt.hpp"
#include "minimc/support/overload.hpp"

namespace MiniMC {
  namespace CPA {
    namespace PathFormula {

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
	MiniMC::VMT::Pathformula::PathFormulaEngine engine;
	MiniMC::Model::VariableMap<MiniMC::VMT::Pathformula::Value> metas;
	  
      };

      Transferer::Transferer (const SMTLib::Context_ptr& context,const MiniMC::Model::Program& prgm) : _internal(new Internal (context,prgm)) {}
      Transferer::~Transferer () {}
	
	
      MiniMC::CPA::State_ptr Transferer::doTransfer(const MiniMC::CPA::State& s, const MiniMC::CPA::Transition& trans) {
	const MiniMC::Model::Edge& e = *trans.edge;
	assert(trans.proc == 0 && "PathFormula only useful for one process systems");
	auto resstate = s.copy();
	auto& nstate = static_cast<MiniMC::CPA::PathFormula::State&>(*resstate);
	if (nstate.getStack().activeRecord ().getLocation () != e.getFrom ())
	  return nullptr;
	nstate.getStack().activeRecord().setLocation (e.getTo ());
	MiniMC::VMT::Status status  = MiniMC::VMT::Status::Ok;
	auto& termbuilder = _internal->context->getBuilder ();
	
	MiniMC::VMT::Pathformula::PathControl control{termbuilder};
	
	MiniMC::VMT::Pathformula::PathFormulaState newvm {control,
							  nstate.getStack(),
							  nstate.makeEvaluationContext(trans.proc)};
	auto& instr = e.getInstructions();
	status = _internal->engine.execute(instr,newvm);
	
	if (status == MiniMC::VMT::Status::Ok)  {
	  if (control.getAssump ()) 
	    nstate.addConstraints (control.getAssump ());
	  return resstate;
	}
	else {	  
	  return nullptr;

	}
      }
	

      

    } // namespace PathFormula
  }   // namespace CPA
} // namespace MiniMC
