#include <boost/program_options.hpp>
#include <string>
#include <vector>


#include "support/feedback.hpp"
#include "support/sequencer.hpp"
#include "support/host.hpp"

#include "algorithms/algorithm.hpp"
#include "algorithms/printgraph.hpp"


#include "cpa/location.hpp"
#ifdef MINIMC_SYMBOLIC
#include "cpa/pathformula.hpp"
#endif
#include "cpa/concrete.hpp"
#include "cpa/compound.hpp"


#include "loaders/loader.hpp"

#include "plugin.hpp"

namespace po = boost::program_options;
namespace {
    enum class CPAUsage {
	Location,
	Concrete,
#ifdef MINIMC_SYMBOLIC
	PathFormula
#endif
	};

  
  struct LocalOptions {
	CPAUsage CPA = CPAUsage::Location;;
	std::string outputname;
	bool filter;
  };

  LocalOptions locoptions;

  
  auto runAlgorithm (MiniMC::Model::Program& prgm, MiniMC::Algorithms::SetupOptions sopt,bool filter,MiniMC::CPA::CPA_ptr cpa) {
        MiniMC::Support::Sequencer<MiniMC::Model::Program> seq;
	MiniMC::Algorithms::setupForAlgorithm (seq,sopt);
	MiniMC::Algorithms::PrintCPA algo(MiniMC::Algorithms::PrintCPA::Options {.filterSatis = filter, .delayTillConverge = !filter,.cpa = cpa});
	if (seq.run (prgm)) {
	  
	  auto res = algo.run (prgm);
	  if (res == MiniMC::Algorithms::Result::Success) {
	    MiniMC::Support::getMessager ().message (MiniMC::Support::Localiser ("Outputting graph to '%1%.dot'").format (locoptions.outputname));
		algo.getAnalysisResult().graph->write (locoptions.outputname);
	  }
	  return MiniMC::Support::ExitCodes::AllGood;
	}
	return MiniMC::Support::ExitCodes::ConfigurationError;;
  }

  
  void addOptions (po::options_description& op,MiniMC::Algorithms::SetupOptions& sopt) {
	po::options_description desc("Print Graph Options");
	auto updateCPA = [&sopt] (int val) {
	switch (val) {
#ifdef MINIMC_SYMBOLIC
	case 3:
	  sopt.replacememnodet = true;
	  sopt.convergencePoints = true;
	  
	  locoptions.CPA = CPAUsage::PathFormula;
	  
	  break;
#endif
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
#ifdef MINIMC_SYMBOLIC	   
	   "\t 3: PathFormula\n"
#endif
	   )
	  ("pgraph.expandnondet",po::bool_switch (&sopt.expandNonDet),"Expand all non-deterministic values")
	  ("pgraph.filtersatis",po::bool_switch (&locoptions.filter),"Filter out unsatisfied states")
	  
	  ("pgraph.splitcmps",po::bool_switch (&sopt.splitCMPS),"Split control-flow at comparisons")
	  ("pgraph.convergence",boost::program_options::bool_switch(&sopt.convergencePoints),"Make sure convergencepoints only has to incoming edges")
	  ("pgraph.output",po::value<std::string>(&locoptions.outputname)->default_value("CPA"),"output filename")
	  
	  ;

	op.add(desc);

  }


  
}
  



MiniMC::Support::ExitCodes pgraph_main (MiniMC::Model::Program_ptr& prgm,  MiniMC::Algorithms::SetupOptions& sopt) {
  
  
  
  MiniMC::Support::ExitCodes res;
  MiniMC::CPA::CPA_ptr cpa = nullptr;
  switch (locoptions.CPA) {
#ifdef MINIMC_SYMBOLIC
  case CPAUsage::PathFormula:
    cpa = std::make_shared<MiniMC::CPA::Compounds::CPA> (std::initializer_list<MiniMC::CPA::CPA_ptr>({
	  std::make_shared<MiniMC::CPA::Location::CPA> (),
	  std::make_shared<MiniMC::CPA::PathFormula::CPA> ()}));
    break;
    
#endif
  case CPAUsage::Concrete:
    cpa = std::make_shared<MiniMC::CPA::Compounds::CPA> (std::initializer_list<MiniMC::CPA::CPA_ptr>({
	std::make_shared<MiniMC::CPA::Location::CPA> (),
	std::make_shared<MiniMC::CPA::Concrete::CPA> ()}));
    break;
    
  case CPAUsage::Location:
  default:
    cpa = std::make_shared<MiniMC::CPA::Location::CPA> ();
    //res = runAlgorithm<MiniMC::CPA::Location::CPADef> (*prgm,sopt,locoptions.filter);
    break;
  }
  return runAlgorithm (*prgm,sopt,locoptions.filter,cpa);
    
}


static CommandRegistrar pgraph_reg ("pgraph",pgraph_main,"Generate the state graph for given CPA",addOptions);
