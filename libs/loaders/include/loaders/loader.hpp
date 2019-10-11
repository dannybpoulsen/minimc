#ifndef _LOADER__
#define _LOADER__

#include <memory>

#include "model/cfg.hpp"

namespace MiniMC {
  namespace Loaders {
	enum class Type {
					 LLVM
	};

	class Loader {
	public:
	  virtual MiniMC::Model::Program_ptr loadFromFile (const std::string& file) = 0;
	};

	template<Type>
	std::unique_ptr<Loader> makeLoader ();
	
  }
}


#endif
