#include "minimc/model/types.hpp"
#include "minimc/model/variables.hpp"
#include "minimc/support/random.hpp"

#include <limits>
#include <generator>
#include <iostream>
namespace MiniMC {
  namespace Model {
    struct StochasticGenerator {
      StochasticGenerator (std::size_t fuzzycount) : fuzzycount(fuzzycount) {}
      template<class R,class T>
      std::generator<MiniMC::Model::Value_ptr> generate (T min =  std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max()) const {
	MiniMC::Support::Random rand;
	for(std::size_t i = 0; i < fuzzycount; i++) 
	  co_yield R::make(rand.uniform_int(min,max));
      }
      
      std::generator<MiniMC::Model::Value_ptr> generate (const MiniMC::Model::Type& t) const {
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

      std::size_t fuzzycount;
    };
    
    struct NonDetGenerator {
      template<typename T>
      std::generator<T> generate (T min = std::numeric_limits<T>::min(), T max  = std::numeric_limits<T>::max())  const {
	for (T i = min; i < max; ++i)
	  co_yield i;
	co_yield std::numeric_limits<T>::max();
      }
      
      template<class R,class T>
      std::generator<MiniMC::Model::Value_ptr> generate (T min =  std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max()) const {
	for (auto t : generate (min,max))
	  co_yield R::make(t);
      }
      
      
      std::generator<MiniMC::Model::Value_ptr> generate (const MiniMC::Model::Type& t) const {
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
