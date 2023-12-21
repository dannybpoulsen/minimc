#ifndef _LOADER__
#define _LOADER__

#include "minimc/model/cfg.hpp"
#include "minimc/support/exceptions.hpp"
#include "minimc/support/localisation.hpp"
#include "minimc/support/feedback.hpp"


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

    template <class T>
    struct TOption {
      using ValueType = T;
      TOption (const std::string name,
	       const std::string descr,
	       T* value) : name(name), description(descr),value(value) {}
      void set (const T& t) {*value=t;}
      std::string name;
      std::string description;
      T* value;
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
      Loader()  {}
      
      virtual ~Loader() {}
      virtual MiniMC::Model::Program loadFromFile(const std::string& file, MiniMC::Model::TypeFactory_ptr& tfac, Model::ConstantFactory_ptr& cfac, MiniMC::Support::Messager&) = 0;
      virtual MiniMC::Model::Program loadFromString(const std::string& str, MiniMC::Model::TypeFactory_ptr& tfac, Model::ConstantFactory_ptr& cfac,MiniMC::Support::Messager&) = 0;

      template<class T>
      auto& getOption (std::size_t i) {return std::get<T> (options.at (i));}
      
      template<class T>
      void setOption (std::size_t i, typename T::ValueType t) {std::get<T> (options.at(i)).set (t);}

      auto& getOptions () {return options;}
    protected:
      template<class T,class... Args>
      void addOption (Args... args) {options.push_back (T{args...}); }
      std::vector<LoaderOption> options;
    };

    using Loader_ptr = std::shared_ptr<Loader>;

    struct LoaderRegistrar {
      LoaderRegistrar(std::string name);
      virtual Loader_ptr makeLoader() = 0;
      auto& getName() const { return name; }
      std::string name;
    };

    const std::vector<LoaderRegistrar*>& getLoaders();
    LoaderRegistrar* findLoader (const std::string& s);
    
  } // namespace Loaders
} // namespace MiniMC

#endif
