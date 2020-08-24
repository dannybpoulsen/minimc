#ifndef _CONCRETE_NOMEM__
#define _CONCRETE_NOMEM__

#include "support/localisation.hpp"
#include "model/cfg.hpp"
#include "cpa/interface.hpp"
#include "model/checkers/HasInstruction.hpp"
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
		  assert(l);
		  assert(r);
		  return std::hash<MiniMC::CPA::State>{} (*l) == std::hash<MiniMC::CPA::State>{} (*r);
		}
		
		static void coverCopy (const State_ptr& from, State_ptr& to) {
		}
      };
      
      struct PrevalidateSetup {
		static void validate (MiniMC::Support::Sequencer<MiniMC::Model::Program>& seq, MiniMC::Support::Messager& mess) {
		  seq.template add<MiniMC::Model::Checkers::HasNoInstruction<
			MiniMC::Model::InstructionCode::NonDet>
						   ,MiniMC::Support::Messager&,const std::string&> (mess,"This CPA does not support '%1%' instructions.");
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
