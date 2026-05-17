#include <boost/program_options.hpp>
#include <sstream>

#include "minimc/cpa/state.hpp"
#include "minimc/loaders/loader.hpp"
#include "minimc/algorithms/reachability.hpp"
#include "minimc/cpa/interface.hpp"

#include "minimc/vm/vmt.hpp"
#include "options.hpp"
#include "plugin.hpp"

#include <future>

namespace po = boost::program_options;

namespace {
  
  
  struct LocalOptions {
    MiniMC::Algorithms::Reachability::SearchStrategy search_strat{MiniMC::Algorithms::Reachability::SearchStrategy::DFS};
    bool symbolic{false};
    bool concretize{false};
    bool all{false};
    };
  
  class MCCommand :public Command {
  public:
    void addOptions (po::options_description& op) {
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
	("mc.strategy",po::value<std::string> ()->default_value ({"DFS"})->notifier (setSearchStrategy),"Select search strategy\n"
	 "\t BFS\n"
	 "\t DFS\n"
	 )
	("mc.symbolic",po::bool_switch (&locoptions.symbolic),"Do a symbolic execution")
	("mc.concretize",po::bool_switch (&locoptions.concretize),"Concretize states")
	("mc.all",po::bool_switch (&locoptions.all),"Find all violations")
	
	;
      
      
      op.add(desc);
    }    

    
    MiniMC::Host::ExitCodes runCommand (MiniMC::Model::Program&& prgm, MiniMC::Support::Interaction& messager,const SetupOptions& sopt) {    
      auto cpa = makeCPA (sopt);
      
      auto initstate = cpa->makeInitialState({prgm.getEntryPoints (),
	  prgm.getHeapLayout (),
	  prgm});
      
      auto goal = [](const MiniMC::CPA::State& state) {
	return state.isSet (MiniMC::VMT::FlagType::AssertViolated);
      };
      
    
      MiniMC::Algorithms::Reachability::Reachability reach {cpa->makeTransfer(prgm),messager};

      std::unique_ptr<MiniMC::CPA::StateOutputter> outputter =  std::make_unique<MiniMC::CPA::CPAStateOutputter> (prgm);
      if  (locoptions.concretize)
	outputter = std::make_unique<MiniMC::CPA::CPAConcreteStateOutputter> (prgm,sopt.smt.selsmt.makeContext());
	
      auto result = MiniMC::Support::AsyncExecutor{}.execute(messager,[&reach,&initstate,&goal,this](){return reach.search(*initstate,goal,MiniMC::Algorithms::Reachability::DefaultFilter,locoptions.search_strat);});
      
      
      bool findMore = true;
      while (result.verdict () == MiniMC::Algorithms::Reachability::Verdict::Found && findMore) {
	messager.getMessager() << MiniMC::Support::TInfo<std::string> {"Found Violation"};
	
	outputter->output (*result.foundState(),messager.getMessager().raw_stream (MiniMC::Support::Severity::Info)) << "\n";
	
	findMore = locoptions.all;
	if (findMore)
	  result = MiniMC::Support::AsyncExecutor{}.execute(messager,[&reach](){return reach.continueSearch();});
	
	
      }
      
      messager.getMessager().raw_stream (MiniMC::Support::Severity::Info) << "\n" << "Searched " << result.exploredStates() << "\n";
	
      
      
      return MiniMC::Host::ExitCodes::AllGood;
    }
    
    
    std::string getName () const override {return "mc";}
    std::string getDescritpion () const override {return "Check whether it is possible to reach an assert violation. ";}
    
  private:
    MiniMC::CPA::TCPA_ptr makeCPA (const SetupOptions& sopt) {
      
      if (locoptions.symbolic)
	//cpa.add<MiniMC::CPA::CPAType::Pathformula>(sopt.smt.selsmt);
	return MiniMC::CPA::makeCPA<MiniMC::CPA::CPAType::Symbolic> ();
      else
	return MiniMC::CPA::makeCPA<MiniMC::CPA::CPAType::Concrete> ();
      
    }
    LocalOptions locoptions;
    
  };
} // namespace

static CommandRegistrar<MCCommand> mc_reg;
