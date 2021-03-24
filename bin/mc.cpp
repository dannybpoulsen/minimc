#include <boost/program_options.hpp>


#include "algorithms/reach.hpp"
#include "loaders/loader.hpp"

#include "plugin.hpp"

namespace po = boost::program_options;

MiniMC::Support::ExitCodes runAlgorithm (MiniMC::Model::Program& prgm,  const MiniMC::Algorithms::SetupOptions sopt, MiniMC::Algorithms::ExplicitReachability::ReachabilityResult expected ) {
  using algorithm = MiniMC::Algorithms::ExplicitReachability;
  auto mess = MiniMC::Support::makeMessager (MiniMC::Support::MessagerType::Terminal);
  MiniMC::Support::Sequencer<MiniMC::Model::Program> seq;
  MiniMC::Algorithms::setupForAlgorithm (seq,sopt);
  algorithm algo(typename algorithm::Options {.messager = sopt.messager});
  if (seq.run (prgm)) {
	if (algo.run (prgm) == MiniMC::Algorithms::Result::Success) {
	  if (algo.getAnalysisResult ().result == expected) {
		MiniMC::Support::ExitCodes::AllGood;
	  }
	  else {
		MiniMC::Support::ExitCodes::UnexpectedResult;
	  }
	}
	
  }
  return MiniMC::Support::ExitCodes::ConfigurationError;
	
}


MiniMC::Support::ExitCodes mc_main (MiniMC::Model::Program_ptr& prgm, std::vector<std::string>& parameters,  MiniMC::Algorithms::SetupOptions& sopt) {
  sopt.expandNonDet = true;

  MiniMC::Algorithms::ExplicitReachability::ReachabilityResult expect = MiniMC::Algorithms::ExplicitReachability::ReachabilityResult::NoViolation;
  po::options_description desc("Basic SMC Options");

  auto setExpected= [&] (int val) {
				   switch (val) {
				   case 1:
					 expect = MiniMC::Algorithms::ExplicitReachability::ReachabilityResult::AssertViolated;;
					 break;
				   case 2:
					 expect = MiniMC::Algorithms::ExplicitReachability::ReachabilityResult::Inconclusive;
					 break;
				   default:
					 expect = MiniMC::Algorithms::ExplicitReachability::ReachabilityResult::NoViolation;
					 break;
					 
				   }
				 };
  
  desc.add_options()
    ("expect",po::value<int> ()->default_value (0)->notifier (setExpected),"Set the expected verification result\nx"
    "\t 1 AssertViolation\n"
	 "\t 2 Inconclusive\n"
	 "\t 0 NoViolation\n")
	
    ;

  
  return runAlgorithm (*prgm,sopt,expect);
  
}

static CommandRegistrar mc_reg ("mc",mc_main,"Check whether it is possible to reach an assert violation. Classic reachability analysis. ");

