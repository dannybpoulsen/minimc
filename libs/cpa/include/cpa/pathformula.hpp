#ifndef _PATHFORMULA__
#define _PATHFORMULA__

#include "cpa/interface.hpp"
#include "support/localisation.hpp"
#include "model/checkers/HasInstruction.hpp"


namespace MiniMC {
  namespace CPA {
    namespace PathFormula {
      struct StateQuery : public MiniMC::CPA::StateQuery{
	MiniMC::CPA::State_ptr makeInitialState (const MiniMC::Model::Program&);
      };
      
      struct Transferer : public MiniMC::CPA::Transferer {
	MiniMC::CPA::State_ptr doTransfer (const State_ptr& s, const MiniMC::Model::Edge_ptr&,proc_id);
		
      };
      
      struct Joiner : public MiniMC::CPA::Joiner {  
	MiniMC::CPA::State_ptr doJoin (const State_ptr& l, const State_ptr& r);
	
	bool covers (const State_ptr& l, const State_ptr& r) {
	  return false;
	}
		
      };
	  

      struct ValidateInstructions : public MiniMC::Support::Sink<MiniMC::Model::Program> {
	ValidateInstructions (MiniMC::Support::Messager& ptr) : mess (ptr) {}
	virtual bool run (MiniMC::Model::Program&  prgm) {
	  for (auto& F : prgm.getEntryPoints ()) {
	    for (auto& E : F->getCFG()->getEdges ()) {
	      if (E->hasAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
		for (auto& I : E->getAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
		  if (I.getOpcode () == MiniMC::Model::InstructionCode::Call) {
		    MiniMC::Support::Localiser error_mess ("This CPA does not support '%1%' instructions."); 
		    mess.error (error_mess.format (I.getOpcode ()));
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
	  
      struct PrevalidateSetup {
	bool validate (const MiniMC::Model::Program& prgm, MiniMC::Support::Messager& mess) {
	  return MiniMC::Model::Checkers::HasNoInstruction<MiniMC::Model::InstructionCode::Call> (mess,"This CPA does not support '%1%' instructions.").run (prgm);
	}
      };
      
      using CPA = CPADef<
	StateQuery,
	Transferer,
	Joiner,
	MiniMC::CPA::Storer,
	MiniMC::CPA::PrevalidateSetup>;
      	  
    }
  }
}
#endif
