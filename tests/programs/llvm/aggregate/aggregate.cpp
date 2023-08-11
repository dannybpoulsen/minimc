#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"


#include "model/controller.hpp"
#include "model/cfg.hpp"
#include "model/checkers/typechecker.hpp"
#include "cpa/interface.hpp"
#include "cpa/concrete.hpp"
#include "cpa/location.hpp"
#include "cpa/location.hpp"
#include "algorithms/reachability/reachability.hpp"
#include "support/feedback.hpp"
#include "loaders/loader.hpp"
#include <filesystem>

auto loadProgram (MiniMC::Loaders::LoaderRegistrar& loader, const std::string& s) {
  MiniMC::Model::TypeFactory_ptr tfac = std::make_shared<MiniMC::Model::TypeFactory64>();
  MiniMC::Model::ConstantFactory_ptr cfac = std::make_shared<MiniMC::Model::ConstantFactory64>(tfac);
  MiniMC::Support::Messager mess;
  auto path = std::filesystem::path {__FILE__}.parent_path () / s;
  return loader.makeLoader (tfac,cfac)->loadFromFile (path,mess);
  
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

TEST_CASE("Frame") {
  //Arrange
  MiniMC::Support::Messager mess;
  auto loadRegistrar = MiniMC::Loaders::findLoader ("LLVM");
  REQUIRE (loadRegistrar != nullptr);
  loadRegistrar->setOption<MiniMC::Loaders::VecStringOption> (1,{"main"});
  auto prgm = loadProgram (*loadRegistrar,"insert_extract_fail.ll");
  CHECK(MiniMC::Model::Checkers::TypeChecker{prgm}.Check (mess));
  
}

TEST_CASE("Frame") {
  MiniMC::Support::Messager mess;
  //Arrange
  auto loadRegistrar = MiniMC::Loaders::findLoader ("LLVM");
  REQUIRE (loadRegistrar != nullptr);
  loadRegistrar->setOption<MiniMC::Loaders::VecStringOption> (1,{"main"});
  auto prgm = loadProgram (*loadRegistrar,"insert_extract_fail.ll");
  MiniMC::Model::Controller control(prgm);
  control.createAssertViolateLocations ();
  

  MiniMC::CPA::AnalysisBuilder analysis_builder (std::make_shared<MiniMC::CPA::Location::CPA> ());
  analysis_builder.addDataCPA (std::make_shared<MiniMC::CPA::Concrete::CPA> ());
  auto initialState = analysis_builder.makeInitialState({prgm.getEntryPoints (),
      prgm.getHeapLayout (),
      prgm.getInitialiser (),
      prgm});

  //ACT 
  MiniMC::Algorithms::Reachability::Reachability reachabilityChecker {analysis_builder.makeTransfer (prgm)};
  auto verdict = reachabilityChecker.search (mess,initialState,goal);

  //Assert 
  CHECK (verdict == MiniMC::Algorithms::Reachability::Verdict::Found);
}

TEST_CASE("Frame") {
  MiniMC::Support::Messager mess;
  //Arrange
  auto loadRegistrar = MiniMC::Loaders::findLoader ("LLVM");
  REQUIRE (loadRegistrar != nullptr);
  loadRegistrar->setOption<MiniMC::Loaders::VecStringOption> (1,{"main"});
  auto prgm = loadProgram (*loadRegistrar,"insert_extract_nofai.ll");
  MiniMC::Model::Controller control(prgm);
  control.createAssertViolateLocations ();
  
  MiniMC::CPA::AnalysisBuilder analysis_builder (std::make_shared<MiniMC::CPA::Location::CPA> ());
  analysis_builder.addDataCPA (std::make_shared<MiniMC::CPA::Concrete::CPA> ());
  auto initialState = analysis_builder.makeInitialState({prgm.getEntryPoints (),
      prgm.getHeapLayout (),
      prgm.getInitialiser (),
      prgm});

  //ACT 
  MiniMC::Algorithms::Reachability::Reachability reachabilityChecker {analysis_builder.makeTransfer (prgm)};
  auto verdict = reachabilityChecker.search (mess,initialState,goal);

  //Assert 
  CHECK (verdict == MiniMC::Algorithms::Reachability::Verdict::NotFound);
  
}
