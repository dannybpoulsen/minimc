#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"


#include "model/cfg.hpp"
#include "model/modifications/modifications.hpp"
#include "model/checkers/typechecker.hpp"
#include "cpa/interface.hpp"
#include "cpa/concrete.hpp"
#include "cpa/location.hpp"
#include "cpa/location.hpp"
#include "algorithms/reachability.hpp"
#include "support/feedback.hpp"
#include "loaders/loader.hpp"
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

auto makeLoader () {
  auto registrar = MiniMC::Loaders::findLoader ("LLVM");
  REQUIRE (registrar != nullptr);
  return registrar->makeLoader ();
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
  auto loadRegistrar = makeLoader ();//MiniMC::Loaders::findLoader ("LLVM");
  loadRegistrar->setOption<MiniMC::Loaders::VecStringOption> (1,{"main"});
  auto prgm = loadProgram (*loadRegistrar,"insert_extract_fail.ll");
  CHECK(MiniMC::Model::Checkers::TypeChecker{prgm}.Check (mess));
  
}

TEST_CASE("Frame") {
  MiniMC::Support::Messager mess;
  //Arrange
  auto loadRegistrar = makeLoader ();//MiniMC::Loaders::findLoader ("LLVM");
  loadRegistrar->setOption<MiniMC::Loaders::VecStringOption> (1,{"main"});
  auto prgm = loadProgram (*loadRegistrar,"insert_extract_fail.ll");
  

  MiniMC::CPA::AnalysisBuilder analysis_builder (std::make_shared<MiniMC::CPA::Location::CPA> ());
  analysis_builder.addDataCPA (std::make_shared<MiniMC::CPA::Concrete::CPA> ());
  auto initialState = analysis_builder.makeInitialState({prgm.getEntryPoints (),
      prgm.getHeapLayout (),
      prgm.getInitialiser (),
      prgm});

  //ACT 
  MiniMC::Algorithms::Reachability::Reachability reachabilityChecker {analysis_builder.makeTransfer (prgm)};
  auto res = reachabilityChecker.search (mess,initialState,goal);

  //Assert 
  CHECK (res.verdict ()  == MiniMC::Algorithms::Reachability::Verdict::Found);
}

TEST_CASE("Frame") {
  MiniMC::Support::Messager mess;
  //Arrange
  auto loadRegistrar = makeLoader (); //MiniMC::Loaders::findLoader ("LLVM");
  loadRegistrar->setOption<MiniMC::Loaders::VecStringOption> (1,{"main"});
  auto prgm = loadProgram (*loadRegistrar,"insert_extract_nofai.ll");
  
  MiniMC::CPA::AnalysisBuilder analysis_builder (std::make_shared<MiniMC::CPA::Location::CPA> ());
  analysis_builder.addDataCPA (std::make_shared<MiniMC::CPA::Concrete::CPA> ());
  auto initialState = analysis_builder.makeInitialState({prgm.getEntryPoints (),
      prgm.getHeapLayout (),
      prgm.getInitialiser (),
      prgm});

  //ACT 
  MiniMC::Algorithms::Reachability::Reachability reachabilityChecker {analysis_builder.makeTransfer (prgm)};
  auto res = reachabilityChecker.search (mess,initialState,goal);

  //Assert 
  CHECK (res.verdict ()== MiniMC::Algorithms::Reachability::Verdict::NotFound);
  
}
