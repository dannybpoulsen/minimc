#include "loaders/loader.hpp"

#include <vector>

namespace MiniMC {
  namespace Loaders {
    auto& internalGetRegistrars () {
      static std::vector<LoaderRegistrar*> vect;
      return vect;
    }

    const std::vector<LoaderRegistrar*>& getLoaders () {
      return internalGetRegistrars ();
    }
      
    
    LoaderRegistrar::LoaderRegistrar (std::string name) : name(std::move(name)) {
      internalGetRegistrars ().push_back (this);
    }

    
    
  }
}
