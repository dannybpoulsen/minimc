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
	ValueMap values{vstack.getTotalRegisters ()};
	
	MiniMC::VMT::Pathformula::ValueLookup lookup{values,termbuilder};
	for (auto& reg : vstack.getRegisters ()) {
	  auto val = lookup.unboundValue (reg->getType ());
	  lookup.saveValue (*reg,std::move(val));
	}
	
	MiniMC::VMT::Pathformula::Memory memory{termbuilder};
	memory.createHeapLayout (descr.getHeap ());
	
	
	auto state =  std::make_shared<MiniMC::CPA::PathFormula::State>(CallStack{std::move(values)},std::move(memory),std::move(term),*context);	

	MiniMC::VMT::Pathformula::PathFormulaEngine engine{MiniMC::VMT::Pathformula::Operations{termbuilder},MiniMC::VMT::Pathformula::Casts{termbuilder}};
	MiniMC::VMT::Pathformula::PathControl control{termbuilder};
	MiniMC::VMT::Pathformula::ValueLookup nlookup {state->getStack().back().values,termbuilder};
	StackControl stackcontrol{state->getStack (),descr.getProgram (),*context};
        decltype(engine)::State newvm {nlookup,state->getMemory (),control,stackcontrol};
	decltype(engine)::ConstState convm {nlookup,state->getMemory (),control,stackcontrol};
	engine.execute(descr.getInit (),newvm,convm);
	
	return state;
      }

      MiniMC::CPA::State_ptr Joiner::doJoin(const State_ptr& lstate, const State_ptr& rstate) {
	auto& termbuilder = context->getBuilder ();
	auto& lstate_ = static_cast<const MiniMC::CPA::PathFormula::State&>(*lstate);
        auto& rstate_ = static_cast<const MiniMC::CPA::PathFormula::State&>(*rstate);
	auto& lpath = lstate_.getPathformula ();
	auto breaker = [&termbuilder,&lpath](const MiniMC::VMT::Pathformula::PathFormulaVMVal& l, const MiniMC::VMT::Pathformula::PathFormulaVMVal& r ) {
	  auto performMerge = [&termbuilder,&lpath]<typename T>(const T& ll, const T&  rr) {
	    if constexpr (std::is_same<T,MiniMC::VMT::Pathformula::AggregateValue> ()) {
		return MiniMC::VMT::Pathformula::PathFormulaVMVal{T{termbuilder.buildTerm (SMTLib::Ops::ITE,{lpath,ll.getTerm (),rr.getTerm ()}),ll.size()}};
	  
	      }
	    else 
	      return MiniMC::VMT::Pathformula::PathFormulaVMVal{T{termbuilder.buildTerm (SMTLib::Ops::ITE,{lpath,ll.getTerm (),rr.getTerm ()})}};
	  };
	  if (l.is<MiniMC::VMT::Pathformula::I8Value> ()) {
	    return performMerge (l.as<MiniMC::VMT::Pathformula::I8Value> (),r.as<MiniMC::VMT::Pathformula::I8Value> ());
	  }
	  else if (l.is<MiniMC::VMT::Pathformula::I16Value> ()) {
	    return performMerge (l.as<MiniMC::VMT::Pathformula::I16Value> (),r.as<MiniMC::VMT::Pathformula::I16Value> ());
	  }

	  else if (l.is<MiniMC::VMT::Pathformula::I32Value> ()) {
	    return performMerge (l.as<MiniMC::VMT::Pathformula::I32Value> (),r.as<MiniMC::VMT::Pathformula::I32Value> ());
	  }

	  else if (l.is<MiniMC::VMT::Pathformula::I64Value> ()) {
	    return performMerge (l.as<MiniMC::VMT::Pathformula::I64Value> (),r.as<MiniMC::VMT::Pathformula::I64Value> ());
	  }

	  else if (l.is<MiniMC::VMT::Pathformula::PointerValue> ()) {
	    return performMerge (l.as<MiniMC::VMT::Pathformula::PointerValue> (),r.as<MiniMC::VMT::Pathformula::PointerValue> ());
	  }

	  else if (l.is<MiniMC::VMT::Pathformula::BoolValue> ()) {
	    return performMerge (l.as<MiniMC::VMT::Pathformula::BoolValue> (),r.as<MiniMC::VMT::Pathformula::BoolValue> ());
	  }

	  else if (l.is<MiniMC::VMT::Pathformula::AggregateValue> ()) {
	    return performMerge (l.as<MiniMC::VMT::Pathformula::AggregateValue> (),r.as<MiniMC::VMT::Pathformula::AggregateValue> ());
	  }
	  
	  return l;

	};

	
	
	//ValueMap values = ValueMap::merge (lstate_.getValues (),rstate_.getValues (),breaker);
	//MiniMC::VMT::Pathformula::Memory memory{termbuilder};
	
	//auto npath = termbuilder.buildTerm (SMTLib::Ops::Or,{lstate_.getPathformula (), rstate_.getPathformula ()});
	//return std::make_shared<MiniMC::CPA::PathFormula::State>(std::move(values),std::move(memory),std::move(npath),*context);
	return nullptr;
      }
      
      MiniMC::CPA::State_ptr Transferer::doTransfer(const State_ptr& s, const MiniMC::Model::Edge_ptr& e, proc_id id) {
        assert(id == 0 && "PathFormula only useful for one process systems");
	auto resstate = s->copy();
	auto& ostate = static_cast<const MiniMC::CPA::PathFormula::State&>(*s);
        auto& nstate = static_cast<MiniMC::CPA::PathFormula::State&>(*resstate);
	MiniMC::VMT::Status status  = MiniMC::VMT::Status::Ok;
	auto& termbuilder = context->getBuilder ();
	MiniMC::VMT::Pathformula::PathControl control{termbuilder};
	
	MiniMC::VMT::Pathformula::ValueLookup nlookup {nstate.getStack().back().values,termbuilder};
	StackControl stackcontrol{nstate.getStack (),e->getProgram (),*context};
        if (e->hasAttribute<MiniMC::Model::AttributeType::Instructions>()) {

	  
	  
	  decltype(engine)::State newvm {nlookup,nstate.getMemory (),control,stackcontrol};
	  decltype(engine)::ConstState convm {nlookup,nstate.getMemory (),control,stackcontrol};
          auto& instr = e->getAttribute<MiniMC::Model::AttributeType::Instructions>();
	  if (!instr.isPhi ()) {
	    status = engine.execute(instr,newvm,convm);
	    
	  }
	  else{
	    MiniMC::VMT::Pathformula::ValueLookup olookup {const_cast<State&> (ostate).getStack().back().values,termbuilder};
	    
	    decltype(engine)::ConstState oldvm {olookup,const_cast<State&> (ostate).getMemory (),control,stackcontrol};
	    status = engine.execute(instr,newvm,oldvm);
	    
	  }
	  
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
