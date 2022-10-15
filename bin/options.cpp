#include "config.h"
#include "options.hpp"
#include "support/smt.hpp"
#include "plugin.hpp"
#include "cpa/location.hpp"
#ifdef MINIMC_SYMBOLIC
#include "cpa/pathformula.hpp"
#endif
#include "cpa/concrete.hpp"
#include "loaders/loader.hpp"
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
    ("transform.expand_nondet", boost::program_options::bool_switch(&options.transform.expand_nondet) , "Expand Non");
  return general;
}

po::options_description loadOptions (SetupOptions& options) {
  
  po::options_description general("Load Options");
  
  auto setLoader = [&options](std::size_t val) {
    if (val < MiniMC::Loaders::getLoaders ().size ()) {
      options.load.registrar = MiniMC::Loaders::getLoaders ().at (val);
    }
    else
      throw MiniMC::Support::ConfigurationException ("Can't find specificed Loader");
  };
  
  general.add_options()
    ("inputfile", po::value<std::string>(&options.load.inputname), "Input file")
    ("task", boost::program_options::value<std::vector<std::string>>(&options.load.tasks), "Add task as entrypoint");
  std::stringstream str;
  str << "Model Loader\n";
  int i = 0;
  for (auto& loader : MiniMC::Loaders::getLoaders ()) {
    str << "\t " << i << ": " << loader->getName () << "\n";
    i++;
  }
  general.add_options ()
    ("loader",po::value<std::size_t> ()->default_value(0)->notifier (setLoader),str.str().c_str());

  for (auto& loader : MiniMC::Loaders::getLoaders ()) {
    
    po::options_description opt_arr(loader->getName ());
    for (auto& opt : loader->getOptions ()) {
      
      std::visit ([loader,&opt_arr](auto& t){
	std::stringstream str;
	str << loader->getName() <<"."<<t.name;
    
	opt_arr.add_options ()
	  (str.str().c_str (),boost::program_options::value(&t.value),t.description.c_str());
      },
	opt
	);
      general.add (opt_arr);
    }
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




po::options_description cpaOptions (std::vector<int>& select) {  
  po::options_description general("CPA Options");
  general.add_options ()
    ("cpa", po::value<std::vector<int>>(&select)->multitoken (), "CPA\n 2: Concrete\n"
#ifdef MINIMC_SYMBOLIC
     "\t 3: PathFormula\n"
#endif
     );
  
  return general;
}

MiniMC::CPA::AnalysisBuilder createUserDefinedCPA(std::vector<int> selected,const SetupOptions& opt) {
  MiniMC::CPA::AnalysisBuilder builder{std::make_shared<MiniMC::CPA::Location::CPA> ()};
  
  for (auto& sel : selected) {
    switch (sel) {
    case 3:
      builder.addDataCPA(std::make_shared<MiniMC::CPA::PathFormula::CPA>(opt.smt.selsmt));
      break;
    case 2:
      builder.addDataCPA (std::make_shared<MiniMC::CPA::Concrete::CPA>());
      break;
      
    }
  }
  return builder;
  
}


void addCommandOptions (po::options_description& general) {
  auto commands = getCommandNameAndDescr ();
  for (auto it : commands) {
    auto registrar = getRegistrar (it.first);
    registrar->getOptions()(general);
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
  
  std::vector<int> cpasel;
  general.add(cpaOptions (cpasel));
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
    
    opt.cpa = createUserDefinedCPA (cpasel,opt);
    
    return true;
    
  }

  
  
  catch (po::error& e) {
    std::cerr << e.what () << std::endl;
    return false;
  }
  
}
