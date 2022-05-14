#ifndef _LOADER__
#define _LOADER__

#include <memory>

#include "model/cfg.hpp"
#include "model/controller.hpp"

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
	  std::size_t stacksize{100};
	};

	template<Type>
	struct OptionsLoad {
	  using Opt = BaseLoadOptions;
	};
    

    struct LoadResult {
      MiniMC::Model::Program_ptr program;
      MiniMC::Model::entry_creator entrycreator;
    };
      
    template<Type t,class Arg = typename OptionsLoad<t>::Opt>
    LoadResult loadFromFile (const std::string& file, Arg loadOptions);
    
    template<Type t,class Arg = typename OptionsLoad<t>::Opt>
    LoadResult  loadFromString (const std::string& str, Arg loadOptions);
    
    
  }
}


#endif
