#ifndef _PLUGIN__
#define _PLUGIN__

#include "cpa/interface.hpp"
#include "model/controller.hpp"
#include "options.hpp"
#include "host/host.hpp"
#include <boost/program_options.hpp>
#include <functional>
#include <string>
#include <unordered_map>

using subcommandfunc = std::function<MiniMC::Host::ExitCodes(MiniMC::Model::Controller&, const MiniMC::CPA::AnalysisBuilder&)>;
using options_func = std::function<void(boost::program_options::options_description&)>;

struct CommandRegistrar;
void registerCommand(const std::string&, CommandRegistrar&);
bool isCommand(const std::string&);
subcommandfunc getCommand(const std::string&);
CommandRegistrar* getRegistrar(const std::string& s);
options_func getOptionsFunc(const std::string&);
std::unordered_map<std::string, std::string> getCommandNameAndDescr();

struct CommandRegistrar {
  CommandRegistrar(const std::string& s, subcommandfunc func, const std::string& desc, options_func ofunc) : s(s), func(func), desc(desc), opt(ofunc) {
    registerCommand(s, *this);
  }
  std::string getName() const { return s; }
  std::string getDescritpion() const { return desc; }
  subcommandfunc getFunction() const { return func; }
  options_func getOptions() const { return opt; }

private:
  std::string s;
  subcommandfunc func;
  std::string desc;
  options_func opt;
};

bool parseOptionsAddHelp(boost::program_options::variables_map& map, boost::program_options::options_description& opt, std::vector<std::string>& params);

#endif
