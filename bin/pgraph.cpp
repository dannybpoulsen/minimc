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
  
  struct LocalOptions {
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
  
	desc.add_options()
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
  MiniMC::CPA::CPA_ptr cpa = createUserDefinedCPA (CPASelector::Location);
  
  return runAlgorithm (*prgm,sopt,locoptions.filter,cpa);
    
}


static CommandRegistrar pgraph_reg ("pgraph",pgraph_main,"Generate the state graph for given CPA",addOptions);
