#include <boost/program_options.hpp>

#include "support/feedback.hpp"
#include "support/sequencer.hpp"
#include "support/host.hpp"
#include "algorithms/enumstates.hpp"
#include "model/modifications/rremoveretsentry.hpp"
#include "model/modifications/replacememnondet.hpp"
#include "cpa/location.hpp"
#ifdef MINIMC_SYMBOLIC

#include "cpa/pathformula.hpp"
#endif
#include "cpa/compound.hpp"


#include "loaders/loader.hpp"

#include "plugin.hpp"

namespace po = boost::program_options;

namespace {

  auto runAlgorithm (MiniMC::Model::Program& prgm, const MiniMC::Algorithms::SetupOptions sopt, MiniMC::CPA::CPA_ptr cpa ) {
    MiniMC::Support::Sequencer<MiniMC::Model::Program> seq;
    MiniMC::Algorithms::setupForAlgorithm (seq,sopt);
    MiniMC::Algorithms::EnumStates algo(MiniMC::Algorithms::EnumStates::Options {.cpa = cpa});
    if (seq.run (prgm)) {
      algo.run (prgm);
      return MiniMC::Support::ExitCodes::AllGood;
    }
    return MiniMC::Support::ExitCodes::ConfigurationError;
  }

  enum class CPAUsage {
	Location,
#ifdef MINIMC_SYMBOLIC
	PathFormula
#endif
  };
  
  struct LocalOptions {
    CPAUsage CPA = CPAUsage::Location;
  };

  LocalOptions locoptions;
  
  void addOptions (po::options_description& op,MiniMC::Algorithms::SetupOptions& sopt) {
	po::options_description desc("Enum Options");
	auto updateCPA = [] (int val) {
	  switch (val) {
#ifdef MINIMC_SYMBOLIC
	  case 3:
		locoptions.CPA = CPAUsage::PathFormula;
		break;
#endif
	  case 1:
	  default:
		locoptions.CPA = CPAUsage::Location;
		break;
		
	  }
	};
	
	desc.add_options()
	  ("enum.cpa,c",po::value<int>()->default_value(1)->notifier(updateCPA), "CPA\n"
	   "\t 1: Location\n"
#ifdef MINIMC_SYMBOLIC
	   "\t 3: PathFormula With CVC4\n"
#endif
	   );
    
	op.add(desc);
  
  }

  

}

MiniMC::Support::ExitCodes enum_main (MiniMC::Model::Program_ptr& prgm,   MiniMC::Algorithms::SetupOptions& sopt)  {
	
  MiniMC::CPA::CPA_ptr cpa = nullptr;
  switch (locoptions.CPA) {
#ifdef MINIMC_SYMBOLIC
  case CPAUsage::PathFormula:
    cpa = std::make_shared<MiniMC::CPA::Compounds::CPA> (std::initializer_list<MiniMC::CPA::CPA_ptr>({
	  std::make_shared<MiniMC::CPA::Location::CPA> (),
	  std::make_shared<MiniMC::CPA::PathFormula::CPA> ()}));
    break;
    
#endif
  case CPAUsage::Location:
  default:
    cpa = std::make_shared<MiniMC::CPA::Location::CPA> ();
    //res = runAlgorithm<MiniMC::CPA::Location::CPADef> (*prgm,sopt);
    break;
  }
  assert (cpa);
  return runAlgorithm (*prgm,sopt,cpa);;
  
}

static CommandRegistrar enum_reg ("enum",enum_main,"Enumerate total number of states in CPA",addOptions);

