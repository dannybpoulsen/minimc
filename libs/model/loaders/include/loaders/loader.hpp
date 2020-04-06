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
	  virtual MiniMC::Model::Program_ptr loadFromFile (const std::string& file, MiniMC::Model::TypeFactory_ptr&, MiniMC::Model::ConstantFactory_ptr&) = 0;
	};
	
	template<Type>
	std::unique_ptr<Loader> makeLoader ();

	struct BaseLoadOptions {
	  MiniMC::Model::TypeFactory_ptr tfactory;
	  MiniMC::Model::ConstantFactory_ptr cfactory;
	};

	template<Type>
	struct OptionsLoad {
	  using Opt = BaseLoadOptions;
	};
	
	template<Type t,class Arg = typename OptionsLoad<t>::Opt>
	MiniMC::Model::Program_ptr loadFromFile (const std::string& file, Arg loadOptions);

	template<Type t,class Arg = typename OptionsLoad<t>::Opt>
	MiniMC::Model::Program_ptr loadFromString (const std::string& str, Arg loadOptions);	
  }
}


#endif
