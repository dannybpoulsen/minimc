#include <boost/program_options.hpp>

#include "support/feedback.hpp"
#include "support/sequencer.hpp"
#include "support/host.hpp"
#include "algorithms/enumstates.hpp"
#include "model/modifications/rremoveretsentry.hpp"
#include "model/modifications/replacememnondet.hpp"
#include "cpa/location.hpp"
#include "cpa/pathformula.hpp"
#include "cpa/compound.hpp"


#include "loaders/loader.hpp"

#include "plugin.hpp"

namespace po = boost::program_options;

namespace {

  template<class CPADef>
  auto runAlgorithm (MiniMC::Model::Program& prgm, const MiniMC::Algorithms::SetupOptions sopt) {
	using algorithm = MiniMC::Algorithms::EnumStates<CPADef>;
	MiniMC::Support::Sequencer<MiniMC::Model::Program> seq;
	MiniMC::Algorithms::setupForAlgorithm (seq,sopt);
	algorithm algo(typename algorithm::Options {});
	if (seq.run (prgm)) {
	  algo.run (prgm);
	  return MiniMC::Support::ExitCodes::AllGood;
	}
	return MiniMC::Support::ExitCodes::ConfigurationError;
  }

  enum class CPAUsage {
	Location,
	CVC4PathFormula
  };
  
  struct LocalOptions {
	CPAUsage CPA = CPAUsage::Location;
  };

  LocalOptions locoptions;
  
  void addOptions (po::options_description& op,MiniMC::Algorithms::SetupOptions& sopt) {
	po::options_description desc("Enum Options");
	auto updateCPA = [] (int val) {
	  switch (val) {
	  case 3:
		locoptions.CPA = CPAUsage::CVC4PathFormula;
		break;
	  case 1:
	  default:
		locoptions.CPA = CPAUsage::Location;
		break;
		
	  }
	};
	
	desc.add_options()
	  ("enum.cpa,c",po::value<int>()->default_value(1)->notifier(updateCPA), "CPA\n"
	   "\t 1: Location\n"
	   "\t 3: PathFormula With CVC4\n"
	   );
    
	op.add(desc);
  
  }

  

}

MiniMC::Support::ExitCodes enum_main (MiniMC::Model::Program_ptr& prgm,   MiniMC::Algorithms::SetupOptions& sopt)  {
	
  using CVC4Path = MiniMC::CPA::Compounds::CPADef<0,
												  MiniMC::CPA::Location::CPADef,
												  MiniMC::CPA::PathFormula::CVC4CPA
												  >;
  MiniMC::Support::ExitCodes res;
  switch (locoptions.CPA) {
  case CPAUsage::CVC4PathFormula:
	res = runAlgorithm<CVC4Path> (*prgm,sopt);
	break;
  case CPAUsage::Location:
  default:
    res = runAlgorithm<MiniMC::CPA::Location::CPADef> (*prgm,sopt);
    break;
  }
  
  return res;
  
}

static CommandRegistrar enum_reg ("enum",enum_main,"Enumerate total number of states in CPA",addOptions);

