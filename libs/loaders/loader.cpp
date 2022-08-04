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
      
    
    LoaderRegistrar::LoaderRegistrar (std::string name,std::initializer_list<LoaderOption> opts) : name(std::move(name)),options(opts) {
      internalGetRegistrars ().push_back (this);
    }

    
    
  }
}
