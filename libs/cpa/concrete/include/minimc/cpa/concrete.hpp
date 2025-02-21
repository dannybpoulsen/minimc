
#ifndef _CPA_CONCRETE_INTERFACE__
#define _CPA_CONCRETE_INTERFACE__

#include <memory>
#include <ostream>

#include "minimc/values/concrete/concrete.hpp"
#include "minimc/cpa/common.hpp"
#include "minimc/cpa/interface.hpp"
#include "minimc/cpa/state.hpp"
#include "minimc/hash/hashing.hpp"
#include "minimc/model/cfg.hpp"
#include "minimc/support/feedback.hpp"
#include <unordered_map>



namespace MiniMC {
  namespace CPA {
    namespace Concrete {
      
      using CPA = MiniMC::CPA::Common::CPA<MiniMC::VMT::Concrete::ValueDefinition>;
      
    } // namespace Concrete
  }   // namespace CPA
} // namespace MiniMC

#endif
