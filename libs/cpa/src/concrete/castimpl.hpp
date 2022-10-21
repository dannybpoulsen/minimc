#include "model/instructions.hpp"

#include "model/variables.hpp"
#include "util/array.hpp"
#include "support/exceptions.hpp"
#include "support/localisation.hpp"
#include "support/casts.hpp"


namespace MiniMC {
  namespace CPA {
    namespace Concrete {
	  
	  template<MiniMC::Model::InstructionCode opc,class F,class T>
	  MiniMC::Util::Array castexec (const MiniMC::Util::Array& left) {
		assert(left.getSize() == sizeof(F));
		MiniMC::Util::Array res (sizeof(T));
		auto ll = left.template read<F> ();
		
		if constexpr (opc == MiniMC::Model::InstructionCode::Trunc) {
		  if constexpr (sizeof(F) >= sizeof(T)) {
			res.template set<T> (0,MiniMC::Support::trunc<F,T> (ll));
		  }
		  else {
			throw MiniMC::Support::Exception ("Improper truncation");
		  }
		}
		else if constexpr (opc == MiniMC::Model::InstructionCode::ZExt) {
		  if constexpr (sizeof(F) < sizeof(T)) {
			res.template set<T> (0,MiniMC::Support::zext<F,T> (ll));
		  }
		  else
			throw MiniMC::Support::Exception ("Improperty extenstion");
		}
		else if constexpr (opc == MiniMC::Model::InstructionCode::SExt) {
		  if constexpr (sizeof(F) < sizeof(T)) {
			res.template set<T> (0,MiniMC::Support::sext<F,T> (ll));
		  }
		  else {
			throw MiniMC::Support::Exception ("Improperty extenstion");
		  }
		}

		/*else if constexpr (opc == MiniMC::Model::InstructionCode::PtrToInt) {
		  assert (false && "Not implemented");
		}
		
		else if constexpr (opc == MiniMC::Model::InstructionCode::IntToPtr) {
		  assert (false && "Not implemented");
		  }*/
		
		else if constexpr (opc == MiniMC::Model::InstructionCode::BitCast) {
		  return left;
		}

		else if constexpr (opc == MiniMC::Model::InstructionCode::BoolZExt ||
						   opc == MiniMC::Model::InstructionCode::IntToBool) {
		  if (ll) {
			res.template set<T> (0,1);
		  }
		  else {
			res.template set<T> (0,0);
		  }
		  
		  
		}
		
		else if constexpr (opc == MiniMC::Model::InstructionCode::BoolSExt) {
		  if (ll) {
			res.template set<T> (0,std::numeric_limits<T>::max());
		  }
		  else {
			res.template set<T> (0,0);
		  }
		  
		}

		else {
		  throw NotImplemented<opc> ();
		}
		

		return res;
		
	  }

	  template<MiniMC::Model::InstructionCode opc,class T>
	  MiniMC::Util::Array Stepcastexec (const MiniMC::Util::Array& left) {
		switch (left.getSize ()) {
		case 1:
		  return castexec<opc,MiniMC::uint8_t,T> (left);
		case 2:
		  return castexec<opc,MiniMC::uint16_t,T> (left);
		case 4:
		  return castexec<opc,MiniMC::uint32_t,T> (left);
		case 8:
		  return castexec<opc,MiniMC::uint64_t,T> (left);
		default:
		  throw MiniMC::Support::Exception (MiniMC::Support::Localiser ("Unsupport size %0% for operation '%1%").format (left.getSize(),opc));
			
		}
	  }

	  template<MiniMC::Model::InstructionCode opc>
	  MiniMC::Util::Array Stepcastexec1 (const MiniMC::Util::Array& left,std::size_t tosize) {
		switch (tosize) {
		case 1:
		  return Stepcastexec<opc,MiniMC::uint8_t> (left);
		case 2:
		  return Stepcastexec<opc,MiniMC::uint16_t> (left);
		case 4:
		  return Stepcastexec<opc,MiniMC::uint32_t> (left);
		case 8:
		  return Stepcastexec<opc,MiniMC::uint64_t> (left);
		default:
		  throw MiniMC::Support::Exception (MiniMC::Support::Localiser ("Unsupport size %0% for operation '%1%").format (left.getSize(),opc));
			
		}
	  }

	  

	  
	  
	}
  }
}
