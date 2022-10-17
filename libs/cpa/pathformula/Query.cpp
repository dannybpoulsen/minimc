#include "cpa/pathformula.hpp"
#include "smt/context.hpp"
#include "state.hpp"
#include "support/smt.hpp"

namespace MiniMC {
  namespace CPA {
    namespace PathFormula {

      CommonState_ptr StateQuery::makeInitialState(const InitialiseDescr& descr) {
	auto& entrypoints = descr.getEntries ();

	if (entrypoints.size () > 1) {
	  throw MiniMC::Support::ConfigurationException ("Pathformula only supports one entry point");
	}
	
	auto& func = entrypoints[0];
	auto& vstack = func->getRegisterStackDescr ();
	auto& termbuilder =  context->getBuilder ();
	
	auto term = termbuilder.makeBoolConst (true);
	
	MiniMC::VMT::Pathformula::ValueLookup lookup{vstack.getTotalRegisters (),termbuilder};
	for (auto& reg : vstack.getRegisters ()) {
	  auto val = lookup.defaultValue (reg->getType ());
	  lookup.saveValue (*reg,std::move(val));
	}
	
	MiniMC::VMT::Pathformula::Memory memory{termbuilder};
	memory.createHeapLayout (descr.getHeap ());
	
	auto state =  std::make_shared<MiniMC::CPA::PathFormula::State>(ActivationStack{{std::move(lookup),nullptr}},
									std::move(memory),
									std::move(term),
									*context);	

	MiniMC::VMT::Pathformula::PathFormulaEngine engine{MiniMC::VMT::Pathformula::PathFormulaEngine::OperationsT{termbuilder},MiniMC::VMT::Pathformula::PathFormulaEngine::CasterT{termbuilder},descr.getProgram ()};
	MiniMC::VMT::Pathformula::PathControl control{termbuilder};
	StackControl stackcontrol{state->getStack (),*context};
        decltype(engine)::State newvm {state->getMemory (),control,stackcontrol};
	
	engine.execute(descr.getInit (),newvm);
	
	return state;
      }

      MiniMC::CPA::CommonState_ptr Joiner::doJoin(const CommonState_ptr&, const CommonState_ptr&) {
	return nullptr;
      }
      
      MiniMC::CPA::CommonState_ptr Transferer::doTransfer(const CommonState_ptr& s, const MiniMC::Model::Edge* e, proc_id id [[maybe_unused]]) {
        assert(id == 0 && "PathFormula only useful for one process systems");
	auto resstate = s->copy();
	auto& nstate = static_cast<MiniMC::CPA::PathFormula::State&>(*resstate);
	MiniMC::VMT::Status status  = MiniMC::VMT::Status::Ok;
	auto& termbuilder = context->getBuilder ();
	MiniMC::VMT::Pathformula::PathControl control{termbuilder};
      
	StackControl stackcontrol{nstate.getStack (),*context};
        
	decltype(engine)::State newvm {nstate.getMemory (),control,stackcontrol};
	auto& instr = e->getInstructions();
	status = engine.execute(instr,newvm);
	
	if (status ==MiniMC::VMT::Status::Ok)  {
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
