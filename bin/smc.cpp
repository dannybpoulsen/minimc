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
	MiniMC::Algorithms::setupForAlgorithm<algorithm> (seq,sopt);	
	algorithm algo(typename algorithm::Options {opt});
	return MiniMC::Algorithms::runSetup (seq,algo,prgm);
  }

  enum class Algo {
				   Fixed,
				   Clopper
  };
}
  
int smc_main (MiniMC::Model::Program_ptr& prgm, std::vector<std::string>& parameters, const MiniMC::Algorithms::SetupOptions& sopt) {
  MiniMC::Algorithms::ProbaChecker<MiniMC::Support::Statistical::ClopperPearson>::Options clopperOpt {.messager = sopt.messager};
	MiniMC::Algorithms::ProbaChecker<MiniMC::Support::Statistical::FixedEffort>::Options fixedOpt {.messager = sopt.messager};
	std::size_t length;
	Algo algo = Algo::Fixed;
	std::string input; 
	po::options_description desc("Basic SMC Options");

	auto setAlgo = [&] (int val) {
				   switch (val) {
				   case 1:
					 algo = Algo::Fixed;
					 break;
				   case 2:
					 algo = Algo::Clopper;
					 break;
				   }
				 };
  
  desc.add_options()
    ("algorithm",po::value<int> ()->default_value (1)->notifier (setAlgo),"Algorithm\n"
    "\t 1 Fixed Effort\n"
    "\t 1 Clopper Pearson\n"
     )
    ("length",po::value<std::size_t> (&length),"Length")
    ;
    
  
  po::options_description fixed ("Fixed Effort Options");
  fixed.add_options ()
    ("samples",po::value<std::size_t> (&fixedOpt.smcoptions.effort),"Samples")
	("falpha",po::value<MiniMC::proba_t> (&fixedOpt.smcoptions.alpha),"Significance");
	
  
  po::options_description clopper ("Clopper Options");
  clopper.add_options ()
    ("width",po::value<MiniMC::proba_t> (&clopperOpt.smcoptions.width),"Desired Width")
	("calpha",po::value<MiniMC::proba_t> (&clopperOpt.smcoptions.alpha),"Significance")
	
	;
  
  po::options_description cmdline;
  cmdline.add(desc).
    add(clopper).
    add(fixed);
  
  
  po::variables_map vm; 

  if (!parseOptionsAddHelp (vm,cmdline,parameters)) {
	return -1;
  }
 

  clopperOpt.len = length;
  fixedOpt.len = length;
  MiniMC::Algorithms::Result res;
  switch (algo) {
  case Algo::Fixed:
    res = runAlgorithm<MiniMC::Support::Statistical::FixedEffort> (*prgm,sopt,fixedOpt);
    break;
  case Algo::Clopper:
    res = runAlgorithm<MiniMC::Support::Statistical::ClopperPearson> (*prgm,sopt,clopperOpt);
    break;
  }
  return static_cast<int> (res);
}


static CommandRegistrar smc_reg ("smc",smc_main,"Determine the probability of reaching an assert violation. Non-deterministic choices is converted to uniform choices.");
