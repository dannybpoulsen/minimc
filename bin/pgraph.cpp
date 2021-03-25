#include <boost/program_options.hpp>
#include <string>
#include <vector>


#include "support/feedback.hpp"
#include "support/sequencer.hpp"
#include "support/host.hpp"

#include "algorithms/algorithm.hpp"
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
  auto runAlgorithm (MiniMC::Model::Program& prgm, MiniMC::Algorithms::SetupOptions sopt,bool filter) {
	using algorithm = MiniMC::Algorithms::PrintCPA<CPADef>;
	MiniMC::Support::Sequencer<MiniMC::Model::Program> seq;
	MiniMC::Algorithms::setupForAlgorithm (seq,sopt);
	algorithm algo(typename algorithm::Options {.messager = sopt.messager, . filterSatis = filter, .delayTillConverge = !filter});
	if (seq.run (prgm)) {
	  
	  auto res = algo.run (prgm);
	  if (res == MiniMC::Algorithms::Result::Success) {
		sopt.messager->message ("Writing Graph");
		algo.getAnalysisResult().graph->write ("CPA");
		sopt.messager->message ("Wrote Graph");
	  }
	  return MiniMC::Support::ExitCodes::AllGood;
	}
	return MiniMC::Support::ExitCodes::ConfigurationError;;
  }

  enum class CPAUsage {
	Location,
	Concrete,
	CVC4PathFormula
  };

  
  struct LocalOptions {
	CPAUsage CPA = CPAUsage::Location;;
	bool filter;
  };

  LocalOptions locoptions;
  
  void addOptions (po::options_description& op,MiniMC::Algorithms::SetupOptions& sopt) {
	po::options_description desc("Print Graph Options");
	bool filter;
	auto updateCPA = [&sopt] (int val) {
	switch (val) {
	case 3:
	  sopt.replacememnodet = true;
	  sopt.convergencePoints = true;
	  
	  locoptions.CPA = CPAUsage::CVC4PathFormula;
	  
	  break;
	case 2:
	  locoptions.CPA = CPAUsage::Concrete;
	  break;
	case 1:
	default:
	  locoptions.CPA = CPAUsage::Location;
	  break;
	  
	}
	};
	
  
	desc.add_options()
	  ("pgraph.cpa",po::value<int>()->default_value(1)->notifier(updateCPA), "CPA\n"
	   "\t 1: Location\n"
	   "\t 2: Concrete\n"
	   "\t 3: PathFormula With CVC4\n"
	   )
	  ("pgraph.expandnondet",po::bool_switch (&sopt.expandNonDet),"Expand all non-deterministic values")
	  ("pgraph.filtersatis",po::bool_switch (&filter),"Filter out unsatisfied states")
	  
	  ("pgraph.splitcmps",po::bool_switch (&sopt.splitCMPS),"Split control-flow at comparisons")
	  ("pgraph.convergence",boost::program_options::bool_switch(&sopt.convergencePoints),"Make sure convergencepoints only has to incoming edges")
	  
	  ;

	op.add(desc);

  }


  
}
  



MiniMC::Support::ExitCodes pgraph_main (MiniMC::Model::Program_ptr& prgm,  MiniMC::Algorithms::SetupOptions& sopt) {
  
  using CVC4Path = MiniMC::CPA::Compounds::CPADef<0,
												  MiniMC::CPA::SingleLocation::CPADef,
												  MiniMC::CPA::PathFormula::CVC4CPA
												  >;

    using CPAConcrete = MiniMC::CPA::Compounds::CPADef<0,
													MiniMC::CPA::Location::CPADef,
													MiniMC::CPA::Concrete::CPADef
												  >;
  
  MiniMC::Support::ExitCodes res;
  switch (locoptions.CPA) {
  case CPAUsage::CVC4PathFormula:
	res = runAlgorithm<CVC4Path> (*prgm,sopt,locoptions.filter);
	break;
  case CPAUsage::Concrete:
	res = runAlgorithm<CPAConcrete> (*prgm,sopt,locoptions.filter);
	break;
	
  case CPAUsage::Location:
  default:
    res = runAlgorithm<MiniMC::CPA::Location::CPADef> (*prgm,sopt,locoptions.filter);
    break;
  }

  return res;
}


static CommandRegistrar pgraph_reg ("pgraph",pgraph_main,"Generate the state graph for given CPA",addOptions);
