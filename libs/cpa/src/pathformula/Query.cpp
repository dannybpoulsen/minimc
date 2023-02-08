#include "cpa/pathformula.hpp"
#include "smt/context.hpp"
#include "state.hpp"
#include "smt/smt.hpp"

namespace MiniMC {
  namespace CPA {
    namespace PathFormula {

      CommonState_ptr CPA::makeInitialState(const InitialiseDescr& descr) {
	auto& entrypoints = descr.getEntries ();

	if (entrypoints.size () > 1) {
	  throw MiniMC::Support::ConfigurationException ("Pathformula only supports one entry point");
	}
	
	auto& func = entrypoints[0];
	auto& vstack = func->getRegisterDescr ();
	auto& termbuilder =  context->getBuilder ();
	
	
	auto term = termbuilder.makeBoolConst (true);
	MiniMC::Model::VariableMap<MiniMC::VMT::Pathformula::PathFormulaVMVal> gvalues {descr.getProgram().getCPURegs().getTotalRegisters ()};
	MiniMC::Model::VariableMap<MiniMC::VMT::Pathformula::PathFormulaVMVal> values {vstack.getTotalRegisters ()};
	
	MiniMC::VMT::Pathformula::ActivationStack stack{std::move(gvalues),MiniMC::VMT::Pathformula::ActivationRecord{std::move(values),nullptr}};
	MiniMC::VMT::Pathformula::Memory memory{termbuilder};
	auto state =  std::make_shared<MiniMC::CPA::PathFormula::State>(std::move(stack),
									std::move(memory),
									std::move(term),
									*context);	
	MiniMC::VMT::Pathformula::ValueLookup lookup{state->getStack (),termbuilder};
	
	for (auto& reg : vstack.getRegisters ()) {
	  auto val = lookup.defaultValue (reg->getType ());
	  lookup.saveValue (*reg,std::move(val));
	}
	

	memory.createHeapLayout (descr.getHeap ());
	
	
	MiniMC::VMT::Pathformula::PathFormulaEngine engine{MiniMC::VMT::Pathformula::PathFormulaEngine::OperationsT{termbuilder},MiniMC::VMT::Pathformula::PathFormulaEngine::CasterT{termbuilder},descr.getProgram ()};
	MiniMC::VMT::Pathformula::PathControl control{termbuilder};
	StackControl stackcontrol{state->getStack (),*context};
	decltype(engine)::State newvm {state->getMemory (),control,stackcontrol,lookup};
	
	engine.execute(descr.getInit (),newvm);
	
	return state;
      }

      MiniMC::CPA::CommonState_ptr Joiner::doJoin(const CommonState&, const CommonState&) {
	return nullptr;
      }


      struct Transferer::Internal {
	Internal (SMTLib::Context_ptr context,const MiniMC::Model::Program& prgm) : context(context),
		      engine(MiniMC::VMT::Pathformula::PathFormulaEngine::OperationsT{context->getBuilder()},MiniMC::VMT::Pathformula::PathFormulaEngine::CasterT{context->getBuilder()},prgm) {}
	SMTLib::Context_ptr context;
	MiniMC::VMT::Pathformula::PathFormulaEngine engine;
	
	
      };

      Transferer::Transferer (const SMTLib::Context_ptr& context,const MiniMC::Model::Program& prgm) : _internal(new Internal (context,prgm)) {}
      Transferer::~Transferer () {}
	
	
      MiniMC::CPA::CommonState_ptr Transferer::doTransfer(const CommonState& s, const MiniMC::Model::Edge& e, proc_id id [[maybe_unused]]) {
        assert(id == 0 && "PathFormula only useful for one process systems");
	auto resstate = s.copy();
	auto& nstate = static_cast<MiniMC::CPA::PathFormula::State&>(*resstate);
	MiniMC::VMT::Status status  = MiniMC::VMT::Status::Ok;
	auto& termbuilder = _internal->context->getBuilder ();
	MiniMC::VMT::Pathformula::PathControl control{termbuilder};
	
	StackControl stackcontrol{nstate.getStack (),*_internal->context};
	MiniMC::VMT::Pathformula::ValueLookup lookup{nstate.getStack(),termbuilder};
	
	decltype(_internal->engine)::State newvm {nstate.getMemory (),control,stackcontrol,lookup};
	auto& instr = e.getInstructions();
	status = _internal->engine.execute(instr,newvm);
	
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
