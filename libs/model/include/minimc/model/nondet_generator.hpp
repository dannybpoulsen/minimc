#include "minimc/model/types.hpp"
#include "minimc/model/variables.hpp"

#include <limits>
#include <generator>
namespace MiniMC {
  namespace Model {
    struct NonDetGenerator {
      template<typename T>
      std::generator<T> generate (T min = std::numeric_limits<T>::min(), T max  = std::numeric_limits<T>::max()) {
	for (T i = min; i < max; ++i)
	  co_yield i;
	co_yield std::numeric_limits<T>::max();
      }
      
      template<class R,class T>
      std::generator<MiniMC::Model::Value_ptr> generate (T min =  std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max()) {
	for (auto t : generate (min,max))
	  co_yield R::make(t);
      }
      
      
      std::generator<MiniMC::Model::Value_ptr> generate (const MiniMC::Model::Type& t) {
	switch (t.getTypeID ()) {
	case MiniMC::Model::TypeID::I8: 
	  return generate<MiniMC::Model::I8Integer,MiniMC::BV8> (); 
	case MiniMC::Model::TypeID::I16: 
	  return generate<MiniMC::Model::I16Integer,MiniMC::BV16> ();
	case MiniMC::Model::TypeID::I32: 
	  return generate<MiniMC::Model::I32Integer,MiniMC::BV32> ();
	case MiniMC::Model::TypeID::I64: 
	  return generate<MiniMC::Model::I64Integer,MiniMC::BV64> ();
	default:
	  throw MiniMC::Support::Exception ("Can't generate non-det-range for this type");
	}
      }
      
    };
  }
}
