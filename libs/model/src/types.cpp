#include <limits>
#include <unordered_map>

#include "minimc/model/types.hpp"
#include "minimc/model/variables.hpp"


#include "minimc/support/exceptions.hpp"


namespace MiniMC {
  namespace Model {


    
    

    std::ostream& operator<< (std::ostream& os, TypeID id){
      switch (id) {
      case TypeID::Void: return os << "Void";
      case TypeID::Bool: return os << "Bool";
      case TypeID::I8: return os << "I8";
      case TypeID::I16: return os << "I16";
      case TypeID::I32: return os << "I32";
      case TypeID::I64: return os << "I64";
      case TypeID::Pointer: return os << "Pointer";
      case TypeID::Pointer32: return os << "Pointer32";
      case TypeID::Aggregate: return os << "Aggregate";
      case TypeID::Memory: return os << "Memory";
      default:
	std::unreachable();
      }
    }
    
    
  } // namespace Model
} // namespace MiniMC
