#include <boost/program_options.hpp>


#include "algorithms/reach.hpp"
#include "loaders/loader.hpp"

#include "plugin.hpp"

namespace po = boost::program_options;

namespace {
  
  MiniMC::Support::ExitCodes runAlgorithm (MiniMC::Model::Program& prgm,  const MiniMC::Algorithms::SetupOptions sopt, MiniMC::Algorithms::ExplicitReachability::ReachabilityResult expected ) {
	using algorithm = MiniMC::Algorithms::ExplicitReachability;
	MiniMC::Support::Sequencer<MiniMC::Model::Program> seq;
	MiniMC::Algorithms::setupForAlgorithm (seq,sopt);
	algorithm algo(typename algorithm::Options {});
	if (seq.run (prgm)) {
	  if (algo.run (prgm) == MiniMC::Algorithms::Result::Success) {
		if (algo.getAnalysisResult ().result == expected) {
		  return MiniMC::Support::ExitCodes::AllGood;
		}
		else {
		  return MiniMC::Support::ExitCodes::UnexpectedResult;
		}
	  }
	  
	}
	return MiniMC::Support::ExitCodes::ConfigurationError;
	
  }

  
  struct LocalOptions {
	MiniMC::Algorithms::ExplicitReachability::ReachabilityResult expect;	
  };
  
  LocalOptions locoptions;
  
  
  void addOptions (po::options_description& op,MiniMC::Algorithms::SetupOptions& sopt) {
	locoptions.expect = MiniMC::Algorithms::ExplicitReachability::ReachabilityResult::NoViolation;
	
	auto setExpected= [&] (int val) {
	  switch (val) {
	  case 1:
		locoptions.expect = MiniMC::Algorithms::ExplicitReachability::ReachabilityResult::AssertViolated;;
		break;
	  case 2:
		locoptions.expect = MiniMC::Algorithms::ExplicitReachability::ReachabilityResult::Inconclusive;
		break;
	  default:
		locoptions.expect = MiniMC::Algorithms::ExplicitReachability::ReachabilityResult::NoViolation;
		break;
		
	  }
	};

	po::options_description desc("MC Options");
	desc.add_options()
	  ("mc.expect",po::value<int> ()->default_value (0)->notifier (setExpected),"Set the expected verification result\n"
	   "\t 1 AssertViolation\n"
	   "\t 2 Inconclusive\n"
	   "\t 0 NoViolation\n")
	  
	  ;

	op.add(desc);
  }

  
}


MiniMC::Support::ExitCodes mc_main (MiniMC::Model::Program_ptr& prgm,   MiniMC::Algorithms::SetupOptions& sopt) {
  sopt.expandNonDet = true;  
  return runAlgorithm (*prgm,sopt,locoptions.expect);
  
}

static CommandRegistrar mc_reg ("mc",mc_main,"Check whether it is possible to reach an assert violation. Classic reachability analysis. ",addOptions);

