#include <gsl/pointers>
#include "cpa/pathformula.hpp"
#include "util/ssamap.hpp"
#include "util/smtconstruction.hpp"
#include "util/vm.hpp"
#include "state.hpp"
#include "smt/context.hpp"
#include "instructionimpl.hpp"

namespace MiniMC {
  namespace CPA {
    namespace PathFormula {
	  

	  template<SMTLib::SMTBackend b>
	  State_ptr StateQuery<b>::makeInitialState (const MiniMC::Model::Program& prgm) {
		MiniMC::Util::SSAMap map;
		auto context = SMTLib::makeContext<b> ();
		for (auto& entry : prgm.getEntryPoints ()) {
		  auto stackDescr = entry->getVariableStackDescr ();
		  for (auto& v : stackDescr->getVariables ()) {
			map.initialiseValue (v.get(),MiniMC::Util::buildSMTValue (context->getBuilder (),v));
		  }
		  
		}
		return std::make_shared<MiniMC::CPA::PathFormula::State> (map,context,context->getBuilder().makeBoolConst (true));
	  }

	  MiniMC::CPA::State_ptr Joiner::doJoin (const State_ptr& l, const State_ptr& r) {
		/*auto& left = static_cast<const MiniMC::CPA::PathFormula::State&> (*l);
		auto& right = static_cast<const MiniMC::CPA::PathFormula::State&> (*r);
		if (left.getContext() != right.getContext ()) {
		  return nullptr;
		}
		auto& builder = left.getContext ()->getBuilder ();
		
		auto boolsort = builder.makeSort (SMTLib::SortKind::Bool,{});
		auto mergeVar = builder.makeVar (boolsort,"");
	    auto mergeOp = [&](const SMTLib::Term_ptr& lterm, const SMTLib::Term_ptr& rterm) {
						 return builder.buildTerm (SMTLib::Ops::ITE,{mergeVar,lterm,rterm}); 
					   };
		MiniMC::Util::SSAMap nmap = MiniMC::Util::SSAMap::merge (left.getSSAMap (),right.getSSAMap(), mergeOp);
		
		return std::make_shared<MiniMC::CPA::PathFormula::State> (nmap,left.getContext(),mergeOp (left.getPathFormula(),right.getPathFormula ())); 
		*/
		return nullptr;
	  }
	  
	  MiniMC::CPA::State_ptr Transferer::doTransfer (const State_ptr& s, const MiniMC::Model::Edge_ptr& e,proc_id id) {
		assert (id == 0 && "PathFormula only useful for one process systems");
		State_ptr resstate = s->copy ();
		auto& oState = static_cast<const MiniMC::CPA::PathFormula::State&> (*s);
		auto& nState = static_cast<MiniMC::CPA::PathFormula::State&> (*resstate);


		VMData data {.oldSSAMap = &oState.getSSAMap (),
					 .newSSAMap = &nState.getSSAMap (),
					 .smtbuilder = &nState.getContext()->getBuilder(),
					 .path = nState.getPathFormula ()
		};
		
		if (e->hasAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
		  
		  auto& instr = e->getAttribute<MiniMC::Model::AttributeType::Instructions> ();
		  try {
		
			if (!instr.isPhi) {
			  data.oldSSAMap = data.newSSAMap;
			}
			auto it = instr.begin();
			auto end = instr.end ();
			MiniMC::Util::runVM<decltype(it),VMData,ExecuteInstruction> (it,end,data);
			nState.getPathFormula () = data.path;

			
		  }
		  catch  (MiniMC::Support::AssumeViolated) {
			return nullptr;
		  }
		
		}
		
		return resstate;
	  }
	
	  template class StateQuery<SMTLib::SMTBackend::CVC4>;
	  
	}
  }
}
