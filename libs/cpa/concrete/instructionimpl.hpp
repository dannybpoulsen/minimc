#ifndef _INSTRUCTION_IMPL
#define _INSTRUCTION_IMPL

#include "model/instructions.hpp"

#include "model/variables.hpp"
#include "util/array.hpp"
#include "support/exceptions.hpp"
#include "tacimpl.hpp"

namespace MiniMC {
  namespace CPA {
    namespace Concrete {
	  using VariableLookup = VariableMap<MiniMC::Util::Array>;

	  struct GlobalLocalVariableLookup{
		VariableLookup* global;
		VariableLookup* local;
		auto LookUp (const MiniMC::Model::Variable_ptr& v) const  {
		  if (v->isGlobal ()) {
			return global->at (v);
		  }
		  else
			return local_>at (v);
		}

		void set (const MiniMC::Model::Variable_ptr& v, const MiniMC:Util::Array& arr) {
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
			throw MiniMC::Support::Exception ("No Evaluation of constants available");
		  }
		}
		
	  };
	  
	  struct VMData {
		const GlobalLocalVariableLookup readFrom;
		GlobalLocalVariableLookup writeTo;
		
	  };
	  
	  struct ExecuteInstruction {

		template<MiniMC::Model::InstructionCode opc>
		static void execute (VMData& data,
							 const MiniMC::Model::Instruction& i)  {
		  
		}
	  };

	}
  }
}

#endif
