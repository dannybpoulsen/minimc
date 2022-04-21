#include <boost/program_options.hpp>


#include "loaders/loader.hpp"
#include "algorithms/simulationmanager.hpp"


#include "plugin.hpp"

namespace po = boost::program_options;

namespace {
  
  enum class ExpectReach {
    Reachable,
    NotReachable,
    Inconclusive
  };
  
  struct LocalOptions {
    ExpectReach expect{ExpectReach::Inconclusive};	
  };
  
  LocalOptions locoptions;
  
  
  void addOptions (po::options_description& op) {
	
    auto setExpected= [&] (int val) {
      switch (val) {
      case 1:
	locoptions.expect = ExpectReach::Reachable;
	break;
      case 2:
	locoptions.expect = ExpectReach::Inconclusive;
	break;
      default:
	locoptions.expect = ExpectReach::NotReachable;
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


MiniMC::Support::ExitCodes mc_main (MiniMC::Model::Controller& prgm,const MiniMC::CPA::CPA_ptr& cpa) {

  prgm.expandNonDet();
  
  auto& messager = MiniMC::Support::getMessager ();
  messager.message("Initiating Reachability");

  auto query = cpa->makeQuery();
  auto transfer = cpa->makeTransfer();
  auto initstate = query->makeInitialState(*prgm.getProgram ());

  auto goal = [](const MiniMC::CPA::State_ptr& state) {
    return state->assertViolated();
  };

  auto filter = [](const MiniMC::CPA::State_ptr& state) {
    return state->getConcretizer()->isFeasible() == MiniMC::CPA::Solver::Feasibility::Feasible;
  };
  
  
  
  MiniMC::CPA::State_ptr foundState = nullptr;
  MiniMC::Algorithms::SimulationManager simmanager(MiniMC::Algorithms::SimManagerOptions{
      .storer = cpa->makeStore(),
      .transfer = cpa->makeTransfer()});

  simmanager.insert(initstate);
  foundState = simmanager.reachabilitySearch({
      .filter = filter,
      .goal = goal
      
    });
  
  messager.message("Finished Reachability");
  if (foundState) {
    MiniMC::Support::getMessager ().message (MiniMC::Support::Localiser ("Found Violation").format ());
    if (locoptions.expect == ExpectReach::Reachable)
      return MiniMC::Support::ExitCodes::AllGood;
    else
      return MiniMC::Support::ExitCodes::UnexpectedResult;
  }
  else {
    
    MiniMC::Support::getMessager ().message (MiniMC::Support::Localiser ("No violation found").format ());
    if (locoptions.expect == ExpectReach::Reachable)
      return MiniMC::Support::ExitCodes::UnexpectedResult;
    else
      return MiniMC::Support::ExitCodes::AllGood;
  }
}

static CommandRegistrar mc_reg ("mc",mc_main,"Check whether it is possible to reach an assert violation. Classic reachability analysis. ",addOptions);


