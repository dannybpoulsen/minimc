#include <memory>
#include <iostream>

#include "minimc/cpa/common.hpp"
#include "minimc/hash/hashing.hpp"
#include "minimc/support/overload.hpp"
#include "minimc/values/symb/symb.hpp"


namespace MiniMC {
  namespace CPA {
    namespace Symbolic {
      using CPA = MiniMC::CPA::Common::CPA<MiniMC::Values::Symb::ValueDefinition>;
      
      
      
    } // namespace Concrete
    template<>
    MiniMC::CPA::TCPA_ptr makeCPA<CPAType::Symbolic> () {
      return std::make_shared<Symbolic::CPA> ();
    }
    
  }   // namespace CPA
} // namespace MiniMC
