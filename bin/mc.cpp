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


MiniMC::Host::ExitCodes mc_main (MiniMC::Model::Controller& controller, const MiniMC::CPA::AnalysisBuilder& cpa) {
  MiniMC::Support::Messager messager{};
  auto& prgm = *controller.getProgram ();
  if (prgm.getEntryPoints().size () <= 0) {
    messager. message<MiniMC::Support::Severity::Error>("Nothing to analyse --- No Entry Points in loaded program");
    return MiniMC::Host::ExitCodes::ConfigurationError;
  }

  messager.message("Initiating Reachability");
  auto initstate = cpa.makeInitialState({prgm.getEntryPoints (),
	prgm.getHeapLayout (),
	prgm.getInitialiser (),
	prgm});

  auto goal = [](const MiniMC::CPA::AnalysisState& state) {
    return state.getCFAState ()->getLocationState ().assertViolated ();
  };
  
  
  auto notify = [&messager](auto& t) {messager.message<MiniMC::Support::Severity::Progress> (t);};
  MiniMC::Algorithms::Reachability::Reachability reach {cpa.makeTransfer(prgm)};
  reach.getPWProgresMeasure ().listen (notify);
  auto verdict = reach.search (initstate,goal);
  messager.message("Finished Reachability");
  
  
  if (verdict == MiniMC::Algorithms::Reachability::Verdict::Found) {
    MiniMC::Support::getMessager ().message (MiniMC::Support::Localiser ("Found Violation").format ());
    std::cerr << reach.foundState () << std::endl;
    
    if (locoptions.expect == ExpectReach::Reachable)
      return MiniMC::Host::ExitCodes::AllGood;
    else
      return MiniMC::Host::ExitCodes::UnexpectedResult;
  }

  if (verdict == MiniMC::Algorithms::Reachability::Verdict::NotFound) {
    MiniMC::Support::getMessager ().message (MiniMC::Support::Localiser ("No violation found").format ());
    if (locoptions.expect == ExpectReach::Reachable)
      return MiniMC::Host::ExitCodes::UnexpectedResult;
    else
      return MiniMC::Host::ExitCodes::AllGood;
  }
  
  
  
  return MiniMC::Host::ExitCodes::AllGood;
}

static CommandRegistrar mc_reg ("mc",mc_main,"Check whether it is possible to reach an assert violation. Classic reachability analysis. ",addOptions);


