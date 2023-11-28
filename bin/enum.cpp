#include <boost/program_options.hpp>

#include "options.hpp"
#include "support/sequencer.hpp"
#include "host/host.hpp"
#include "algorithms/reachability/reachability.hpp"

#include "plugin.hpp"

namespace po = boost::program_options;

namespace {
  
  void addOptions (po::options_description&) {  
    
  }
}

MiniMC::Host::ExitCodes enum_main (MiniMC::Model::Program&& prgm, MiniMC::Support::Messager& messager)  {
  messager << MiniMC::Support::TInfo {"Initiating EnumStates"};
  
  auto initstate = cpa.makeInitialState(MiniMC::CPA::InitialiseDescr{
      prgm.getEntryPoints (),
      prgm.getHeapLayout (),
      prgm.getInitialiser (),
      prgm});
  
  auto goal = [](const MiniMC::CPA::AnalysisState&) {
    return false;
  };
  
  MiniMC::Algorithms::Reachability::Reachability reach {cpa.makeTransfer (prgm)};
  reach.search (messager,initstate,goal);
  
  messager << MiniMC::Support::TInfo ("Finished EnumStates");
  messager << MiniMC::Support::TInfo (MiniMC::Support::Localiser("Total Number of States %1%").format(reach.getNumberExploredStates()));
  
  return MiniMC::Host::ExitCodes::AllGood;
}

static CommandOldStyle enum_reg ("enum",enum_main,"Enumerate total number of states in CPA",addOptions);

