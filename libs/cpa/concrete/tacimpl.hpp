#include "model/instructions.hpp"

#include "model/variables.hpp"
#include "util/array.hpp"
#include "support/exceptions.hpp"
#include "support/div.hpp"
#include "support/rightshifts.hpp"


namespace MiniMC {
  namespace CPA {
    namespace Concrete {
	  
	  template<MiniMC::Model::InstructionCode opc,class T>
	  MiniMC::Util::Array tacexec (const MiniMC::Util::Array& left, const MiniMC::Util::Array& right) {
		MiniMC::Util::Array res (sizeof(T));
		auto ll = left.template read<T> ();
		auto rr = right.template read<T> ();

		if constexpr ( opc == MiniMC::Model::InstructionCode::Add) {
		  res.template set<T> (0, ll+rr);
		}

		else if constexpr (opc == MiniMC::Model::InstructionCode::Sub) {
		  res.template set<T> (0, ll - rr);
		}

		else if constexpr ( opc == MiniMC::Model::InstructionCode::Mul) {
		  res.template set<T> (0, ll*rr);
		}

		else if constexpr ( opc == MiniMC::Model::InstructionCode::UDiv) {
		  res.template set<T> (0, MiniMC::Support::div(ll,rr));
			
		}

		else if constexpr ( opc == MiniMC::Model::InstructionCode::SDiv) {
		  res.template set<T> (0, MiniMC::Support::idiv(ll,rr));
		}

		else if constexpr ( opc == MiniMC::Model::InstructionCode::Shl) {
		  res.template set<T> (0, ll <<rr );
		}

		else if constexpr ( opc == MiniMC::Model::InstructionCode::LShr) {
		  res.template set<T> (0, MiniMC::Support::lshr(ll,rr) );
		}

		else if constexpr ( opc == MiniMC::Model::InstructionCode::AShr) {
		  res.template set<T> (0, MiniMC::Support::ashr(ll,rr) );
		}

		else if constexpr ( opc == MiniMC::Model::InstructionCode::And) {
		  res.template set<T> (0, ll & rr );
		}

		else if constexpr ( opc == MiniMC::Model::InstructionCode::Or) {
		  res.template set<T> (0, ll | rr );
		}

		else if constexpr ( opc == MiniMC::Model::InstructionCode::Xor) {
		  res.template set<T>(0, ll ^ rr );
		}
		
		return res;
	  }
		

	}
  }
}
