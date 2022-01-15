#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <boost/program_options.hpp>
#include "support/localisation.hpp"
#include "support/smt.hpp"
#include "config.h"
#include "loaders/loader.hpp"
#include "algorithms/algorithm.hpp"
#include "support/timing.hpp"
#include "plugin.hpp"

#include "cpa/location.hpp"
#ifdef MINIMC_SYMBOLIC
#include "cpa/pathformula.hpp"
#endif
#include "cpa/concrete.hpp"
#include "cpa/compound.hpp"


namespace po = boost::program_options;

void printBanner (std::ostream& os) {
  os << MiniMC::Support::Version::TOOLNAME << " "<<  MiniMC::Support::Version::VERSION_MAJOR << "." << MiniMC::Support::Version::VERSION_MINOR << " (" << __DATE__  << ")" <<std::endl;
  os << "Revision: " << MiniMC::Support::Version::GIT_HASH << std::endl;
  os << std::endl;
}

CPASelector selectedCPA = CPASelector::Automatic;
std::vector<MiniMC::Support::SMT::SMTDescr> smts;
MiniMC::Support::SMT::SMTDescr* selsmt;

MiniMC::CPA::CPA_ptr createUserDefinedCPA (CPASelector defaultSelector) {      
  assert(defaultSelector != CPASelector::Automatic);
  CPASelector sel = (selectedCPA != CPASelector::Automatic) ? selectedCPA : defaultSelector;
  switch (sel) {
#ifdef MINIMC_SYMBOLIC
  
  case CPASelector::LocationPathformula:
    return std::make_shared<MiniMC::CPA::Compounds::CPA> (std::initializer_list<MiniMC::CPA::CPA_ptr>({
	  std::make_shared<MiniMC::CPA::Location::CPA> (),
	  std::make_shared<MiniMC::CPA::PathFormula::CPA> (MiniMC::Support::SMT::getSMTFactory (selsmt))}));
    break;
    
#endif
  case CPASelector::LocationConcrete:
    return std::make_shared<MiniMC::CPA::Compounds::CPA> (std::initializer_list<MiniMC::CPA::CPA_ptr>({
	  std::make_shared<MiniMC::CPA::Location::CPA> (),
	  std::make_shared<MiniMC::CPA::Concrete::CPA> ()}));
    break;
    
  case CPASelector::Location:
  default:
      
    return std::make_shared<MiniMC::CPA::Location::CPA> ();
    //res = runAlgorithm<MiniMC::CPA::Location::CPADef> (*prgm,sopt,locoptions.filter);
    break;
  }
}



