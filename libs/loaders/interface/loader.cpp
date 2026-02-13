#include "minimc/loaders/loader.hpp"

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
    
    
    
    
    LoaderRegistrar::LoaderRegistrar (std::string name) : name(std::move(name)) {
      internalGetRegistrars ().push_back (this);
    }
    
    GenericLoader::GenericLoader () {
      for (auto t : getLoaders()) {
	loaders.push_back (t->makeLoader ());
      }
    }
    
    LoadResult GenericLoader::loadFromFile(const std::string& file, MiniMC::Support::Messager& m) {
      for (auto& l : loaders) {
	auto exp = l->loadFromFile(file,m);
	if (exp.has_value())
	  return exp;
      }
      return std::unexpected {Error::LoadFailed};
    }
    
    LoadResult GenericLoader::loadFromString(const std::string& str, MiniMC::Support::Messager& m) {
      for (auto& l : loaders) {
	auto exp = l->loadFromString(str,m);
	if (exp.has_value())
	  return exp;
      }
      return std::unexpected {Error::LoadFailed};
    }
    
    
    
  }
}
