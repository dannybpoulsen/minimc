#include "options.hpp"
#include "support/smt.hpp"
#include "plugin.hpp"
#include "cpa/location.hpp"
#ifdef MINIMC_SYMBOLIC
#include "cpa/pathformula.hpp"
#endif
#include "cpa/compound.hpp"


#include <boost/program_options.hpp>


namespace po = boost::program_options;

po::options_description modificationOptions (SetupOptions& options) {
  po::options_description general("Modifications Options");
  general.add_options()
    ("simplifycfg", boost::program_options::bool_switch(&options.modifications.simplifyCFG), "Simplify the CFG structure")
    ("constfold", boost::program_options::bool_switch(&options.modifications.foldConstants), "Constant Folding")
    ("replacesub", boost::program_options::bool_switch(&options.modifications.replaceSub), "Replace sub instructions")
    ("inlinefunctions", po::value<std::size_t>(&options.modifications.inlinefunctions), "Inline function calls")
    ("unrollloops", po::value<std::size_t>(&options.modifications.unrollLoops), "Unroll Loops")
    ("removeallocas", po::bool_switch(&options.modifications.removeAllocs), "Remove Alloca (replace them with equivalent construction)")
    ("replacememnondet", po::bool_switch(&options.modifications.replacememnodet), "Remove Alloca (replace them with equivalent construction)")
    ("removephi", po::bool_switch(&options.modifications.removephi), "Removephi");
    
  return general;
}

po::options_description loadOptions (SetupOptions& options) {
  auto addTask = [&options](const std::string s) {
    options.load.tasks.push_back (s);
  };
  po::options_description general("Load Options");
  general.add_options()
    ("inputfile", po::value<std::string>(&options.load.inputname), "Input file")
    ("task", boost::program_options::value<std::string>()->notifier (addTask), "Add task as entrypoint");
  
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

enum class CPASel {
  Location,
  LocationConcrete,
  LocationPathformula
};


po::options_description cpaOptions (CPASel& select) {
  auto selCPA = [&select](const std::size_t sel) {
    switch (sel) {
    case 1:
      select = CPASel::Location;
      break;
    case 2:
      select = CPASel::LocationConcrete;
      break;
    case 3:
      select = CPASel::LocationPathformula;
      break;
    default:
      select = CPASel::Location;
      
    }
  };

  
  po::options_description general("CPA Options");
  general.add_options ()
    ("cpa", po::value<int>()->default_value(0)->notifier(selCPA), "CPA\n 0: Automatic\n\t 1: Location\n\t 2: Concrete\n"
#ifdef MINIMC_SYMBOLIC
     "\t 3: PathFormula\n"
#endif
     );
  
  return general;
}

MiniMC::CPA::CPA_ptr createUserDefinedCPA(CPASel sel,const SetupOptions& opt) {
  switch (sel) {
#ifdef MINIMC_SYMBOLIC

    case CPASel::LocationPathformula:
      return std::make_shared<MiniMC::CPA::Compounds::CPA>(std::initializer_list<MiniMC::CPA::CPA_ptr>({std::make_shared<MiniMC::CPA::Location::CPA>(),
	    std::make_shared<MiniMC::CPA::PathFormula::CPA>(opt.smt.selsmt)}));
      break;

#endif
    case CPASel::LocationConcrete:
      return std::make_shared<MiniMC::CPA::Compounds::CPA>(std::initializer_list<MiniMC::CPA::CPA_ptr>({std::make_shared<MiniMC::CPA::Location::CPA>(),
                                                                                                        std::make_shared<MiniMC::CPA::Concrete::CPA>()}));
      break;

    case CPASel::Location:
    default:

      return std::make_shared<MiniMC::CPA::Location::CPA>();
      // res = runAlgorithm<MiniMC::CPA::Location::CPADef> (*prgm,sopt,locoptions.filter);
      break;
  }
}


void addCommandOptions (po::options_description& general) {
  auto commands = getCommandNameAndDescr ();
  for (auto it : commands) {
    auto registrar = getRegistrar (it.first);
    registrar->getOptions()(general);
  }
}

bool parseOptions(int argc, char* argv[], SetupOptions& opt)  {
  

  po::options_description general("MiniMC");
  
  general.add_options ()
    ("config", boost::program_options::value<std::string>(), "Read configuration from config file");
  general.add(modificationOptions (opt));
  general.add(loadOptions (opt));
  general.add(smtOptions (opt));
  general.add(defOptions (opt));
  CPASel cpasel;
  general.add(cpaOptions (cpasel));
  addCommandOptions (general);
  
  po::positional_options_description pos;
  pos.add("inputfile", 1).add("command", 1);
  
  try {
    po::variables_map vm;
    po::parsed_options parsed = po::command_line_parser(argc, argv)
      .options(general)
      .positional(pos)
      .run();
    
    po::store(parsed, vm);
    po::notify(vm);

    if (vm.count("config")) {
      auto parsed = parse_config_file(vm["config"].as<std::string>().c_str(), general, true);
      po::store(parsed, vm);
      po::notify(vm);
    }

    opt.cpa = createUserDefinedCPA (cpasel,opt);
    
    return true;
    
  }

  catch (po::error& e) {
    std::cerr << e.what () << std::endl;
    return false;
  }
  
}
