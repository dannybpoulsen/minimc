#include <unordered_map>
#include <boost/program_options.hpp>

#include "plugin.hpp"


auto& getMap () {
  static std::unordered_map<std::string,Command*> map;
  return map;
}

void registerCommand (const std::string& s,Command& r) {
  getMap().insert(std::make_pair(s,&r));
}

bool isCommand (const std::string& s) {
  return getMap().count (s);
}


CommandOldStyle::CommandOldStyle(std::string s, subcommandfunc func, std::string desc, options_func ofunc) : s(s), func(func), desc(desc), opt(ofunc) {
}

Command* getRegistrar (const std::string& s) {
  return getMap().at(s);
}

std::unordered_map<std::string,std::string> getCommandNameAndDescr () {
  std::unordered_map<std::string,std::string>res;
  for (auto& it : getMap ()) {
    res.insert (std::make_pair(it.first,it.second->getDescritpion ()));
  }
  return res;
}

bool parseOptionsAddHelp (boost::program_options::variables_map& vm, boost::program_options::options_description& desc, std::vector<std::string>& params) {
  bool help;
  desc.add_options()
    ("help,c",boost::program_options::bool_switch(&help), "Print Help Message");
  
  try {
    boost::program_options::store(boost::program_options::command_line_parser(params).
				  options(desc) 
				  .run(), vm);
    boost::program_options::notify (vm);
    
  }
  catch(boost::program_options::error& e) {
    if (help) {
      std::cerr << desc << std::endl;
    }
    else 
      std::cerr << e.what () << std::endl;
    
    return false;
  }
  
  if (help) {
    std::cerr << desc << std::endl;
    return false;
  }
  
  
  
  return true;
  
  
}



