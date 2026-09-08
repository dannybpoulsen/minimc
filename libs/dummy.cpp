#include "minimc/cpa/interface.hpp"
#include "minimc/minimc.hpp"
#include "minimc/model/modifications/modifications.hpp"
#include "minimc/algorithms/reachability.hpp"


namespace MiniMC {
  bool ReachabilityChecker::selectCPA (MiniMC::CPA::CPAType type) {
    switch (type) {
    case MiniMC::CPA::CPAType::Concrete:
      cpa = MiniMC::CPA::makeCPA<MiniMC::CPA::CPAType::Concrete> ();
      break;
    case MiniMC::CPA::CPAType::Symbolic:
      cpa = MiniMC::CPA::makeCPA<MiniMC::CPA::CPAType::Symbolic> ();
      break;
    case MiniMC::CPA::CPAType::Probabilistic:
      cpa = MiniMC::CPA::makeCPA<MiniMC::CPA::CPAType::Probabilistic> (10UL);
      break;
      
    }
    return true;
  }

  Result ReachabilityChecker::search(const std::string name) {
    MiniMC::Model::Modifications::ProgramManager manager;
    manager.add<MiniMC::Model::Modifications::LowerPhi> ();
    manager.add<MiniMC::Model::Modifications::SplitAsserts> ();
    if (auto prgm = loader.loadFromFile (name,interaction.getMessager())) {
      auto prgm2 = manager(std::move(prgm.value()));
      auto initialState = cpa->makeInitialState({
	  prgm2.getEntryPoints (),
	  prgm2.getHeapLayout (),
	  prgm2}
	);
      
      auto goal =  [](const MiniMC::CPA::State& state) {
	return state.isSet (MiniMC::VMT::FlagType::AssertViolated);
      };

      MiniMC::Algorithms::Reachability::Reachability reachabilityChecker {cpa->makeTransfer (prgm2),interaction};
      auto res = reachabilityChecker.search (*initialState,goal);
      switch (res.verdict()) {
      case MiniMC::Algorithms::Reachability::Verdict::NotFound:
	return Result::NotReachable;
      case MiniMC::Algorithms::Reachability::Verdict::Found:
	return Result::Reachable;
      }
    }
    return Result::NotReachable;
      
  }
    
  
} // namespace MiniMC