int main (int argc,char* argv[]) {

  std::string input;
  std::string subcommand;
  std::vector<std::string> subargs;
  MiniMC::Support::setMessager (MiniMC::Support::MessagerType::Terminal);
  MiniMC::Algorithms::SetupOptions soptions;
  
  auto updateSpace = [&] (int val) {
					   switch (val) {
					   case 1:
						 soptions.reduction =MiniMC::Algorithms::SpaceReduction::None;
						 break;
					   case 2:
					   default:
						 soptions.reduction =MiniMC::Algorithms::SpaceReduction::Conservative;
						 break;
						 
					   }
					 };
  po::options_description hidden("Hidden");
  po::options_description general("General Options");
  
  auto printHelp  = [&general]() {
	printBanner (std::cerr);
    std::cerr << "Usage: "<< MiniMC::Support::Version::TOOLNAME << "[OPTIONS] INPUT SUBCOMMAND [SUBCOMMAND OPTIONS]" <<std::endl; 
    std::cerr << general << std::endl;
    std::cerr << "Subcommands" << std::endl;
    auto comms = getCommandNameAndDescr ();
    for (auto& it :  comms) {
      std::cerr << it.first <<"\t" << it.second << std::endl;
    }
    return static_cast<int>(MiniMC::Support::ExitCodes::ConfigurationError);
  };

  auto updateCPA = [&soptions] (int val) {
    switch (val) {
#ifdef MINIMC_SYMBOLIC
    case 3:
      soptions.convergencePoints = true;
      
      selectedCPA = CPASelector::LocationPathformula;
      
      break;
#endif
    case 2:
      selectedCPA = CPASelector::LocationConcrete;
      break;
    case 1:
      selectedCPA = CPASelector::Location;
      break;
      
    default:
      selectedCPA = CPASelector::Automatic;
      break;
      
    }
  };
  
  general.add_options()
	("config",boost::program_options::value<std::string>(),"Read configuration from config file")
	("task",boost::program_options::value< std::vector< std::string > >(),"Add task as entrypoint")
	("spacereduction",po::value<int> ()->default_value(1)->notifier(updateSpace), "Space Reduction\n"
	 "\t 1: None\n"
	 "\t 2: Conservative\n"
	 )
    ("simplifycfg",boost::program_options::bool_switch(&soptions.simplifyCFG),"Simplify the CFG structure")
    ("constfold",boost::program_options::bool_switch(&soptions.foldConstants),"Constant Folding")
    ("replacesub",boost::program_options::bool_switch(&soptions.replaceSub),"Replace sub instructions")
    ("inlinefunctions",po::value<std::size_t> (&soptions.inlinefunctions),"Inline function calls")
    ("unrollloops",po::value<std::size_t> (&soptions.unrollLoops),"Unroll Loops")
    ("removeallocas",po::bool_switch (&soptions.removeAllocs),"Remove Alloca (replace them with equivalent construction)")
    ("replacememnondet",po::bool_switch (&soptions.replacememnodet),"Remove Alloca (replace them with equivalent construction)")
    ("removephi",po::bool_switch (&soptions.removephi),"Removephi")
    ("cpa",po::value<int>()->default_value(0)->notifier(updateCPA), "CPA\n"
     "\t 0: Automatic\n"
     "\t 1: Location\n"
     "\t 2: Concrete\n"
#ifdef MINIMC_SYMBOLIC	   
     "\t 3: PathFormula\n"
#endif
     )

    ;
  
  
  for (auto& it :  getCommandNameAndDescr ()) {
	getOptionsFunc (it.first) (general,soptions);
  }
#ifdef MINIMC_SYMBOLIC
  //SMT options
  po::options_description smt("SMT Options");
  
  MiniMC::Support::SMT::getSMTBackends (std::back_inserter (smts));
  auto setSMTSolver = [] (std::size_t val) {
    if (val < smts.size ()) {
      selsmt  = &smts[val];
    }
    else {
      selsmt = &smts[0];
    }
  };
  
  if (smts.size ()) {
	std::stringstream str;
	str<< "SMT Solver\n";
	int i = 0;
	for (auto& ss : smts) {
	  str << "\t " << i << ": " << ss.name () << "\n"; 
	  i++;
	}
	
	smt.add_options ()
	  ("smt.solver",po::value<std::size_t> ()->default_value(0)->notifier(setSMTSolver), str.str ().c_str());
	general.add (smt);
  }
#endif
  
  hidden.add_options()
	("command",po::value<std::string> (&subcommand), "Subcommand")
	("subargs",po::value<std::vector<std::string>> (),"Subcommand parameteers")
	("inputfile",po::value<std::string> (&input),"Input file")
	;

  
  po::options_description desc;
  desc.add (general).add(hidden);

  
  
  
  po::positional_options_description pos;
  pos.add("inputfile", 1).
	add("command", 1).
    add("subargs", -1);
  
  try {
	po::variables_map vm; 
	po::parsed_options parsed = po::command_line_parser(argc, argv)
	  .options(desc)
	  .positional(pos)
	  .allow_unregistered ()
	  .run();
	po::store(parsed,vm);
	po::notify (vm);

	if (vm.count("config")) {
	  auto parsed =parse_config_file(vm["config"].as<std::string>().c_str(), desc,true); 
	  po::store(parsed, vm);
	  po::notify(vm);
	  subargs = po::collect_unrecognized (parsed.options, po::include_positional);
	  for (auto t : po::collect_unrecognized(parsed.options, po::include_positional)) {
		subargs.push_back(t);
	  }
	}
	
	if (!vm.count ("command")) {
	  printHelp ();
	  return static_cast<int>(MiniMC::Support::ExitCodes::ConfigurationError);
	}

	
	if (!vm.count ("inputfile")) {
	  printHelp ();

	  return static_cast<int>(MiniMC::Support::ExitCodes::ConfigurationError);
	}
	
	//Load Program
	MiniMC::Model::TypeFactory_ptr tfac = std::make_shared<MiniMC::Model::TypeFactory64> ();
	MiniMC::Model::ConstantFactory_ptr cfac = std::make_shared<MiniMC::Model::ConstantFactory64> ();
	MiniMC::Model::Program_ptr prgm = MiniMC::Loaders::loadFromFile<MiniMC::Loaders::Type::LLVM> (input, typename MiniMC::Loaders::OptionsLoad<MiniMC::Loaders::Type::LLVM>::Opt {.tfactory = tfac,
																																												  .cfactory =cfac}
	  );
	
  assert(prgm);
  
  if (vm.count ("task")) {
    std::vector< std::string > entries = vm["task"].as< std::vector< std::string > >();
    for (std::string& s : entries) {
	  try {
		auto func = prgm->getFunction (s);
		auto entry = createEntryPoint (prgm,func);
		prgm->addEntryPoint (entry->getName());
	  }
	  catch (MiniMC::Support::FunctionDoesNotExist& ){
		std::cerr << MiniMC::Support::Localiser{"Function '%1%' specicifed as entry point does not exists. "}.format (s) << std::endl;
		return -1;
      }
    }
  }
  if (prgm->getEntryPoints ().size () > 1)
    soptions.isConcurrent = true;
  if (prgm->getEntryPoints().size() == 0) {
    std::cerr << MiniMC::Support::Localiser{"At least one entry point must be specified. "}.format() << std::endl;
    return -1;
  }
  
  
  if (isCommand (subcommand)) {
	
	return static_cast<int>(getCommand(subcommand) (prgm,soptions));
  }
  
  else {
    std::cerr << MiniMC::Support::Localiser{"Unknown subcommand '%1%"}.format (subcommand) << std::endl;
	return static_cast<int>(MiniMC::Support::ExitCodes::ConfigurationError);
	  
  }
  
  }
  catch(po::error& e) {
	printHelp ();
    return static_cast<int>(MiniMC::Support::ExitCodes::ConfigurationError);
  }
  catch (MiniMC::Support::ConfigurationException& ex) {
	std::cerr << "Configuration error: " << ex.what () << std::endl;
	return static_cast<int>(MiniMC::Support::ExitCodes::ConfigurationError);
  }
  catch (MiniMC::Support::Exception& ex) {
    std::cerr << "An error happended: " << ex.what () << std::endl;
    return static_cast<int>(MiniMC::Support::ExitCodes::RuntimeError);
  }
  
    
}
