#include <memory>
#include <iostream>

#include "minimc/cpa/concrete.hpp"
#include "minimc/cpa/common.hpp"
#include "minimc/hash/hashing.hpp"
#include "minimc/support/overload.hpp"
#include "minimc/values/concrete/concrete.hpp"
#include "minimc/values/concrete/value.hpp"
#include "minimc/values/concrete/operations.hpp"


namespace MiniMC {
  namespace CPA {
    namespace Concrete {
      using CPA = MiniMC::CPA::Common::CPA<MiniMC::VMT::Concrete::ValueDefinition>;
      
      
      
    } // namespace Concrete
    template<>
    MiniMC::CPA::TCPA_ptr makeCPA<CPAType::Concrete> () {
      return std::make_shared<Concrete::CPA> ();
    }
    
  }   // namespace CPA
} // namespace MiniMC
