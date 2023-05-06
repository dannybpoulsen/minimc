#ifndef _LOADER__
#define _LOADER__

#include "model/cfg.hpp"
#include "model/controller.hpp"
#include "support/exceptions.hpp"
#include "support/localisation.hpp"


#include <memory>
#include <string>
#include <vector>
#include <variant>
#include <initializer_list>

namespace MiniMC {
  namespace Loaders {

    class LoadError : public MiniMC::Support::ConfigurationException {
    public:
      LoadError(const std::string& mess = "Failed to load program" ) : ConfigurationException(mess) {}
    };

    class VariadicFunctionsNotSupported : public LoadError {
    public:
      VariadicFunctionsNotSupported(const std::string& name) : LoadError(
									 MiniMC::Support::Localiser("Function '%1%' is varidic. This is not supported by MiniMC").format (name)) 
      {}
    };

    struct LoadResult {
      MiniMC::Model::Program_ptr program;
    };

    template <class T>
    struct TOption {
      TOption (const std::string name,
	       const std::string descr,
	       T value) : name(name), description(descr),value(value) {}
      std::string name;
      std::string description;
      T value;
    };
    
    using IntOption = TOption<std::size_t>;
    using BoolOption = TOption<bool>;
    using StringOption = TOption<std::string>;
    using VecStringOption = TOption<std::vector<std::string>>;
    
    
    using LoaderOption = std::variant<IntOption,
                                      StringOption,
				      VecStringOption,
				      BoolOption>;

    struct Loader {
      Loader(MiniMC::Model::TypeFactory_ptr& tfac,
             Model::ConstantFactory_ptr& cfac) : tfactory(tfac),
                                                 cfactory(cfac) {}

      virtual ~Loader() {}
      virtual LoadResult loadFromFile(const std::string& file) = 0;
      virtual LoadResult loadFromString(const std::string& str) = 0;

    protected:
      MiniMC::Model::TypeFactory_ptr tfactory;
      MiniMC::Model::ConstantFactory_ptr cfactory;
    };

    using Loader_ptr = std::unique_ptr<Loader>;

    struct LoaderRegistrar {

      LoaderRegistrar(std::string name, std::initializer_list<LoaderOption> opts);
      LoaderRegistrar(std::string name) : LoaderRegistrar (std::move(name),{}) {}
      virtual Loader_ptr makeLoader(MiniMC::Model::TypeFactory_ptr& tfac, Model::ConstantFactory_ptr cfac) = 0;
      auto& getName() const { return name; }
      auto& getOptions () {return options;}

      template<class T>
      auto& getOption (std::size_t i) {return std::get<T> (options.at (i));}
    protected:
      template<class T,class... Args>
      void addOption (Args... args) {options.push_back (T{args...}); }
    private:
      std::string name;
      std::vector<LoaderOption> options;
    };

    const std::vector<LoaderRegistrar*>& getLoaders();
    LoaderRegistrar* findLoader (const std::string& s);
    
  } // namespace Loaders
} // namespace MiniMC

#endif
