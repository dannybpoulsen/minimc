#ifndef _PATHFORMULA__
#define _PATHFORMULA__

#include "cpa/interface.hpp"
#include "smt/context.hpp"
#include "support/localisation.hpp"
#include "model/checkers/HasInstruction.hpp"


namespace MiniMC {
  namespace CPA {
    namespace PathFormula {
	  template<SMTLib::SMTBackend>
	  struct StateQuery {
		static MiniMC::CPA::State_ptr makeInitialState (const MiniMC::Model::Program&);
		static size_t nbOfProcesses (const State_ptr& );
		static MiniMC::Model::Location_ptr getLocation (const State_ptr&, proc_id);
	  };
      
      struct Transferer {
		static MiniMC::CPA::State_ptr doTransfer (const State_ptr& s, const MiniMC::Model::Edge_ptr&,proc_id);
		
      };
      
      struct Joiner {  
		static MiniMC::CPA::State_ptr doJoin (const State_ptr& l, const State_ptr& r);
	
		static bool covers (const State_ptr& l, const State_ptr& r) {
		  return false;
		}
		
		static void coverCopy (const State_ptr& from, State_ptr& to) {}
      

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
		static void validate (MiniMC::Support::Sequencer<MiniMC::Model::Program>& seq, MiniMC::Support::Messager& mess) {
		  seq.template add<MiniMC::Model::Checkers::HasNoInstruction<
			MiniMC::Model::InstructionCode::Call>
						   ,MiniMC::Support::Messager&,const std::string&> (mess,"This CPA does not support '%1%' instructions.");
		}
      };
      
	  template<SMTLib::SMTBackend b>
      struct CPADef {
		using Query = StateQuery<b>;
		using Transfer = Transferer;
		using Join = Joiner;
		using Storage = MiniMC::CPA::Storer<Join>; 
		using PreValidate = PrevalidateSetup;
	  };

	  using CVC4CPA = CPADef<SMTLib::SMTBackend::CVC4>;
	  
	}
  }
}
#endif
