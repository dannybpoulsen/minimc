#ifndef _PLUGIN__
#define _PLUGIN__

#include <string>
#include <functional>
#include <unordered_map>
#include <boost/program_options.hpp>

#include "loaders/loader.hpp"
#include "algorithms/algorithm.hpp"
#include "support/host.hpp"


using subcommandfunc= std::function<MiniMC::Support::ExitCodes(MiniMC::Model::Program_ptr&,MiniMC::Algorithms::SetupOptions&)>;
using options_func = std::function<void(boost::program_options::options_description&,MiniMC::Algorithms::SetupOptions&)>;


struct CommandRegistrar;
void registerCommand (const std::string&, CommandRegistrar&);
bool isCommand (const std::string&);
subcommandfunc getCommand (const std::string&);
options_func getOptionsFunc (const std::string&);
std::unordered_map<std::string,std::string> getCommandNameAndDescr ();

struct CommandRegistrar {
  CommandRegistrar (const std::string& s,subcommandfunc func, const std::string& desc, options_func ofunc) : s(s),func(func),desc(desc),opt(ofunc) {
	registerCommand (s,*this);
  }
  std::string getName () const {return s;}
  std::string getDescritpion () const { return desc;}
  subcommandfunc getFunction () const {return func;}
  options_func getOptions () const {return opt;}
  
private:
  std::string s;
  subcommandfunc func;
  std::string desc;
  options_func opt;
};


bool parseOptionsAddHelp (boost::program_options::variables_map& map, boost::program_options::options_description& opt, std::vector<std::string>& params);

#endif
