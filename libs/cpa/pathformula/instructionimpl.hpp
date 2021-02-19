#ifndef _PATHIMPL__
#define _PATHIMPL__

#include "util/ssamap.hpp"
#include "util/smtconstruction.hpp"


namespace MiniMC {
  namespace CPA {
    namespace PathFormula {
	  
      struct VMData {
		const MiniMC::Util::SSAMap* oldSSAMap;
		MiniMC::Util::SSAMap* newSSAMap;
		SMTLib::TermBuilder* smtbuilder;
		SMTLib::Term_ptr path;
		void finalise() {}
      };
	  
      template<MiniMC::Model::InstructionCode c>
      class NotImplemented : public MiniMC::Support::Exception {
      public:
		NotImplemented () : MiniMC::Support::Exception (MiniMC::Support::Localiser{"Instruction '%1%' not implemented for this CPA"}.format (c)) {}
      };

	  
      struct ExecuteInstruction {
		template<MiniMC::Model::InstructionCode opc>
		static void execute (VMData& data,
							 const MiniMC::Model::Instruction& i)  {
	
		  if constexpr (MiniMC::Model::InstructionData<opc>::isTAC ||
						MiniMC::Model::InstructionData<opc>::isComparison) {
			MiniMC::Model::InstHelper<opc> helper (i);
			auto& res = helper.getResult ();
			auto& left = helper.getLeftOp ();
			auto& right = helper.getRightOp ();
			SMTLib::Ops smtop = MiniMC::Util::convertToSMTOp<opc> ();
			auto leftTerm = MiniMC::Util::buildSMTTerm (*data.oldSSAMap,*data.smtbuilder,left);//data.oldSSAMap->lookup (left.get());
			auto rightTerm = MiniMC::Util::buildSMTTerm (*data.oldSSAMap,*data.smtbuilder,right);
			data.newSSAMap->updateValue (res.get(),data.smtbuilder->buildTerm (smtop,{leftTerm,rightTerm}));
		  }
		  else if constexpr (MiniMC::Model::InstructionData<opc>::isPredicate ) {
			MiniMC::Model::InstHelper<opc> helper (i);
			auto& left = helper.getLeftOp ();
			auto& right = helper.getRightOp ();
			SMTLib::Ops smtop = MiniMC::Util::convertToSMTOp<opc> ();
			auto leftTerm = MiniMC::Util::buildSMTTerm (*data.oldSSAMap,*data.smtbuilder,left);//data.oldSSAMap->lookup (left.get());
			auto rightTerm = MiniMC::Util::buildSMTTerm (*data.oldSSAMap,*data.smtbuilder,right);
			auto conjunct = data.smtbuilder->buildTerm (smtop,{leftTerm,rightTerm});
			data.path = data.smtbuilder->buildTerm(SMTLib::Ops::And,{data.path,conjunct});
	  
		  }
		  else if constexpr (opc == MiniMC::Model::InstructionCode::Assign) {
			MiniMC::Model::InstHelper<opc> helper (i);
			auto& res = helper.getResult ();
			auto& value = helper.getValue ();
			auto valTerm = MiniMC::Util::buildSMTTerm (*data.oldSSAMap,*data.smtbuilder,value);
			data.newSSAMap->updateValue (res.get(),valTerm);
		  }
		  else if constexpr (opc == MiniMC::Model::InstructionCode::Assume) {
			MiniMC::Model::InstHelper<opc> helper (i);
			auto assert = helper.getAssert ();
			auto assertTerm = MiniMC::Util::buildSMTTerm (*data.oldSSAMap,*data.smtbuilder,assert);
			data.path = data.smtbuilder->buildTerm(SMTLib::Ops::And,{data.path,assertTerm});
		  }
		  else if constexpr (opc == MiniMC::Model::InstructionCode::NegAssume) {
			MiniMC::Model::InstHelper<opc> helper (i);
			auto assert = helper.getAssert ();
			auto assertTerm = MiniMC::Util::buildSMTTerm (*data.oldSSAMap,*data.smtbuilder,assert);
			auto notted = data.smtbuilder->buildTerm(SMTLib::Ops::Not,{assertTerm});
			data.path = data.smtbuilder->buildTerm(SMTLib::Ops::And,{data.path,notted});
		    
		  }

		  else if constexpr (opc == MiniMC::Model::InstructionCode::NonDet) {
			MiniMC::Model::InstHelper<opc> helper (i);
			auto& res = helper.getResult ();
			auto valTerm = MiniMC::Util::buildSMTValue (*data.smtbuilder,res);
			
			data.newSSAMap->updateValue (res.get(),valTerm);
			
		  }
		  
		  else if constexpr (opc == MiniMC::Model::InstructionCode::Skip) {
			//Do nothing
		  }
		  
		  else {
			throw NotImplemented<opc> ();
		  }
		}
      };
      
      
      
    }
  }
}


#endif
