#include <boost/program_options.hpp>
#include <string>
#include <vector>


#include "support/feedback.hpp"
#include "support/sequencer.hpp"
#include "algorithms/printgraph.hpp"
#include "savers/savers.hpp"

#include "cpa/location.hpp"
#include "cpa/concrete_no_mem.hpp"
#include "cpa/pathformula.hpp"
#include "cpa/compound.hpp"

#include "plugin.hpp"

namespace po = boost::program_options;
namespace {
template<class CPADef>
void runAlgorithm (MiniMC::Model::Program& prgm, MiniMC::Algorithms::SetupOptions sopt) {
  using algorithm = MiniMC::Algorithms::PrintCPA<CPADef>;
  MiniMC::Support::Sequencer<MiniMC::Model::Program> seq;
  MiniMC::Algorithms::setupForAlgorithm<algorithm> (seq,sopt);
  algorithm algo(typename algorithm::Options {.messager = sopt.messager});
  MiniMC::Savers::OptionsSave<MiniMC::Savers::Type::JSON>::Opt saveOpt {.writeTo = &std::cout};
  MiniMC::Savers::saveModel<MiniMC::Savers::Type::JSON> (prgm.shared_from_this (),saveOpt);
  
}
  
enum class CPAUsage {
					 Location,
					 LocationExplicit,
					 CVC4PathFormula
};

}

int tojson_main (MiniMC::Model::Program_ptr& prgm, std::vector<std::string>& parameters, MiniMC::Algorithms::SetupOptions& sopt) {
  CPAUsage CPA = CPAUsage::Location;
  po::options_description desc("Conversion Options");
  std::string input;
  auto updateCPA = [&CPA] (int val) {
					 switch (val) {
					 case 2:
					   CPA = CPAUsage::LocationExplicit;
					   break;
					 case 3:
					   CPA = CPAUsage::CVC4PathFormula;
					   break;
					 case 1:
					 default:
					   CPA = CPAUsage::Location;
					   break;
					 
					 }
				   };

  
  desc.add_options()
    ("cpa,c",po::value<int>()->default_value(1)->notifier(updateCPA), "CPA\n"
     "\t 1: Location\n"
     "\t 2: Location and explicit stack-variable\n"
	 "\t 3: PathFormula With CVC4\n"
     )
    ("expandnondet",po::bool_switch (&sopt.expandNonDet),"Expand all non-deterministic values")
    ("splitcmps",po::bool_switch (&sopt.splitCMPS),"Split control-flow at comparisons")
	("inlinefunctions",po::value<std::size_t> (&sopt.inlinefunctions),"Inline function calls")
	
	;
  
  
  po::variables_map vm; 
  
  if (!parseOptionsAddHelp (vm,desc,parameters)) {
	return -1;
  }
  
  using LocExpliStack = MiniMC::CPA::Compounds::CPADef<0,
													   MiniMC::CPA::Location::CPADef,
													   MiniMC::CPA::ConcreteNoMem::CPADef
													   >;
  using CVC4Path = MiniMC::CPA::Compounds::CPADef<0,
												  MiniMC::CPA::Location::CPADef,
												  MiniMC::CPA::PathFormula::CVC4CPA
												  >;
  switch (CPA) {
  case CPAUsage::CVC4PathFormula:
	runAlgorithm<CVC4Path> (*prgm,sopt);
	break;
  case CPAUsage::LocationExplicit:
    runAlgorithm<LocExpliStack> (*prgm,sopt);
    break;
  case CPAUsage::Location:
  default:
	runAlgorithm<MiniMC::CPA::Location::CPADef> (*prgm,sopt);
    break;
  }

  return static_cast<int> (0);
}


static CommandRegistrar tojson_reg ("convert",tojson_main,"Convert input to JSON format");
