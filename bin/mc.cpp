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


MiniMC::Support::ExitCodes mc_main (MiniMC::Model::Controller& controller, const MiniMC::CPA::CPA_ptr& cpa) {

  controller.expandNonDet();
  controller.typecheck ();
  
  MiniMC::Support::Messager messager{};
  messager.message("Initiating Reachability");
  
  auto query = cpa->makeQuery();
  auto joiner = cpa->makeJoin ();
  auto& prgm = *controller.getProgram ();
  auto transfer = cpa->makeTransfer(prgm);
  
  auto initstate = query->makeInitialState({prgm.getEntryPoints (),
	prgm.getHeapLayout (),
	prgm.getInitialiser (),
	prgm});

  auto goal = [](const MiniMC::CPA::State_ptr& state) {
    return state->getLocationState().assertViolated();
  };
  
  
  auto notify = [&messager](auto& t) {messager.message<MiniMC::Support::Severity::Progress> (t);};
  MiniMC::Algorithms::Reachability::Reachability reach {transfer,joiner};
  reach.getPWProgresMeasure ().listen (notify);
  auto verdict = reach.search (initstate,goal);
  messager.message("Finished Reachability");
  
  
  if (verdict == MiniMC::Algorithms::Reachability::Verdict::Found) {
    MiniMC::Support::getMessager ().message (MiniMC::Support::Localiser ("Found Violation").format ());

    auto foundState = reach.foundState ();
  
    
    if (locoptions.expect == ExpectReach::Reachable)
      return MiniMC::Support::ExitCodes::AllGood;
    else
      return MiniMC::Support::ExitCodes::UnexpectedResult;
  }

  if (verdict == MiniMC::Algorithms::Reachability::Verdict::NotFound) {
    MiniMC::Support::getMessager ().message (MiniMC::Support::Localiser ("No violation found").format ());
    if (locoptions.expect == ExpectReach::Reachable)
      return MiniMC::Support::ExitCodes::UnexpectedResult;
    else
      return MiniMC::Support::ExitCodes::AllGood;
  }
  
  
  
  return MiniMC::Support::ExitCodes::AllGood;
}

static CommandRegistrar mc_reg ("mc",mc_main,"Check whether it is possible to reach an assert violation. Classic reachability analysis. ",addOptions);


