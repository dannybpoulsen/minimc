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
	static State_ptr doJoin (const State_ptr& l, const State_ptr& r) {return nullptr;}
		
	static bool covers (const State_ptr& l, const State_ptr& r) {
	  return std::hash<MiniMC::CPA::State>{} (*l) == std::hash<MiniMC::CPA::State>{} (*r);
	}

	static void coverCopy (const State_ptr& from, State_ptr& to) {
	}
      };
		
		
      struct ValidateInstructions : public MiniMC::Support::Sink<MiniMC::Model::Program> {
	ValidateInstructions (MiniMC::Support::Messager& ptr) : mess (ptr) {}
	virtual bool run (MiniMC::Model::Program&  prgm) {
	  return true;
	}
      private:
	MiniMC::Support::Messager& mess;
      };
      
      struct PrevalidateSetup {
	static void setup (MiniMC::Support::Sequencer<MiniMC::Model::Program>& seq, MiniMC::Support::Messager& mess) {
	  seq.template add<MiniMC::Model::Modifications::RemoveRetEntryPoints> ();
	}
	  
	static void validate (MiniMC::Support::Sequencer<MiniMC::Model::Program>& seq, MiniMC::Support::Messager& mess) {
	  seq.template add<ValidateInstructions,MiniMC::Support::Messager&> (mess);
	}
      };
      
	  
      struct CPADef {
	using Query = StateQuery;
	using Transfer = Transferer;
	using Join = Joiner;
	using Storage = MiniMC::CPA::Storer<Join>; 
	using PreValidate = PrevalidateSetup;
      };
    }
  }
}


#endif
