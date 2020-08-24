#ifndef _PLUGIN__
#define _PLUGIN__

#include <string>
#include <functional>
#include <unordered_map>
#include <boost/program_options.hpp>

#include "loaders/loader.hpp"
#include "algorithms/algorithm.hpp"


using subcommandfunc= std::function<int(MiniMC::Model::Program_ptr&,std::vector<std::string>&, MiniMC::Algorithms::SetupOptions&)>;


struct CommandRegistrar;
void registerCommand (const std::string&, CommandRegistrar&);
bool isCommand (const std::string&);
subcommandfunc getCommand (const std::string&);
std::unordered_map<std::string,std::string> getCommandNameAndDescr ();

struct CommandRegistrar {
  CommandRegistrar (const std::string& s,subcommandfunc func, const std::string& desc) : s(s),func(func),desc(desc) {
	registerCommand (s,*this);
  }
  std::string getName () const {return s;}
  std::string getDescritpion () const { return desc;}
  subcommandfunc getFunction () const {return func;}
  
private:
  std::string s;
  subcommandfunc func;
  std::string desc;
};


bool parseOptionsAddHelp (boost::program_options::variables_map& map, boost::program_options::options_description& opt, std::vector<std::string>& params);

#endif
