#include <boost/program_options.hpp>


#include "loaders/loader.hpp"
#include "algorithms/reachability/reachability.hpp"


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
    MiniMC::Algorithms::Reachability::SearchStrategy search_strat{MiniMC::Algorithms::Reachability::SearchStrategy::DFS};
  };
  
  LocalOptions locoptions;
  
  
  void addOptions (po::options_description& op) {
	
    auto setExpected= [] (int val) {
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

    auto setSearchStrategy= [] (const std::string val) {
      if (val == "DFS") {
	locoptions.search_strat = MiniMC::Algorithms::Reachability::SearchStrategy::DFS;
      }

      else if (val == "BFS") {
	locoptions.search_strat = MiniMC::Algorithms::Reachability::SearchStrategy::BFS;      
      }
    };
    
    po::options_description desc("MC Options");
    desc.add_options()
      ("mc.expect",po::value<int> ()->default_value (0)->notifier (setExpected),"Set the expected verification result\n"
       "\t 1 AssertViolation\n"
       "\t 2 Inconclusive\n"
       "\t 0 NoViolation\n")
      ("mc.strategy",po::value<std::string> ()->default_value ({"DFS"})->notifier (setSearchStrategy),"Select search strategy\n"
       "\t BFS\n"
       "\t DFS\n"
       )
      ;
       
    
    op.add(desc);
  }

  
}


MiniMC::Host::ExitCodes mc_main (MiniMC::Model::Program&& prgm, const MiniMC::CPA::AnalysisBuilder& cpa, MiniMC::Support::Messager& messager) {    
  auto initstate = cpa.makeInitialState({prgm.getEntryPoints (),
      prgm.getHeapLayout (),
      prgm.getInitialiser (),
      prgm});

  auto goal = [](const MiniMC::CPA::AnalysisState& state) {
    auto& locationstate = state.getCFAState ().getLocationState ();
    auto procs = locationstate.nbOfProcesses ();
    
    for (std::size_t i = 0; i < procs; ++i) {
      if (locationstate.getLocation (i).getInfo ().getFlags ().isSet (MiniMC::Model::Attributes::AssertViolated))
	return true;
    }
    
    return false;
  };
  
  
  messager << MiniMC::Support::TInfo<std::string> {"Initiating Reachability"};
  MiniMC::Algorithms::Reachability::Reachability reach {cpa.makeTransfer(prgm)};
  reach.setSearchStrategy (locoptions.search_strat);
    
  auto verdict = reach.search (messager,initstate,goal);
  messager << MiniMC::Support::TInfo<std::string> {"Finished Reachability"};
  
  
  if (verdict == MiniMC::Algorithms::Reachability::Verdict::Found) {
    messager << MiniMC::Support::TInfo<std::string> {"Found Violation"};
    MiniMC::CPA::StateOutputter{prgm}.output (reach.foundState(),std::cerr) << std::endl;
    
    if (locoptions.expect == ExpectReach::Reachable)
      return MiniMC::Host::ExitCodes::AllGood;
    else
      return MiniMC::Host::ExitCodes::UnexpectedResult;
  }
  
  if (verdict == MiniMC::Algorithms::Reachability::Verdict::NotFound) {
    messager <<  MiniMC::Support::TInfo<std::string> {"No violation found"};
    if (locoptions.expect == ExpectReach::Reachable)
      return MiniMC::Host::ExitCodes::UnexpectedResult;
    else
      return MiniMC::Host::ExitCodes::AllGood;
  }
  
  
  
  return MiniMC::Host::ExitCodes::AllGood;
}

static CommandRegistrar mc_reg ("mc",mc_main,"Check whether it is possible to reach an assert violation. Classic reachability analysis. ",addOptions);


