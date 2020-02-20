#include <boost/program_options.hpp>

#include "support/feedback.hpp"
#include "support/sequencer.hpp"
#include "support/statistical/fixed_effort.hpp"
#include "algorithms/pprintgraph.hpp"

#include "loaders/loader.hpp"

namespace po = boost::program_options;

auto createLoader (int val) {
  return MiniMC::Loaders::makeLoader<MiniMC::Loaders::Type::LLVM> ();
}


void runAlgorithm (MiniMC::Model::Program& prgm,std::size_t samples, std::size_t length) {
  using algorithm = MiniMC::Algorithms::ProbaChecker<MiniMC::Support::Statistical::FixedEffort>;
  auto mess = MiniMC::Support::makeMessager (MiniMC::Support::MessagerType::Terminal);
  MiniMC::Support::Sequencer<MiniMC::Model::Program> seq;
  MiniMC::Algorithms::setupForAlgorithm<algorithm,std::size_t,std::size_t> (seq,*mess,samples,length);
  
  seq.run (prgm);
}

int main (int argc,char* argv[]) {
  

  int cpaSelected = 0;
  po::options_description desc("General Options");
  std::string input;
  bool help;
  std::size_t length = 100;
  std::size_t samples = 100;
  
  desc.add_options()
    ("task",boost::program_options::value< std::vector< std::string > >(),"Add task as entrypoint")
    ("inputfile",po::value<std::string> (&input),"Input file")
	("samples",po::value<std::size_t> (&samples),"Samples")
	("length",po::value<std::size_t> (&length),"Length")
	
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
  runAlgorithm (*prgm,samples,length);
    
}
