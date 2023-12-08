#include <boost/program_options.hpp>
#include <sstream>

#include "loaders/loader.hpp"
#include "algorithms/reachability.hpp"
#include "cpa/concrete.hpp"
#include "cpa/pathformula.hpp"


#include "options.hpp"
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
    bool symbolic{false};
  };
  
  class MCCommand :public Command {
  public:
    void addOptions (po::options_description& op) {
      
      auto setExpected= [this] (int val) {
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
      
      auto setSearchStrategy= [this] (const std::string val) {
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
	("mc.symbolic",po::bool_switch (&locoptions.symbolic),"Do a symbolic execution")
	
	;
      
      
      op.add(desc);
    }    

    
    MiniMC::Host::ExitCodes runCommand (MiniMC::Model::Program&& prgm, MiniMC::Support::Messager& messager,const SetupOptions& sopt) {    
      MiniMC::CPA::AnalysisBuilder cpa = makeCPABuilder (sopt);
      
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
      
    
      MiniMC::Algorithms::Reachability::Reachability reach {cpa.makeTransfer(prgm)};
      reach.setSearchStrategy (locoptions.search_strat);
      
      auto result = reach.search (messager,initstate,goal);
      
      
      if (result.verdict () == MiniMC::Algorithms::Reachability::Verdict::Found) {
	messager << MiniMC::Support::TInfo<std::string> {"Found Violation"};
	std::stringstream str;
	MiniMC::CPA::StateOutputter{prgm}.output (result.foundState(),str) << std::endl;
	messager << MiniMC::Support::TInfo<std::string> {str.str()};
	
	if (locoptions.expect == ExpectReach::Reachable)
	  return MiniMC::Host::ExitCodes::AllGood;
	else
	  return MiniMC::Host::ExitCodes::UnexpectedResult;
      }
      
      if (result.verdict () == MiniMC::Algorithms::Reachability::Verdict::NotFound) {
	messager <<  MiniMC::Support::TInfo<std::string> {"No violation found"};
	if (locoptions.expect == ExpectReach::Reachable)
	  return MiniMC::Host::ExitCodes::UnexpectedResult;
	else
	  return MiniMC::Host::ExitCodes::AllGood;
      }
      
      
      
      return MiniMC::Host::ExitCodes::AllGood;
    }
    
    
    std::string getName () const override {return "mc";}
    std::string getDescritpion () const override {return "Check whether it is possible to reach an assert violation. ";}
    
  private:
    MiniMC::CPA::AnalysisBuilder
    makeCPABuilder (const SetupOptions& sopt) {
      MiniMC::CPA::AnalysisBuilder cpa{std::make_shared<MiniMC::CPA::Location::CPA> ()};
      
      if (locoptions.symbolic)
	cpa.addDataCPA (std::make_shared<MiniMC::CPA::PathFormula::CPA>(sopt.smt.selsmt));
      
      else
	cpa.addDataCPA (std::make_shared<MiniMC::CPA::Concrete::CPA>());
      return cpa;
    }
    LocalOptions locoptions;
    
  };
} // namespace

static CommandRegistrar<MCCommand> mc_reg;
