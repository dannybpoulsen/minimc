#include <boost/program_options.hpp>

#include "options.hpp"
#include "support/feedback.hpp"
#include "support/sequencer.hpp"
#include "support/host.hpp"
#include "algorithms/simulationmanager.hpp"

#include "plugin.hpp"

namespace po = boost::program_options;

namespace {
  
  void addOptions (po::options_description&) {  
    
  }
}

MiniMC::Support::ExitCodes enum_main (MiniMC::Model::Controller& prgm, const MiniMC::CPA::CPA_ptr& cpa)  {
  auto& messager = MiniMC::Support::getMessager ();
  messager.message("Initiating EnumStates");

  auto progresser = messager.makeProgresser();
  auto query = cpa->makeQuery();
  auto initstate = query->makeInitialState(*prgm.getProgram ());
  
  MiniMC::Algorithms::SimulationManager simmanager(MiniMC::Algorithms::SimManagerOptions{
      .storer = cpa->makeStore(),
      .transfer = cpa->makeTransfer()});
  simmanager.insert(initstate);

  simmanager.reachabilitySearch({
      .filter = [](const MiniMC::CPA::State_ptr& state) {
	auto res = state->getConcretizer()->isFeasible();
	return (res == MiniMC::CPA::Solver::Feasibility::Feasible) ||
	  (res == MiniMC::CPA::Solver::Feasibility::Unknown)
	  ;
      }}
    );
  
  messager.message("Finished EnumStates");
  messager.message(MiniMC::Support::Localiser("Total Number of States %1%").format(simmanager.getPSize()));
  
  return MiniMC::Support::ExitCodes::AllGood;
}

static CommandRegistrar enum_reg ("enum",enum_main,"Enumerate total number of states in CPA",addOptions);

