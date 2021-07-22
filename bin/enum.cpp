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

  
  void addOptions (po::options_description& op,MiniMC::Algorithms::SetupOptions& sopt) {  
    
  }
}
MiniMC::Support::ExitCodes enum_main (MiniMC::Model::Program_ptr& prgm,   MiniMC::Algorithms::SetupOptions& sopt)  {
	
  MiniMC::CPA::CPA_ptr cpa = createUserDefinedCPA (CPASelector::Location);
 
  assert (cpa);
  return runAlgorithm (*prgm,sopt,cpa);;
  
}

  static CommandRegistrar enum_reg ("enum",enum_main,"Enumerate total number of states in CPA",addOptions);

