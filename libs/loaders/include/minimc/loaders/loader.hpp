#ifndef _LOADER__
#define _LOADER__

#include "minimc/model/cfg.hpp"
#include "minimc/support/exceptions.hpp"
#include "minimc/support/localisation.hpp"
#include "minimc/support/feedback.hpp"
#include "minimc/support/overload.hpp"


#include <memory>
#include <string>
#include <vector>
#include <variant>
#include <expected>
#include <generator>
#include <unordered_map>
#include <iostream>
#include <initializer_list>

namespace MiniMC {
  namespace Loaders {

    class LoadError : public MiniMC::Support::ConfigurationException {
    public:
      LoadError(const std::string& mess = "Failed to load program" ) : ConfigurationException(mess) {}
    };
    
    template <class T>
    struct TOption {
      TOption (const TOption& ) = default;
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

    
    enum class Error {
      ParseError,
      ProgramError,
      LoadFailed
    };

    using LoadResult = std::expected<MiniMC::Model::Program,Error>;

    struct Loader {
      virtual LoadResult loadFromFile(const std::string& file, MiniMC::Support::Messager&) = 0;
      virtual LoadResult loadFromString(const std::string& str, MiniMC::Support::Messager&) = 0;
      virtual std::string getName() const = 0;

      template<class T>
      void setOption (std::string i, T t) {
	std::visit (MiniMC::Support::Overload {
	    [&t]<typename Opt>(Opt opt) requires std::is_same_v<T,typename Opt::ValueType> {
	      opt.set(t);},
	      MiniMC::Support::Error<void> {}
	  },
	  getOption (i)
	  );
	
      }
      virtual std::generator<LoaderOption> getOptions ()  = 0;
      
    protected:
      virtual LoaderOption  getOption(std::string) = 0; 
    };
    
    struct LoaderDirect : public Loader {
      LoaderDirect(const std::string name) : name(std::move(name))  {}
      
      virtual ~LoaderDirect() {}
      virtual LoadResult loadFromFile(const std::string& file, MiniMC::Support::Messager&) = 0;
      virtual LoadResult loadFromString(const std::string& str, MiniMC::Support::Messager&) = 0;
      
      
      
      std::string getName() const override {return name;}
      std::generator<LoaderOption> getOptions () override {
	for (auto& g : options)
	  co_yield g.second;
      }
    protected:
      LoaderOption getOption (std::string s) {return options.at (s);}
      
      template<class T,class Arg>
      void addOption (std::string name, std::string descr, Arg args) {options.emplace (getName()+"."+name,T{getName()+"."+name,descr,args}); }
      std::unordered_map<std::string,LoaderOption> options;
      std::string name;
    };

    using Loader_ptr = std::shared_ptr<Loader>;

    
    struct GenericLoader : public Loader {
      GenericLoader ();
      
      LoadResult loadFromFile(const std::string& file, MiniMC::Support::Messager&) override;
      LoadResult loadFromString(const std::string& str, MiniMC::Support::Messager&) override;
      
      
      std::string getName() const override {return "Generic";}
      std::generator<LoaderOption> getOptions () override {
	for (auto& l : loaders) {
	  for (auto g : l->getOptions()) {
	    co_yield g;
	  }
	}
      }
    protected:
    protected:
      LoaderOption getOption (std::string) {throw MiniMC::Support::Exception ("Cannot set option on GenericLoader");}
    private:
      std::vector<Loader_ptr> loaders;
    };
    

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
