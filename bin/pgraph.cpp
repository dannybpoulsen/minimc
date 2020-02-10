#include <boost/program_options.hpp>

#include "support/feedback.hpp"
#include "support/sequencer.hpp"
#include "algorithms/printgraph.hpp"
#include "model/modifications/rremoveretsentry.hpp"
#include "model/modifications/replacememnondet.hpp"
#include "model/checkers/typechecker.hpp"
#include "cpa/location.hpp"
#include "cpa/concrete_no_mem.hpp"
#include "cpa/compound.hpp"


#include "loaders/loader.hpp"

namespace po = boost::program_options;

auto createLoader (int val) {
  return MiniMC::Loaders::makeLoader<MiniMC::Loaders::Type::LLVM> ();
}

template<class CPADef>
void runAlgorithm (MiniMC::Model::Program& prgm) {
  using algorithm = MiniMC::Algorithms::PrintCPA<CPADef>;
  auto mess = MiniMC::Support::makeMessager (MiniMC::Support::MessagerType::Terminal);
  MiniMC::Support::Sequencer<MiniMC::Model::Program> seq;
  seq.template add<MiniMC::Model::Modifications::RemoveRetEntryPoints> ();
  seq.template add<MiniMC::Model::Checkers::TypeChecker, MiniMC::Support::Messager&> (*mess);
  seq.template add<MiniMC::Algorithms::AWrapper<algorithm>, MiniMC::Support::Messager&> (*mess);
  seq.run (prgm);
}

int main (int argc,char* argv[]) {
  

  int cpaSelected = 0;
  po::options_description desc("General Options");
  std::string input;
  bool help;
  desc.add_options()
    ("cpa,c",po::value<int>(&cpaSelected), "CPA\n"
     "\t 1: Location\n"
     "\t 2: Location and explicit stack-variable\n"
     )
    ("task",boost::program_options::value< std::vector< std::string > >(),"Add task as entrypoint")
    ("inputfile",po::value<std::string> (&input),"Input file")
    ("help,h",po::bool_switch(&help), "Help message.")
    ;
  
  po::positional_options_description positionalOptions; 
  positionalOptions.add("inputfile", 1); 
  po::variables_map vm; 
  
  try {
    po::store(po::command_line_parser(argc, argv).options(desc) 
	      .positional(positionalOptions).run(), vm);
    po::notify (vm);
    
  }
  catch(po::error& e) {
    if (help) {
      std::cerr << desc;
      return 0;
    }
    std::cerr << e.what () << std::endl;
    return -1;
  }

  if (help)
    std::cerr << desc;
  
  auto loader = createLoader (0);
  MiniMC::Model::TypeFactory_ptr tfac = std::make_shared<MiniMC::Model::TypeFactory64> ();
  MiniMC::Model::ConstantFactory_ptr cfac = std::make_shared<MiniMC::Model::ConstantFactory64> ();
  auto prgm = loader->loadFromFile (input,tfac,cfac);

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

  if (!prgm->hasEntryPoints ()) {
    std::cerr << "Please specify entry points functions with --task\n";
    return 0;
  }
  
  using LocExpliStack = MiniMC::CPA::Compounds::CPADef<0,
						       MiniMC::CPA::Location::CPADef,
						       MiniMC::CPA::ConcreteNoMem::CPADef
						       >;
  switch (cpaSelected) {
  
  case 2:
    runAlgorithm<LocExpliStack> (*prgm);
    break;
  case 1:
  default:
    runAlgorithm<MiniMC::CPA::Location::CPADef> (*prgm);
    break;
  }
}
