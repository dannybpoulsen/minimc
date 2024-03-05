#include "minimc/cpa/pathformula.hpp"
#include "smt/context.hpp"
#include "state.hpp"
#include "minimc/smt/smt.hpp"
#include "minimc/support/overload.hpp"

namespace MiniMC {
  namespace CPA {
    namespace PathFormula {

      DataState_ptr CPA::makeInitialState(const InitialiseDescr& descr) {
	auto& entrypoints = descr.getEntries ();

	if (entrypoints.size () != 1) {
	  throw MiniMC::Support::ConfigurationException ("Pathformula only supports one entry point");
	}
	
	auto& func = entrypoints[0].getFunction();
	auto& vstack = func->getRegisterDescr ();
	auto& termbuilder =  context->getBuilder ();
	
	
	auto term = termbuilder.makeBoolConst (true);
	MiniMC::Model::VariableMap<MiniMC::VMT::Pathformula::Value> gvalues {descr.getProgram().getCPURegs().getTotalRegisters ()};
	MiniMC::Model::VariableMap<MiniMC::VMT::Pathformula::Value> values {vstack.getTotalRegisters ()};
	
	MiniMC::VMT::Pathformula::ActivationStack stack{std::move(gvalues),MiniMC::VMT::Pathformula::ActivationRecord{std::move(values),nullptr}};
	MiniMC::VMT::Pathformula::Memory memory{termbuilder};
	auto state =  std::make_shared<MiniMC::CPA::PathFormula::State>(std::move(stack),
									std::move(memory),
									std::move(term),
									*context);
	MiniMC::Model::VariableMap<MiniMC::VMT::Pathformula::Value> metas{1};
	MiniMC::VMT::Pathformula::ValueLookup lookup{state->getStack (),metas,termbuilder};
	
	for (auto& reg : vstack.getRegisters ()) {
	  auto val = lookup.defaultValue (*reg->getType ());
	  lookup.saveValue (*reg,std::move(val));
	}

	for (auto& reg : descr.getProgram().getCPURegs().getRegisters()) {
	  auto val = lookup.defaultValue (*reg->getType ());
	  lookup.saveValue (*reg,std::move(val));
	}

	
	
	auto pit = entrypoints[0].getParams ().begin ();
	auto rit = func->getParameters().begin ();
	for (; pit != entrypoints[0].getParams ().end ();++pit,++rit) {
	  lookup.saveValue  (**rit,lookup.lookupValue (**pit));
	} 
	  

	memory.createHeapLayout (descr.getHeap ());
	for (auto& b : descr.getHeap ()) {
	  if (b.value) {
	    VMT::Pathformula::Value ptr = lookup.lookupValue (MiniMC::Model::Pointer (b.baseobj));
            VMT::Pathformula::Value valueToStor = lookup.lookupValue(*b.value);
	    VMT::Pathformula::Value::visit (MiniMC::Support::Overload {
		
		[&memory]<typename K>(VMT::Pathformula::Value::Pointer& ptr, K& value) requires (!std::is_same_v<K,VMT::Pathformula::Value::Bool>) {
		  memory.storeValue (ptr,value);
		},
		  [](auto&, auto&) {
		    throw MiniMC::Support::Exception ("Error");
		  },
		
		  
		  },
	      ptr,
	      valueToStor
	      );
	    }
	}
	
	return state;
      }

      struct Transferer::Internal {
	Internal (SMTLib::Context_ptr context,const MiniMC::Model::Program& prgm) : context(context),
										    engine(MiniMC::VMT::Pathformula::PathFormulaEngine::OperationsT{context->getBuilder()},prgm),
										    metas(prgm.getMetaRegs().getTotalRegisters())
	{}
	SMTLib::Context_ptr context;
	MiniMC::VMT::Pathformula::PathFormulaEngine engine;
	MiniMC::Model::VariableMap<MiniMC::VMT::Pathformula::Value> metas;
	  
      };

      Transferer::Transferer (const SMTLib::Context_ptr& context,const MiniMC::Model::Program& prgm) : _internal(new Internal (context,prgm)) {}
      Transferer::~Transferer () {}
	
	
      MiniMC::CPA::DataState_ptr Transferer::doTransfer(const DataState& s, const MiniMC::CPA::Transition& trans) {
	const MiniMC::Model::Edge& e = *trans.edge;
	assert(trans.proc == 0 && "PathFormula only useful for one process systems");
	auto resstate = s.copy();
	auto& nstate = static_cast<MiniMC::CPA::PathFormula::State&>(*resstate);
	MiniMC::VMT::Status status  = MiniMC::VMT::Status::Ok;
	auto& termbuilder = _internal->context->getBuilder ();
	
	MiniMC::VMT::Pathformula::PathControl control{termbuilder};
	MiniMC::VMT::Pathformula::StackControl stackcontrol{nstate.getStack ()};
	MiniMC::VMT::Pathformula::ValueLookup lookup{nstate.getStack(),_internal->metas,termbuilder};
	
	MiniMC::VMT::Pathformula::PathFormulaState newvm {nstate.getMemory (),control,stackcontrol,lookup};
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
