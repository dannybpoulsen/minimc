#include <boost/program_options.hpp>
#include <string>
#include <vector>


#include "support/feedback.hpp"
#include "support/sequencer.hpp"
#include "algorithms/printgraph.hpp"
#include "savers/savers.hpp"

#include "cpa/location.hpp"
#include "cpa/pathformula.hpp"
#include "cpa/compound.hpp"

#include "plugin.hpp"

namespace po = boost::program_options;
namespace {
void runAlgorithm (MiniMC::Model::Program& prgm, MiniMC::Algorithms::SetupOptions sopt) {
  MiniMC::Support::Sequencer<MiniMC::Model::Program> seq;
  MiniMC::Algorithms::setupForAlgorithm (seq,sopt);
  MiniMC::Savers::OptionsSave<MiniMC::Savers::Type::JSON>::Opt saveOpt {.writeTo = &std::cout};
  MiniMC::Savers::saveModel<MiniMC::Savers::Type::JSON> (prgm.shared_from_this (),saveOpt);
  
}
  

}

MiniMC::Support::ExitCodes tojson_main (MiniMC::Model::Program_ptr& prgm, std::vector<std::string>& parameters, MiniMC::Algorithms::SetupOptions& sopt) {
  po::options_description desc("Conversion Options");
  std::string input;

  
  desc.add_options()
    ("expandnondet",po::bool_switch (&sopt.expandNonDet),"Expand all non-deterministic values")
    ("splitcmps",po::bool_switch (&sopt.splitCMPS),"Split control-flow at comparisons")
	("inlinefunctions",po::value<std::size_t> (&sopt.inlinefunctions),"Inline function calls")
	
	;
  
  
  po::variables_map vm; 
  
  if (!parseOptionsAddHelp (vm,desc,parameters)) {
	return MiniMC::Support::ExitCodes::ConfigurationError;
  }
  
  runAlgorithm (*prgm,sopt);

  return MiniMC::Support::ExitCodes::AllGood;
}


static CommandRegistrar tojson_reg ("convert",tojson_main,"Convert input to JSON format");
