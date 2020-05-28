#ifndef _LOADER__
#define _LOADER__

#include <memory>

#include "model/cfg.hpp"

namespace MiniMC {
  namespace Loaders {
	/**
	 * The possible model loaders. 
	 **/
	enum class Type {
					 LLVM /**< LLVM Model */
	};

	/** 
	 *
	 * Basic LoaderOptions. 
	 * Different Type of loaders may need different options, but can
	 * use the BaseLoadOptions as part of their load options struct.  
	 *
	 */
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
