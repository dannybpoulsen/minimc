#include "minimc/support/config.h"
#include "options.hpp"
#include "minimc/smt/smt.hpp"
#include "plugin.hpp"
#include "minimc/cpa/location.hpp"
#ifdef MINIMC_SYMBOLIC
#include "minimc/cpa/pathformula.hpp"
#endif
#include "minimc/cpa/concrete.hpp"
#include "minimc/loaders/loader.hpp"
#include <boost/program_options.hpp>


namespace po = boost::program_options;

void printBanner(std::ostream& os) {
  os << MiniMC::Support::Version::TOOLNAME << " " << MiniMC::Support::Version::VERSION_MAJOR << "." << MiniMC::Support::Version::VERSION_MINOR << " (" << __DATE__ << ")" << std::endl;
  os << "Revision: " << MiniMC::Support::Version::GIT_HASH << std::endl;
  os << std::endl;
}

po::options_description transformOptions (SetupOptions& options) {
  po::options_description general("Transform Options");
  general.add_options()
    ("transform.expand_nondet", boost::program_options::bool_switch(&options.transform.expand_nondet) , "Expand Nondeterministic")
    ("transform.unroll", boost::program_options::value(&options.transform.unrollLoops) , "Unroll (all) loops")
    ("transform.inline", boost::program_options::value(&options.transform.inlineFunctions) , "Inline function calls for all entry_points");
    
    
  return general;
}

template<typename ... Ts>                                                 // (7) 
struct Overload : Ts ... { 
    using Ts::operator() ...;
};

std::unordered_map<std::string,MiniMC::Loaders::Loader_ptr > loaders;

po::options_description loadOptions (SetupOptions& options) {
  
  po::options_description general("Load Options");
  
  auto setLoader = [&options](auto& val) {
    auto load = loaders.find (val);
    if ( load != loaders.end ()) {
      options.load.loader = load->second;
    }
    else
      throw MiniMC::Support::ConfigurationException ("Can't find specificed Loader");
  };

  general.add_options()
    ("inputfile", po::value<std::string>(&options.load.inputname), "Input file");
  std::stringstream str;
  str << "Model Loader\n";
  int i = 0;
  for (auto& loader : MiniMC::Loaders::getLoaders ()) {
    str << "\t "  << loader->getName () << "\n";
    i++;
  }
  general.add_options ()
    ("loader",po::value<std::string> ()->default_value(std::string{"LLVM"})->notifier (setLoader),str.str().c_str());

  for (auto& loader_reg : MiniMC::Loaders::getLoaders ()) {
    auto loader = loader_reg->makeLoader ();
    loaders.insert (std::make_pair (loader_reg->getName (),loader));
    
    po::options_description opt_arr(loader_reg->getName ());
    for (auto& opt : loader->getOptions ()) {
      std::visit (
		  Overload {
		    [loader_reg,&opt_arr](MiniMC::Loaders::BoolOption& t) {
		      std::stringstream str;
		      str << loader_reg->getName() <<"."<<t.name;
		      opt_arr.add_options ()
			(str.str().c_str (),boost::program_options::bool_switch(t.value),t.description.c_str());
		    
		    },
		    [loader_reg,&opt_arr](auto& t){
		      std::stringstream str;
		      str << loader_reg->getName() <<"."<<t.name;
		      opt_arr.add_options ()
			(str.str().c_str (),boost::program_options::value(t.value),t.description.c_str());
		    },
		      
		      },
		  opt
		  );
      
      
    
    }
    general.add (opt_arr);
    
  }
  
  return general;
}



po::options_description smtOptions (SetupOptions& options) {
  po::options_description smt("SMT Options");
  std::vector<MiniMC::Support::SMT::SMTDescr> smts;
  MiniMC::Support::SMT::getSMTBackends(std::back_inserter(smts));

  auto setSMTSolver = [smts,&options](std::size_t val) {
    if (val < smts.size()) {
      options.smt.selsmt = smts[val];
    }
  };

  if (smts.size()) {
    std::stringstream str;
    str << "SMT Solver\n";
    int i = 0;
    for (auto& ss : smts) {
      str << "\t " << i << ": " << ss.name() << "\n";
      i++;
    }
    
    smt.add_options()("smt.solver", po::value<std::size_t>()->default_value(0)->notifier(setSMTSolver), str.str().c_str());
  }

  
  return smt;
}

po::options_description defOptions (SetupOptions& options) {
  auto selCommand = [&options](const std::string& sel) {
    if (isCommand (sel)) {
      options.command = getRegistrar (sel);
    }
  };

  po::options_description general("hidden");
  general.add_options ()
    ("command",po::value<std::string> ()->notifier(selCommand),"Select command");
  
  return general;
}


void addCommandOptions (po::options_description& general) {
  auto commands = getCommandNameAndDescr ();
  for (auto it : commands) {
    auto registrar = getRegistrar (it.first);
    registrar->addOptions(general);
  }
}

bool parseOptions(int argc, char* argv[], SetupOptions& opt)  {
  
  bool help;

  po::options_description options("MiniMC");
  po::options_description general("General");
  general.add_options ()
    ("config", boost::program_options::value<std::string>(), "Read configuration from config file")
    ("help", boost::program_options::bool_switch(&help), "Show help")
    ("outputfile", boost::program_options::value<std::string> (&opt.outputname), "Output verified program to file");
  
  general.add(loadOptions (opt));
  general.add(smtOptions (opt));
  general.add(transformOptions (opt));
  
  addCommandOptions (general);

  options.add(defOptions (opt));
  options.add (general);
  
  po::positional_options_description pos;
  pos.add("inputfile", 1).add("command", 1);
  
  try {
    po::variables_map vm;
    po::parsed_options parsed = po::command_line_parser(argc, argv)
      .options(options)
      .positional(pos)
      .run();
    
    po::store(parsed, vm);
    po::notify(vm);

    if (vm.count("config")) {
      auto parsed = parse_config_file(vm["config"].as<std::string>().c_str(), options, true);
      po::store(parsed, vm);
      po::notify(vm);
    }

    if (help){
      printBanner(std::cerr);
      std::cerr << "Usage: " << MiniMC::Support::Version::TOOLNAME << "[OPTIONS] INPUT SUBCOMMAND" << std::endl;
      std::cerr << general << std::endl;
      std::cerr << "Subcommands" << std::endl;
      auto comms = getCommandNameAndDescr();
      for (auto& it : comms) {
	std::cerr << it.first << "\t" << it.second << std::endl;
      }
      
      return false;
    }

    return true;
    
  }

  
  
  catch (po::error& e) {
    std::cerr << e.what () << std::endl;
    return false;
  }
  
}
