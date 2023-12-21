#ifndef _PLUGIN__
#define _PLUGIN__

#include "minimc/cpa/interface.hpp"
#include "options.hpp"
#include "minimc/host/host.hpp"
#include "minimc/support/feedback.hpp"
#include <boost/program_options.hpp>
#include <functional>
#include <string>
#include <unordered_map>

using subcommandfunc = std::function<MiniMC::Host::ExitCodes(MiniMC::Model::Program&&, MiniMC::Support::Messager&,const SetupOptions&)>;
using options_func = std::function<void(boost::program_options::options_description&)>;

struct Command;
bool isCommand(const std::string&);
Command* getRegistrar(const std::string& s);
std::unordered_map<std::string, std::string> getCommandNameAndDescr();

struct Command {
  virtual std::string getName() const = 0;
  virtual std::string getDescritpion() const = 0;
  virtual void addOptions (boost::program_options::options_description& ) {}
  virtual MiniMC::Host::ExitCodes runCommand (MiniMC::Model::Program&& p, MiniMC::Support::Messager& m,const SetupOptions&) = 0;
};

struct CommandOldStyle : public Command {
  CommandOldStyle(std::string s, subcommandfunc func, std::string desc, options_func ofunc);
  std::string getName() const override { return s; }
  std::string getDescritpion() const override { return desc; }
  void addOptions (boost::program_options::options_description& options) override {
    opt (options);
  }

  MiniMC::Host::ExitCodes runCommand (MiniMC::Model::Program&& p, MiniMC::Support::Messager& m,const SetupOptions& sopt) override {
    return func (std::move(p),m,sopt);
  }

private:
  std::string s;
  subcommandfunc func;
  std::string desc;
  options_func opt;
};

bool parseOptionsAddHelp(boost::program_options::variables_map& map, boost::program_options::options_description& opt, std::vector<std::string>& params);

void registerCommand (const std::string& s,Command& r);

template <class T>
struct CommandRegistrar {
  template<class... Args>
  CommandRegistrar (Args... args)  : t(args...){
    registerCommand (t.getName(),t);
  }
  T t;
};

#endif
