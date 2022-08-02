#ifndef _LOADER__
#define _LOADER__


#include "support/exceptions.hpp"
#include "model/cfg.hpp"
#include "model/controller.hpp"

#include <memory>
#include <string>
#include <variant>

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
	  std::size_t stacksize{100};
	};

	template<Type>
	struct OptionsLoad {
	  using Opt = BaseLoadOptions;
	};
    
    class LoadError : public MiniMC::Support::ConfigurationException {
    public:
      LoadError () : ConfigurationException("Failed to load program") {}
    };
    
    struct LoadResult {
      MiniMC::Model::Program_ptr program;
      MiniMC::Model::entry_creator entrycreator;
    };

    template<class T>
    struct TOption {
      std::string name;
      std::string description;
      T value;
    };

    using IntOption = TOption<std::size_t>;
    using StringOption = TOption<std::string>;

    using LoaderOption =  std::variant<IntOption,
				       StringOption>;
    
    struct Loader {
      Loader (MiniMC::Model::TypeFactory_ptr& tfac,
	      Model::ConstantFactory_ptr& cfac) : tfactory(tfac),
						 cfactory(cfac) {}
      
      virtual ~Loader () {}
      virtual LoadResult loadFromFile (const std::string& file, BaseLoadOptions&& ) = 0;
      virtual LoadResult  loadFromString (const std::string& str, BaseLoadOptions&&) = 0;
    protected:
      MiniMC::Model::TypeFactory_ptr tfactory;
      MiniMC::Model::ConstantFactory_ptr cfactory;
	      
    };

    using Loader_ptr = std::unique_ptr<Loader>; 

    struct LoaderRegistrar {
      LoaderRegistrar (std::string name);
      virtual Loader_ptr makeLoader (MiniMC::Model::TypeFactory_ptr& tfac, Model::ConstantFactory_ptr cfac) = 0;
      auto& getName () const { return name;}
    private:
      std::string name;
    };
    
    
    const std::vector<LoaderRegistrar*>& getLoaders ();
      
    
    template<Type>
    Loader_ptr makeLoader (MiniMC::Model::TypeFactory_ptr& tfac, Model::ConstantFactory_ptr cfac);
     
  }
}


#endif
