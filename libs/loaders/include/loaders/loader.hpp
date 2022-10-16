#ifndef _LOADER__
#define _LOADER__

#include "model/cfg.hpp"
#include "model/controller.hpp"
#include "support/exceptions.hpp"

#include <memory>
#include <string>
#include <variant>
#include <initializer_list>

namespace MiniMC {
  namespace Loaders {

    class LoadError : public MiniMC::Support::ConfigurationException {
    public:
      LoadError() : ConfigurationException("Failed to load program") {}
    };

    struct LoadResult {
      MiniMC::Model::Program_ptr program;
      MiniMC::Model::entry_creator entrycreator;
    };

    template <class T>
    struct TOption {
      std::string name;
      std::string description;
      T value;
    };

    using IntOption = TOption<std::size_t>;
    using StringOption = TOption<std::string>;

    using LoaderOption = std::variant<IntOption,
                                      StringOption>;

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
    private:
      std::string name;
      std::vector<LoaderOption> options;
    };

    const std::vector<LoaderRegistrar*>& getLoaders();
    
  } // namespace Loaders
} // namespace MiniMC

#endif
