#include <boost/program_options.hpp>
#include <string>
#include <vector>


#include "support/feedback.hpp"
#include "support/sequencer.hpp"
#include "algorithms/printgraph.hpp"


#include "cpa/location.hpp"
#include "cpa/pathformula.hpp"
#include "cpa/concrete.hpp"

#include "cpa/compound.hpp"


#include "loaders/loader.hpp"

#include "plugin.hpp"

namespace po = boost::program_options;
namespace {
template<class CPADef>
auto runAlgorithm (MiniMC::Model::Program& prgm, MiniMC::Algorithms::SetupOptions sopt) {
  using algorithm = MiniMC::Algorithms::PrintCPA<CPADef>;
  MiniMC::Support::Sequencer<MiniMC::Model::Program> seq;
  MiniMC::Algorithms::setupForAlgorithm<algorithm> (seq,sopt);
  algorithm algo(typename algorithm::Options {.messager = sopt.messager});
  return MiniMC::Algorithms::runSetup (seq,algo,prgm);
}

enum class CPAUsage {
					 Location,
					 Concrete,
					 CVC4PathFormula
};

}

int pgraph_main (MiniMC::Model::Program_ptr& prgm, std::vector<std::string>& parameters, MiniMC::Algorithms::SetupOptions& sopt) {
  CPAUsage CPA = CPAUsage::Location;
  po::options_description desc("Print Graph Options");
  std::string input;
  auto updateCPA = [&CPA] (int val) {
					 switch (val) {
					 case 3:
					   CPA = CPAUsage::CVC4PathFormula;
					   break;
					 case 2:
					   CPA = CPAUsage::Concrete;
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
     "\t 3: PathFormula With CVC4\n"
     )
    ("expandnondet",po::bool_switch (&sopt.expandNonDet),"Expand all non-deterministic values")
    ("splitcmps",po::bool_switch (&sopt.splitCMPS),"Split control-flow at comparisons")
	("inlinefunctions",po::value<std::size_t> (&sopt.inlinefunctions),"Inline function calls")
	("unrollloops",po::value<std::size_t> (&sopt.unrollLoops),"Unroll Loops")
	("convergence",boost::program_options::bool_switch(&sopt.convergencePoints),"Make sure convergencepoints only has to incoming edges")
    
	;
  
  
  po::variables_map vm; 
  
  if (!parseOptionsAddHelp (vm,desc,parameters)) {
	return -1;
  }
  
  using CVC4Path = MiniMC::CPA::Compounds::CPADef<0,
												  MiniMC::CPA::SingleLocation::CPADef,
												  MiniMC::CPA::PathFormula::CVC4CPA
												  >;

    using CPAConcrete = MiniMC::CPA::Compounds::CPADef<0,
													MiniMC::CPA::Location::CPADef,
													MiniMC::CPA::Concrete::CPADef
												  >;
  
  MiniMC::Algorithms::Result res;
  switch (CPA) {
  case CPAUsage::CVC4PathFormula:
	res = runAlgorithm<CVC4Path> (*prgm,sopt);
	break;
  case CPAUsage::Concrete:
	res = runAlgorithm<CPAConcrete> (*prgm,sopt);
	break;
	
  case CPAUsage::Location:
  default:
    res = runAlgorithm<MiniMC::CPA::Location::CPADef> (*prgm,sopt);
    break;
  }

  return static_cast<int> (res);
}


static CommandRegistrar pgraph_reg ("pgraph",pgraph_main,"Generate the state graph for given CPA");
