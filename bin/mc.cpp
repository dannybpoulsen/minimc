#include <boost/program_options.hpp>
#include <sstream>

#include "minimc/loaders/loader.hpp"
#include "minimc/algorithms/reachability.hpp"
#include "minimc/cpa/interface.hpp"

#include "options.hpp"
#include "plugin.hpp"

#include <future>

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
      auto cpa = makeCPA (sopt);
      
      auto initstate = cpa->makeInitialState({prgm.getEntryPoints (),
	  prgm.getHeapLayout (),
	  prgm});
      
      auto goal = [](const MiniMC::CPA::State& state) {
	auto& locationstate = state.getLocationState ();
	auto procs = locationstate.nbOfProcesses ();
      
	for (std::size_t i = 0; i < procs; ++i) {
	  if (locationstate.isActive (i) && locationstate.getLocation (i).getInfo ().getFlags ().isSet (MiniMC::Model::Attributes::AssertViolated))
	    return true;
	}
	
	return false;
      };
      
    
      MiniMC::Algorithms::Reachability::Reachability reach {cpa->makeTransfer(prgm),messager};
      reach.setSearchStrategy (locoptions.search_strat);
      
      auto result = MiniMC::Support::AsyncExecutor{}.execute(messager,[&reach,&initstate,&goal](){return reach.search(*initstate,goal);});
      
      if (result.verdict () == MiniMC::Algorithms::Reachability::Verdict::Found) {
	messager << "Found Violation";
	std::stringstream str;
	MiniMC::CPA::CPAStateOutputter{prgm}.output (*result.foundState(),str) << std::endl;
	messager << str.str();
	
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
    MiniMC::CPA::TCPA_ptr makeCPA (const SetupOptions& sopt) {
      
      if (locoptions.symbolic)
	//cpa.add<MiniMC::CPA::CPAType::Pathformula>(sopt.smt.selsmt);
	return MiniMC::CPA::makeCPA<MiniMC::CPA::CPAType::Pathformula> (sopt.smt.selsmt);
      else
	return MiniMC::CPA::makeCPA<MiniMC::CPA::CPAType::Concrete> ();
      
    }
    LocalOptions locoptions;
    
  };
} // namespace

static CommandRegistrar<MCCommand> mc_reg;
