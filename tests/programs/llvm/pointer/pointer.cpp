#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include "minimc/model/modifications/modifications.hpp"
#include "minimc/model/cfg.hpp"
#include "minimc/cpa/interface.hpp"
#include "minimc/cpa/concrete.hpp"
#include "minimc/cpa/location.hpp"
#include "minimc/cpa/location.hpp"
#include "minimc/algorithms/reachability.hpp"
#include "minimc/loaders/loader.hpp"
#include <filesystem>

auto loadProgram (auto& loader, const std::string& s) {
  MiniMC::Model::TypeFactory_ptr tfac = std::make_shared<MiniMC::Model::TypeFactory64>();
  MiniMC::Model::ConstantFactory_ptr cfac = std::make_shared<MiniMC::Model::ConstantFactory64>(tfac);
  MiniMC::Support::Messager mess;
  auto path = std::filesystem::path {__FILE__}.parent_path () / s;

  MiniMC::Model::Modifications::ProgramManager manager;
  manager.add<MiniMC::Model::Modifications::LowerPhi> ();
  manager.add<MiniMC::Model::Modifications::SplitAsserts> ();
  
  
  return manager(loader.loadFromFile (path,tfac,cfac,mess));
  
}

auto goal (const MiniMC::CPA::AnalysisState& state) {
  auto& locationstate = state.getCFAState ().getLocationState ();
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
  loadRegistrar->setOption<MiniMC::Loaders::VecStringOption> (1,{"main"});
  auto prgm = loadProgram (*loadRegistrar,"null_pointer_cmp.ll"); 
  
  MiniMC::CPA::AnalysisBuilder analysis_builder (std::make_shared<MiniMC::CPA::Location::CPA> ());
  analysis_builder.addDataCPA (std::make_shared<MiniMC::CPA::Concrete::CPA> ());
  auto initialState = analysis_builder.makeInitialState({prgm.getEntryPoints (),
      prgm.getHeapLayout (),
      prgm});

  //ACT 
  MiniMC::Algorithms::Reachability::Reachability reachabilityChecker {analysis_builder.makeTransfer (prgm)};
  auto res = reachabilityChecker.search (mess,initialState,goal);

  //Assert 
  CHECK (res.verdict() == MiniMC::Algorithms::Reachability::Verdict::NotFound);
}

TEST_CASE("Pointer") {
  MiniMC::Support::Messager mess;
  //Arrange
  auto loadRegistrar = makeLoader ();//MiniMC::Loaders::findLoader ("LLVM");
  loadRegistrar->setOption<MiniMC::Loaders::VecStringOption> (1,{"main"});
  auto prgm = loadProgram (*loadRegistrar,"null_pointer_cmp_2.ll");

  MiniMC::CPA::AnalysisBuilder analysis_builder (std::make_shared<MiniMC::CPA::Location::CPA> ());
  analysis_builder.addDataCPA (std::make_shared<MiniMC::CPA::Concrete::CPA> ());
  auto initialState = analysis_builder.makeInitialState({prgm.getEntryPoints (),
      prgm.getHeapLayout (),
      prgm});

  //ACT 
  MiniMC::Algorithms::Reachability::Reachability reachabilityChecker {analysis_builder.makeTransfer (prgm)};
  auto res = reachabilityChecker.search (mess,initialState,goal);

  //Assert 
  CHECK (res.verdict () == MiniMC::Algorithms::Reachability::Verdict::Found);
}

TEST_CASE("Pointer") {
  MiniMC::Support::Messager mess;
  //Arrange
  auto loadRegistrar = makeLoader ();
  loadRegistrar->setOption<MiniMC::Loaders::VecStringOption> (1,{"main"});
  auto prgm = loadProgram (*loadRegistrar,"pointer_conversion.ll");


  MiniMC::CPA::AnalysisBuilder analysis_builder (std::make_shared<MiniMC::CPA::Location::CPA> ());
  analysis_builder.addDataCPA (std::make_shared<MiniMC::CPA::Concrete::CPA> ());
  auto initialState = analysis_builder.makeInitialState({prgm.getEntryPoints (),
      prgm.getHeapLayout (),
      prgm});

  //ACT 
  MiniMC::Algorithms::Reachability::Reachability reachabilityChecker {analysis_builder.makeTransfer (prgm)};
  auto res = reachabilityChecker.search (mess,initialState,goal);

  //Assert 
  CHECK (res.verdict() == MiniMC::Algorithms::Reachability::Verdict::Found);
}
