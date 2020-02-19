#ifndef _CONCRETE_NOMEM__
#define _CONCRETE_NOMEM__

#include "support/localisation.hpp"
#include "model/cfg.hpp"
#include "cpa/interface.hpp"
#include "model/modifications/rremoveretsentry.hpp"
#include "model/modifications/replacenondetuniform.hpp"

namespace MiniMC {
  namespace CPA {
    namespace ConcreteNoMem {
    struct StateQuery {
      static State_ptr makeInitialState (const MiniMC::Model::Program&);
	static size_t nbOfProcesses (const State_ptr& );
	static MiniMC::Model::Location_ptr getLocation (const State_ptr&, proc_id);
      };
      
      struct Transferer {
	static State_ptr doTransfer (const State_ptr& s, const MiniMC::Model::Edge_ptr&,proc_id);
	
      };
      
      struct Joiner {  
	static State_ptr doJoin (const State_ptr& l, const State_ptr& r) {return r;}
      };


      struct ValidateInstructions : public MiniMC::Support::Sink<MiniMC::Model::Program> {
	ValidateInstructions (MiniMC::Support::Messager& ptr) : mess (ptr) {}
	virtual bool run (MiniMC::Model::Program&  prgm) {
	  for (auto& F : prgm.getEntryPoints ()) {
	    for (auto& E : F->getCFG()->getEdges ()) {
	      if (E->hasAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
		for (auto& I : E->getAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
		  if (I.getOpcode () == MiniMC::Model::InstructionCode::NonDet) {
		    MiniMC::Support::Localiser error_mess ("This CPA will replace '%1%' instruction by '%2%"); 
		    mess.warning (error_mess.format (I.getOpcode (), MiniMC::Model::InstructionCode::Uniform));
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
	static void setup (MiniMC::Support::Sequencer<MiniMC::Model::Program>& seq, MiniMC::Support::Messager& mess) {
	  seq.template add<MiniMC::Model::Modifications::RemoveRetEntryPoints> ();
	  seq.template add<MiniMC::Model::Modifications::ReplaceNonDetUniform> ();
	}
	  
	static void validate (MiniMC::Support::Sequencer<MiniMC::Model::Program>& seq, MiniMC::Support::Messager& mess) {
	  seq.template add<ValidateInstructions,MiniMC::Support::Messager&> (mess);
	}
      };
      
	  
      struct CPADef {
		using Query = StateQuery;
		using Transfer = Transferer;
		using Joing = Joiner;
		using Storage = MiniMC::CPA::Storer; 
		using PreValidate = PrevalidateSetup;
	  };
    }
  }
}


#endif
