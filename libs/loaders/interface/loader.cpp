#include "loaders/loader.hpp"

#include <vector>
#include <unordered_map>

namespace MiniMC {
  namespace Loaders {
    auto& internalGetRegistrars () {
      static std::vector<LoaderRegistrar*> vect;
      return vect;
    }

    const std::vector<LoaderRegistrar*>& getLoaders () {
      return internalGetRegistrars ();
    }

    LoaderRegistrar* findLoader (const std::string& s) {
      for (auto& l : getLoaders ()) {
	if (l->getName () == s) {
	  return l;
	}
      }
      return nullptr;
    }
    
    
    
    
    LoaderRegistrar::LoaderRegistrar (std::string name,std::initializer_list<LoaderOption> opts) : name(std::move(name)),options(opts) {
      internalGetRegistrars ().push_back (this);
    }

    
    
  }
}
