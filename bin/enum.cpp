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

MiniMC::Host::ExitCodes enum_main (MiniMC::Model::Controller& controller, const MiniMC::CPA::AnalysisBuilder& cpa)  {
  MiniMC::Support::Messager messager;
  messager.message("Initiating EnumStates");
  
  auto& prgm = *controller.getProgram ();
  auto initstate = cpa.makeInitialState(MiniMC::CPA::InitialiseDescr{
      prgm.getEntryPoints (),
      prgm.getHeapLayout (),
      prgm.getInitialiser (),
      prgm});
  
  auto goal = [](const MiniMC::CPA::AnalysisState&) {
    return false;
  };
  
  auto notify = [&messager](auto& t) {messager.message<MiniMC::Support::Severity::Progress> (t);};
  MiniMC::Algorithms::Reachability::Reachability reach {cpa.makeTransfer (prgm)};
  reach.getPWProgresMeasure ().listen (notify);
  reach.search (initstate,goal);

  messager.message("Finished EnumStates");
  messager.message(MiniMC::Support::Localiser("Total Number of States %1%").format(reach.getPWProgresMeasure ().get().passed));
  
  return MiniMC::Host::ExitCodes::AllGood;
}

static CommandRegistrar enum_reg ("enum",enum_main,"Enumerate total number of states in CPA",addOptions);

