
#ifndef _CPA_CONCRETE_INTERFACE__
#define _CPA_CONCRETE_INTERFACE__

#include <ostream>
#include <memory>
#include "model/checkers/HasInstruction.hpp"

#include <unordered_map>
#include "cpa/state.hpp"
#include "cpa/interface.hpp"
#include "support/sequencer.hpp"
#include "support/feedback.hpp"
#include "model/cfg.hpp"
#include "hash/hashing.hpp"

namespace MiniMC {
  namespace CPA {
    namespace Concrete {
      struct StateQuery : public MiniMC::CPA::StateQuery{
		
	MiniMC::CPA::State_ptr makeInitialState (const MiniMC::Model::Program&);
		
	size_t nbOfProcesses (const MiniMC::CPA::State_ptr& ) {return 0;}

	MiniMC::Model::Location_ptr getLocation (const MiniMC::CPA::State_ptr&, proc_id id) {return nullptr;}
      };
	  
      struct Transferer : public MiniMC::CPA::Transferer{
	
	MiniMC::CPA::State_ptr doTransfer (const MiniMC::CPA::State_ptr& s, const MiniMC::Model::Edge_ptr& e,proc_id id);
      };

	
      struct Joiner : public MiniMC::CPA::Joiner {  
	MiniMC::CPA::State_ptr doJoin (const MiniMC::CPA::State_ptr& l, const MiniMC::CPA::State_ptr& r) {return nullptr;}
	bool covers (const MiniMC::CPA::State_ptr& l, const MiniMC::CPA::State_ptr& r) {
	  return l->hash () == r->hash ();
	}
		
		
      };
	  

      struct PrevalidateSetup : public MiniMC::CPA::PrevalidateSetup  {
	bool validate (const MiniMC::Model::Program& prgm, MiniMC::Support::Messager& mess) {
	  return MiniMC::Model::Checkers::HasNoInstruction<MiniMC::Model::InstructionCode::Call>{mess,"This CPA does not support '%1%' instructions."}.run (prgm);
	}
      };
	
      using CPA = CPADef<
	StateQuery,
	Transferer,
	Joiner,
	MiniMC::CPA::Storer,
	MiniMC::CPA::PrevalidateSetup>;
      
      /*struct CPADef {
	using Query = StateQuery; 
	using Transfer = Transferer;
	using Join = Joiner; 
	using Storage = MiniMC::CPA::Storer<Join>; 
	using PreValidate = PrevalidateSetup;  
      };*/
    }
  }
}

#endif

