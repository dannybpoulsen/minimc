#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include "minimc/model/modifications/modifications.hpp"
#include "minimc/model/cfg.hpp"
#include "minimc/cpa/interface.hpp"
#include "minimc/cpa/concrete.hpp"
#include "minimc/algorithms/reachability.hpp"
#include "minimc/loaders/loader.hpp"
#include <filesystem>

auto loadProgram (auto& loader, const std::string& s) {
  MiniMC::Support::Messager mess;
  auto path = std::filesystem::path {__FILE__}.parent_path () / s;

  MiniMC::Model::Modifications::ProgramManager manager;
  manager.add<MiniMC::Model::Modifications::LowerPhi> ();
  manager.add<MiniMC::Model::Modifications::SplitAsserts> ();
  
  
  return manager(std::move(loader.loadFromFile (path,mess).value()));
  
}

auto goal (const MiniMC::CPA::State& state) {
  auto& locationstate = state.getLocationState ();
  auto procs = locationstate.nbOfProcesses ();
  
  for (std::size_t i = 0; i < procs; ++i) {
    if (locationstate.getLocation (i).getInfo ().getFlags ().isSet (MiniMC::Model::Attributes::AssertViolated))
      return true;
  }
  
  return false;
};


auto makeLoader () {
  auto registrar = MiniMC::Loaders::findLoader ("LLVM");
  REQUIRE (registrar != nullptr);
  return registrar->makeLoader ();
}

TEST_CASE("Pointer") {
  MiniMC::Support::Messager mess;
  //Arrange
  auto loadRegistrar = makeLoader ();
  loadRegistrar->setOption<std::vector<std::string>> (1,{"main"});
  auto prgm = loadProgram (*loadRegistrar,"null_pointer_cmp.ll"); 
  
  MiniMC::CPA::AnalysisBuilder analysis_builder;
  auto  cpa = MiniMC::CPA::makeCPA<MiniMC::CPA::CPAType::Concrete> ();
  auto initialState = cpa->makeInitialState({prgm.getEntryPoints (),
      prgm.getHeapLayout (),
      prgm});

  //ACT 
  MiniMC::Algorithms::Reachability::Reachability reachabilityChecker {cpa->makeTransfer (prgm),mess};
  auto res = reachabilityChecker.search (*initialState,goal);

  //Assert 
  CHECK (res.verdict() == MiniMC::Algorithms::Reachability::Verdict::NotFound);
}

TEST_CASE("Pointer") {
  MiniMC::Support::Messager mess;
  //Arrange
  auto loadRegistrar = makeLoader ();//MiniMC::Loaders::findLoader ("LLVM");
  loadRegistrar->setOption<std::vector<std::string>>  (1,{"main"});
  auto prgm = loadProgram (*loadRegistrar,"null_pointer_cmp_2.ll");

  MiniMC::CPA::AnalysisBuilder analysis_builder;
  auto cpa = MiniMC::CPA::makeCPA<MiniMC::CPA::CPAType::Concrete> ();
  auto initialState = cpa->makeInitialState({prgm.getEntryPoints (),
      prgm.getHeapLayout (),
      prgm});

  //ACT 
  MiniMC::Algorithms::Reachability::Reachability reachabilityChecker {cpa->makeTransfer (prgm),mess};
  auto res = reachabilityChecker.search (*initialState,goal);

  //Assert 
  CHECK (res.verdict () == MiniMC::Algorithms::Reachability::Verdict::Found);
}

TEST_CASE("Pointer") {
  MiniMC::Support::Messager mess;
  //Arrange
  auto loadRegistrar = makeLoader ();
  loadRegistrar->setOption<std::vector<std::string>>  (1,{"main"});
  auto prgm = loadProgram (*loadRegistrar,"pointer_conversion.ll");


  MiniMC::CPA::AnalysisBuilder analysis_builder;
  auto cpa = MiniMC::CPA::makeCPA<MiniMC::CPA::CPAType::Concrete> ();
  auto initialState = cpa->makeInitialState({prgm.getEntryPoints (),
      prgm.getHeapLayout (),
      prgm});

  //ACT 
  MiniMC::Algorithms::Reachability::Reachability reachabilityChecker {cpa->makeTransfer (prgm),mess};
  auto res = reachabilityChecker.search (*initialState,goal);

  //Assert 
  CHECK (res.verdict() == MiniMC::Algorithms::Reachability::Verdict::Found);
}
