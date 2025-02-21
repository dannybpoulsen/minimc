#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"


#include "minimc/model/cfg.hpp"
#include "minimc/cpa/interface.hpp"
#include "minimc/cpa/concrete.hpp"
#include "minimc/model/modifications/modifications.hpp"
#include "minimc/algorithms/reachability.hpp"
#include "minimc/loaders/loader.hpp"
#include <filesystem>

auto loadProgram (auto& loader, const std::string& s) {
  MiniMC::Support::Messager mess;
  auto path = std::filesystem::path {__FILE__}.parent_path () / s;
  //return loader.makeLoader (tfac,cfac)->loadFromFile (path,mess);
  MiniMC::Model::Modifications::ProgramManager manager;
  manager.add<MiniMC::Model::Modifications::LowerPhi> ();
  manager.add<MiniMC::Model::Modifications::SplitAsserts> ();
  
  return manager (std::move(loader.loadFromFile (path,mess).value()));
}

auto goal (const MiniMC::CPA::AnalysisState& state) {
  auto& locationstate = state.getLocationState ();
  auto procs = locationstate.nbOfProcesses ();
  
  for (std::size_t i = 0; i < procs; ++i) {
    if (locationstate.getLocation (i).getInfo ().getFlags ().isSet (MiniMC::Model::Attributes::AssertViolated))
      return true;
  }
  
  return false;
};


TEST_CASE("Phi") {
  MiniMC::Support::Messager mess;
  //Arrange
  auto loadRegistrar = MiniMC::Loaders::findLoader ("LLVM");
  REQUIRE (loadRegistrar != nullptr);
  auto loader = loadRegistrar->makeLoader ();
  loader->setOption<std::vector<std::string>> (1,{"main"});
  auto prgm = loadProgram (*loader,"phi_atomic.ll");

  MiniMC::CPA::AnalysisBuilder analysis_builder;
  analysis_builder.add<MiniMC::CPA::CPAType::Concrete> ();
  auto initialState = analysis_builder.makeInitialState({prgm.getEntryPoints (),
      prgm.getHeapLayout (),
      prgm});
  
  //ACT 
  MiniMC::Algorithms::Reachability::Reachability reachabilityChecker {analysis_builder.makeTransfer (prgm),mess};
  auto res = reachabilityChecker.search (initialState,goal);

  //Assert 
  CHECK (res.verdict () == MiniMC::Algorithms::Reachability::Verdict::NotFound);
}

