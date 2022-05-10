#include "cpa/pathformula.hpp"
#include "smt/context.hpp"
#include "state.hpp"
#include "support/smt.hpp"
#include <gsl/pointers>

namespace MiniMC {
  namespace CPA {
    namespace PathFormula {

      State_ptr StateQuery::makeInitialState(const InitialiseDescr& descr) {
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
	  auto val = lookup.unboundValue (reg->getType ());
	  lookup.saveValue (*reg,std::move(val));
	}
	
	MiniMC::VMT::Pathformula::Memory memory{termbuilder};
	memory.createHeapLayout (descr.getHeap ());
	auto stacksize = std::make_shared<MiniMC::Model::I64Integer> (100);
	auto stack = memory.alloca (lookup.lookupValue(stacksize).as<MiniMC::VMT::Pathformula::I64Value> ());
	
	auto state =  std::make_shared<MiniMC::CPA::PathFormula::State>(ActivationStack{{std::move(lookup),nullptr,stack.as<MiniMC::VMT::Pathformula::PointerValue> ()}},
									std::move(memory),
									std::move(term),
									*context);	

	MiniMC::VMT::Pathformula::PathFormulaEngine engine{MiniMC::VMT::Pathformula::Operations{termbuilder},MiniMC::VMT::Pathformula::Casts{termbuilder}};
	MiniMC::VMT::Pathformula::PathControl control{termbuilder};
	StackControl stackcontrol{state->getStack (),descr.getProgram (),*context,termbuilder};
        decltype(engine)::State newvm {state->getStack().back().values,state->getMemory (),control,stackcontrol};

	engine.execute(descr.getInit (),newvm);
	
	return state;
      }

      MiniMC::CPA::State_ptr Joiner::doJoin(const State_ptr&, const State_ptr&) {
	return nullptr;
      }
      
      MiniMC::CPA::State_ptr Transferer::doTransfer(const State_ptr& s, const MiniMC::Model::Edge_ptr& e, proc_id id) {
        assert(id == 0 && "PathFormula only useful for one process systems");
	auto resstate = s->copy();
	auto& nstate = static_cast<MiniMC::CPA::PathFormula::State&>(*resstate);
	MiniMC::VMT::Status status  = MiniMC::VMT::Status::Ok;
	auto& termbuilder = context->getBuilder ();
	MiniMC::VMT::Pathformula::PathControl control{termbuilder};
	
	//MiniMC::VMT::Pathformula::ValueLookup nlookup {nstate.getStack().back().values,termbuilder};
	StackControl stackcontrol{nstate.getStack (),e->getProgram (),*context,termbuilder};
        if (e->hasAttribute<MiniMC::Model::AttributeType::Instructions>()) {
	  decltype(engine)::State newvm {nstate.getStack().back().values,nstate.getMemory (),control,stackcontrol};
	  auto& instr = e->getAttribute<MiniMC::Model::AttributeType::Instructions>();
	  
	  status = engine.execute(instr,newvm);
	  
	}
	
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
