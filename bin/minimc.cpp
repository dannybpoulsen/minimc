#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <boost/program_options.hpp>
#include "support/localisation.hpp"
#include "loaders/loader.hpp"
#include "algorithms/algorithm.hpp"
#include "plugin.hpp"

namespace po = boost::program_options;

int pgraph_main (MiniMC::Model::Program_ptr& prgm, std::vector<std::string>& parameters, const MiniMC::Algorithms::SetupOptions&);

int mc_main (MiniMC::Model::Program_ptr& prgm, std::vector<std::string>& parameters,const MiniMC::Algorithms::SetupOptions&);

int enum_main (MiniMC::Model::Program_ptr& prgm, std::vector<std::string>& parameters, const MiniMC::Algorithms::SetupOptions&);

int smc_main (MiniMC::Model::Program_ptr& prgm, std::vector<std::string>& parameters, const MiniMC::Algorithms::SetupOptions&);



int main (int argc,char* argv[]) {
  po::options_description desc("General Options");
  std::string input;
  std::string subcommand;
  auto mess = MiniMC::Support::makeMessager (MiniMC::Support::MessagerType::Terminal);
  MiniMC::Algorithms::SetupOptions soptions {.messager = mess.get()};

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
  
  
  desc.add_options()
    ("task",boost::program_options::value< std::vector< std::string > >(),"Add task as entrypoint")
    ("inputfile",po::value<std::string> (&input)->required(),"Input file")
	("spacereduction",po::value<int> ()->default_value(1)->notifier(updateSpace), "Space Reduction\n"
	 "\t 1: None\n"
	 "\t 2: Conservative\n"
	 )
    ("command",po::value<std::string> (&subcommand)->required(), "Subcommand")
	("subargs",po::value<std::vector<std::string>> (),"Subcommand parameteers");

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

	//Load Program
  MiniMC::Model::TypeFactory_ptr tfac = std::make_shared<MiniMC::Model::TypeFactory64> ();
  MiniMC::Model::ConstantFactory_ptr cfac = std::make_shared<MiniMC::Model::ConstantFactory64> ();
  MiniMC::Model::Program_ptr prgm = MiniMC::Loaders::loadFromFile<MiniMC::Loaders::Type::LLVM> (input, typename MiniMC::Loaders::OptionsLoad<MiniMC::Loaders::Type::LLVM>::Opt {.tfactory = tfac,
																																												  .cfactory =cfac}
	);
  
  assert(prgm);
  
  if (vm.count ("task")) {
    std::vector< std::string > entries = vm["task"].as< std::vector< std::string > >();
    std::unordered_map<std::string,MiniMC::Model::Function_ptr> fmap;
    for (auto f: prgm->getFunctions ()) {
      fmap.insert (std::make_pair(f->getName(),f));
    }
    for (std::string& s : entries) {
      prgm->addEntryPoint (fmap.at(s));
    }
  }
  
  std::vector<std::string> subargs = po::collect_unrecognized(parsed.options, po::include_positional);
  subargs.erase(subargs.begin(),subargs.begin()+2);
  
  if (isCommand (subcommand)) {
	return getCommand(subcommand) (prgm,subargs,soptions);
  }
  
  else {
	std::cerr << MiniMC::Support::Localiser{"Unknown subcommand '%1%"}.format (subcommand) << std::endl;
  }
  
  }
  catch(po::error& e) {
    std::cerr << e.what () << std::endl;
	std::cerr << desc << std::endl;
	return -1;
  }
  
    
}
