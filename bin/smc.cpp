#include <boost/program_options.hpp>

#include "support/feedback.hpp"
#include "support/sequencer.hpp"
#include "support/statistical/fixed_effort.hpp"
#include "algorithms/pprintgraph.hpp"

#include "loaders/loader.hpp"

#include "plugin.hpp"

namespace po = boost::program_options;

namespace {
  template<class F>
  auto runAlgorithm (MiniMC::Model::Program& prgm,  MiniMC::Algorithms::SetupOptions sopt, const  typename MiniMC::Algorithms::ProbaChecker<F>::Options& opt) {
	using algorithm = MiniMC::Algorithms::ProbaChecker<F>;
	MiniMC::Support::Sequencer<MiniMC::Model::Program> seq;
	
	MiniMC::Algorithms::setupForAlgorithm (seq,sopt);	
	algorithm algo(typename algorithm::Options {opt});
	if (seq.run (prgm)) {
	  algo.run (prgm);
	  return MiniMC::Support::ExitCodes::AllGood;
	}
	return MiniMC::Support::ExitCodes::ConfigurationError;;
  }

  enum class Algo {
	Fixed,
	Clopper
  };

  struct LocalOptions {
	std::size_t length;
	Algo algo = Algo::Fixed;
	MiniMC::Algorithms::ProbaChecker<MiniMC::Support::Statistical::ClopperPearson>::Options clopperOpt;
	MiniMC::Algorithms::ProbaChecker<MiniMC::Support::Statistical::FixedEffort>::Options fixedOpt;
  };

  LocalOptions locoptions;

  void addOptions (po::options_description& op,MiniMC::Algorithms::SetupOptions& sopt) {
	po::options_description desc("Basic SMC Options");
	
	auto setAlgo = [&] (int val) {
	  switch (val) {
	  case 1:
		locoptions.algo = Algo::Fixed;
		break;
	  case 2:
		locoptions.algo = Algo::Clopper;
		break;
	  }
	};
	
	desc.add_options()
	  ("smc.algorithm",po::value<int> ()->default_value (1)->notifier (setAlgo),"Algorithm\n"
	   "\t 1 Fixed Effort\n"
	   "\t 1 Clopper Pearson\n"
	   )
	  ("smc.length",po::value<std::size_t> (&locoptions.length),"Length")
	  ;
    
  
	po::options_description fixed ("Fixed Effort Options");
	fixed.add_options ()
	  ("smc.fixed.samples",po::value<std::size_t> (&locoptions.fixedOpt.smcoptions.effort),"Samples")
	  ("smc.fixed.alpha",po::value<MiniMC::proba_t> (&locoptions.fixedOpt.smcoptions.alpha),"Significance");
	
  
	po::options_description clopper ("Clopper Options");
	clopper.add_options ()
	  ("smc.clopper.width",po::value<MiniMC::proba_t> (&locoptions.clopperOpt.smcoptions.width),"Desired Width")
	  ("smc.clopper.alpha",po::value<MiniMC::proba_t> (&locoptions.clopperOpt.smcoptions.alpha),"Significance")
	  
	  ;
	
  po::options_description cmdline;
  cmdline.add(desc).
    add(clopper).
    add(fixed);

  op.add(cmdline);
  
  }
  
}
  
MiniMC::Support::ExitCodes smc_main (MiniMC::Model::Program_ptr& prgm,   MiniMC::Algorithms::SetupOptions& sopt) {
  sopt.replaceNonDetUniform = true;

  locoptions.clopperOpt.len = locoptions.length;
  locoptions.fixedOpt.len = locoptions.length;
  MiniMC::Support::ExitCodes res;
  switch (locoptions.algo) {
  case Algo::Fixed:
    res = runAlgorithm<MiniMC::Support::Statistical::FixedEffort> (*prgm,sopt,locoptions.fixedOpt);
    break;
  case Algo::Clopper:
    res = runAlgorithm<MiniMC::Support::Statistical::ClopperPearson> (*prgm,sopt,locoptions.clopperOpt);
    break;
  }
  return res;
}


static CommandRegistrar smc_reg ("smc",smc_main,"Determine the probability of reaching an assert violation. Non-deterministic choices is converted to uniform choices.",addOptions);
