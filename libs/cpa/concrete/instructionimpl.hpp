#ifndef _INSTRUCTION_IMPL
#define _INSTRUCTION_IMPL

#include "model/instructions.hpp"

#include "model/variables.hpp"
#include "util/array.hpp"
#include "support/exceptions.hpp"
#include "tacimpl.hpp"
#include "cmpimpl.hpp"
#include "castimpl.hpp"



namespace MiniMC {
  namespace CPA {
    namespace Concrete {
	  template<MiniMC::Model::InstructionCode c>
      class NotImplemented : public MiniMC::Support::Exception {
      public:
		NotImplemented () : MiniMC::Support::Exception (MiniMC::Support::Localiser{"Instruction '%1%' not implemented for this CPA"}.format (c)) {}
      };
	  
	  using VariableLookup = MiniMC::Model::VariableMap<MiniMC::Util::Array>;
	  
	  struct GlobalLocalVariableLookup{
		VariableLookup* global;
		VariableLookup* local;
		auto LookUp (const MiniMC::Model::Variable_ptr& v) const  {
		  if (v->isGlobal ()) {
			return global->at (v);
		  }
		  else
			return local->at (v);
		}

		void set (const MiniMC::Model::Variable_ptr& v, const MiniMC::Util::Array& arr) {
		  if (v->isGlobal ()) {
			(*global)[v] = arr;
		  }
		  else
			(*local)[v] = arr;
		}

		const MiniMC::Util::Array evaluate (const MiniMC::Model::Value_ptr& v) const {
		  if (v->isVariable ()) {
			auto var =  std::static_pointer_cast<MiniMC::Model::Variable> (v);
			return LookUp (var);
		  }
		  else {
			auto constant = std::static_pointer_cast<MiniMC::Model::Constant> (v);
			if (!constant->isNonCompileConstant ()) {
			  MiniMC::Util::Array arr(constant->getSize ());
			  arr.set_block (0,constant->getSize (),constant->getData ());
			  return arr;
			}

			throw MiniMC::Support::Exception ("No Evaluation of constants available");
		  }
		}
		
	  };
	  
	  struct VMData {
		GlobalLocalVariableLookup readFrom;
		GlobalLocalVariableLookup writeTo;
		void finalise () {}
	  };
	  
	  struct ExecuteInstruction {

		template<MiniMC::Model::InstructionCode opc>
		static void execute (VMData& data,
							 const MiniMC::Model::Instruction& i)  {
		  MiniMC::Model::InstHelper<opc> helper (i);
		  if constexpr (MiniMC::Model::InstructionData<opc>::isTAC) {
			auto& res = helper.getResult ();
			auto& left = helper.getLeftOp ();
			auto& right = helper.getRightOp ();

			auto lval = data.readFrom.evaluate (left);
			auto rval = data.readFrom.evaluate (right);
			data.writeTo .set (std::static_pointer_cast<MiniMC::Model::Variable> (res), Steptacexec<opc> (lval,rval));
			
		  }

		  else if constexpr (MiniMC::Model::InstructionData<opc>::isComparison) {
			std::cerr << std::endl << i << std::endl;
			auto& res = helper.getResult ();
			 auto& left = helper.getLeftOp ();
			 auto& right = helper.getRightOp ();
			 
			 auto lval = data.readFrom.evaluate (left);
			 auto rval = data.readFrom.evaluate (right);
			 std::cerr << lval.getSize() << " " << rval.getSize () << std::endl;
			 data.writeTo .set (std::static_pointer_cast<MiniMC::Model::Variable> (res), Stepcmpexec<opc> (lval,rval));
			 
		  }

		  else if constexpr (MiniMC::Model::InstructionData<opc>::isPredicate) {
			 auto& left = helper.getLeftOp ();
			 auto& right = helper.getRightOp ();
			 
			 auto lval = data.readFrom.evaluate (left);
			 auto rval = data.readFrom.evaluate (right);
			 Steppredexec<opc> (lval,rval);
			 
		  }
		  
		  else if constexpr (MiniMC::Model::InstructionData<opc>::isCast) {
			auto& res = helper.getResult ();
			auto& left = helper.getCastee ();
			std::cerr << std::endl << i << std::endl;
			auto lval = data.readFrom.evaluate (left);
			data.writeTo .set (std::static_pointer_cast<MiniMC::Model::Variable> (res), Stepcastexec1<opc> (lval,res->getType()->getSize()));
			
		  }

		  else if constexpr (opc == MiniMC::Model::InstructionCode::Assign) {
			auto& res = helper.getResult ();
			auto& left = helper.getValue ();
			auto lval = data.readFrom.evaluate (left);
			data.writeTo .set (std::static_pointer_cast<MiniMC::Model::Variable> (res), lval);
		  }

		  else if constexpr (opc == MiniMC::Model::InstructionCode::Skip) {
			
		  }
		   
		  else if constexpr (opc == MiniMC::Model::InstructionCode::Assume) {
			auto& val = helper.getAssert ();
			auto lval = data.readFrom.evaluate (val);
			if (!lval.template read<MiniMC::uint8_t> ())
			  throw MiniMC::Support::AssumeViolated ();
		  }

		  else if constexpr (opc == MiniMC::Model::InstructionCode::Assert) {
			auto& val = helper.getAssert ();
			auto lval = data.readFrom.evaluate (val);
			if (!lval.template read<MiniMC::uint8_t> ())
			  throw MiniMC::Support::AssertViolated ();
		  }

		  else if constexpr (opc == MiniMC::Model::InstructionCode::NegAssume) {
			auto& val = helper.getAssert ();
			auto lval = data.readFrom.evaluate (val);
			if (lval.template read<MiniMC::uint8_t> ())
			  throw MiniMC::Support::AssumeViolated ();
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
