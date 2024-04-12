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
  MiniMC::Model::TypeFactory_ptr tfac = std::make_shared<MiniMC::Model::TypeFactory64>();
  MiniMC::Model::ConstantFactory_ptr cfac = std::make_shared<MiniMC::Model::ConstantFactory64>(tfac);
  MiniMC::Support::Messager mess;
  auto path = std::filesystem::path {__FILE__}.parent_path () / s;

  MiniMC::Model::Modifications::ProgramManager manager;
  
  
  return manager(loader.loadFromFile (path,tfac,cfac,mess));
  
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


auto makeLoader () {
  auto registrar = MiniMC::Loaders::findLoader ("MMC");
  REQUIRE (registrar != nullptr);
  return registrar->makeLoader ();
}

TEST_CASE("Add") {
  MiniMC::Support::Messager mess;
  //Arrange
  auto loadRegistrar = makeLoader ();
  auto prgm = loadProgram (*loadRegistrar,"Add.mmc"); 
  
  MiniMC::CPA::AnalysisBuilder analysis_builder;
  analysis_builder.add<MiniMC::CPA::Concrete::CPA> ();
  auto initialState = analysis_builder.makeInitialState({
      prgm.getEntryPoints (),
      prgm.getHeapLayout (),
      prgm});

  //ACT 
  MiniMC::Algorithms::Reachability::Reachability reachabilityChecker {analysis_builder.makeTransfer (prgm)};
  auto res = reachabilityChecker.search (mess,initialState,goal);

  //Assert 
  CHECK (res.verdict() == MiniMC::Algorithms::Reachability::Verdict::Found);
}

TEST_CASE("Sub") {
  MiniMC::Support::Messager mess;
  //Arrange
  auto loadRegistrar = makeLoader ();
  auto prgm = loadProgram (*loadRegistrar,"Sub.mmc"); 
  
  MiniMC::CPA::AnalysisBuilder analysis_builder;
  analysis_builder.add<MiniMC::CPA::Concrete::CPA> ();
  auto initialState = analysis_builder.makeInitialState({
      prgm.getEntryPoints (),
      prgm.getHeapLayout (),
      prgm});

  //ACT 
  MiniMC::Algorithms::Reachability::Reachability reachabilityChecker {analysis_builder.makeTransfer (prgm)};
  auto res = reachabilityChecker.search (mess,initialState,goal);

  //Assert 
  CHECK (res.verdict() == MiniMC::Algorithms::Reachability::Verdict::Found);
}


TEST_CASE("Mul") {
  MiniMC::Support::Messager mess;
  //Arrange
  auto loadRegistrar = makeLoader ();
  auto prgm = loadProgram (*loadRegistrar,"Mul.mmc"); 
  
  MiniMC::CPA::AnalysisBuilder analysis_builder;
  analysis_builder.add<MiniMC::CPA::Concrete::CPA> ();
  auto initialState = analysis_builder.makeInitialState({
      prgm.getEntryPoints (),
      prgm.getHeapLayout (),
      prgm});

  //ACT 
  MiniMC::Algorithms::Reachability::Reachability reachabilityChecker {analysis_builder.makeTransfer (prgm)};
  auto res = reachabilityChecker.search (mess,initialState,goal);

  //Assert 
  CHECK (res.verdict() == MiniMC::Algorithms::Reachability::Verdict::Found);
}

TEST_CASE("UDiv") {
  MiniMC::Support::Messager mess;
  //Arrange
  auto loadRegistrar = makeLoader ();
  auto prgm = loadProgram (*loadRegistrar,"UDiv.mmc"); 
  
  MiniMC::CPA::AnalysisBuilder analysis_builder;
  analysis_builder.add<MiniMC::CPA::Concrete::CPA> ();
  auto initialState = analysis_builder.makeInitialState({
      prgm.getEntryPoints (),
      prgm.getHeapLayout (),
      prgm});

  //ACT 
  MiniMC::Algorithms::Reachability::Reachability reachabilityChecker {analysis_builder.makeTransfer (prgm)};
  auto res = reachabilityChecker.search (mess,initialState,goal);

  //Assert 
  CHECK (res.verdict() == MiniMC::Algorithms::Reachability::Verdict::Found);
}

TEST_CASE("SDiv") {
  MiniMC::Support::Messager mess;
  //Arrange
  auto loadRegistrar = makeLoader ();
  auto prgm = loadProgram (*loadRegistrar,"SDiv.mmc"); 
  
  MiniMC::CPA::AnalysisBuilder analysis_builder;
  analysis_builder.add<MiniMC::CPA::Concrete::CPA> ();
  auto initialState = analysis_builder.makeInitialState({
      prgm.getEntryPoints (),
      prgm.getHeapLayout (),
      prgm});

  //ACT 
  MiniMC::Algorithms::Reachability::Reachability reachabilityChecker {analysis_builder.makeTransfer (prgm)};
  auto res = reachabilityChecker.search (mess,initialState,goal);

  //Assert 
  CHECK (res.verdict() == MiniMC::Algorithms::Reachability::Verdict::Found);
}


TEST_CASE("Shl") {
  MiniMC::Support::Messager mess;
  //Arrange
  auto loadRegistrar = makeLoader ();
  auto prgm = loadProgram (*loadRegistrar,"Shl.mmc"); 
  
  MiniMC::CPA::AnalysisBuilder analysis_builder;
  analysis_builder.add<MiniMC::CPA::Concrete::CPA> ();
  auto initialState = analysis_builder.makeInitialState({
      prgm.getEntryPoints (),
      prgm.getHeapLayout (),
      prgm});

  //ACT 
  MiniMC::Algorithms::Reachability::Reachability reachabilityChecker {analysis_builder.makeTransfer (prgm)};
  auto res = reachabilityChecker.search (mess,initialState,goal);

  //Assert 
  CHECK (res.verdict() == MiniMC::Algorithms::Reachability::Verdict::Found);
}

TEST_CASE("AShr") {
  MiniMC::Support::Messager mess;
  //Arrange
  auto loadRegistrar = makeLoader ();
  auto prgm = loadProgram (*loadRegistrar,"Ashr.mmc"); 
  
  MiniMC::CPA::AnalysisBuilder analysis_builder;
  analysis_builder.add<MiniMC::CPA::Concrete::CPA> ();
  auto initialState = analysis_builder.makeInitialState({
      prgm.getEntryPoints (),
      prgm.getHeapLayout (),
      prgm});

  //ACT 
  MiniMC::Algorithms::Reachability::Reachability reachabilityChecker {analysis_builder.makeTransfer (prgm)};
  auto res = reachabilityChecker.search (mess,initialState,goal);

  //Assert 
  CHECK (res.verdict() == MiniMC::Algorithms::Reachability::Verdict::Found);
}

TEST_CASE("LShr") {
  MiniMC::Support::Messager mess;
  //Arrange
  auto loadRegistrar = makeLoader ();
  auto prgm = loadProgram (*loadRegistrar,"LShr.mmc"); 
  
  MiniMC::CPA::AnalysisBuilder analysis_builder;
  analysis_builder.add<MiniMC::CPA::Concrete::CPA> ();
  auto initialState = analysis_builder.makeInitialState({
      prgm.getEntryPoints (),
      prgm.getHeapLayout (),
      prgm});

  //ACT 
  MiniMC::Algorithms::Reachability::Reachability reachabilityChecker {analysis_builder.makeTransfer (prgm)};
  auto res = reachabilityChecker.search (mess,initialState,goal);

  //Assert 
  CHECK (res.verdict() == MiniMC::Algorithms::Reachability::Verdict::Found);
}

TEST_CASE("And") {
  MiniMC::Support::Messager mess;
  //Arrange
  auto loadRegistrar = makeLoader ();
  auto prgm = loadProgram (*loadRegistrar,"And.mmc"); 
  
  MiniMC::CPA::AnalysisBuilder analysis_builder;
  analysis_builder.add<MiniMC::CPA::Concrete::CPA> ();
  auto initialState = analysis_builder.makeInitialState({
      prgm.getEntryPoints (),
      prgm.getHeapLayout (),
      prgm});

  //ACT 
  MiniMC::Algorithms::Reachability::Reachability reachabilityChecker {analysis_builder.makeTransfer (prgm)};
  auto res = reachabilityChecker.search (mess,initialState,goal);

  //Assert 
  CHECK (res.verdict() == MiniMC::Algorithms::Reachability::Verdict::Found);
}

TEST_CASE("Or") {
  MiniMC::Support::Messager mess;
  //Arrange
  auto loadRegistrar = makeLoader ();
  auto prgm = loadProgram (*loadRegistrar,"Or.mmc"); 
  
  MiniMC::CPA::AnalysisBuilder analysis_builder;
  analysis_builder.add<MiniMC::CPA::Concrete::CPA> ();
  auto initialState = analysis_builder.makeInitialState({
      prgm.getEntryPoints (),
      prgm.getHeapLayout (),
      prgm});

  //ACT 
  MiniMC::Algorithms::Reachability::Reachability reachabilityChecker {analysis_builder.makeTransfer (prgm)};
  auto res = reachabilityChecker.search (mess,initialState,goal);

  //Assert 
  CHECK (res.verdict() == MiniMC::Algorithms::Reachability::Verdict::Found);
}

TEST_CASE("XOr") {
  MiniMC::Support::Messager mess;
  //Arrange
  auto loadRegistrar = makeLoader ();
  auto prgm = loadProgram (*loadRegistrar,"Xor.mmc"); 
  
  MiniMC::CPA::AnalysisBuilder analysis_builder;
  analysis_builder.add<MiniMC::CPA::Concrete::CPA> ();
  auto initialState = analysis_builder.makeInitialState({
      prgm.getEntryPoints (),
      prgm.getHeapLayout (),
      prgm});

  //ACT 
  MiniMC::Algorithms::Reachability::Reachability reachabilityChecker {analysis_builder.makeTransfer (prgm)};
  auto res = reachabilityChecker.search (mess,initialState,goal);

  //Assert 
  CHECK (res.verdict() == MiniMC::Algorithms::Reachability::Verdict::Found);
}

TEST_CASE("SGt") {
  MiniMC::Support::Messager mess;
  //Arrange
  auto loadRegistrar = makeLoader ();
  auto prgm = loadProgram (*loadRegistrar,"SGt.mmc"); 
  
  MiniMC::CPA::AnalysisBuilder analysis_builder;
  analysis_builder.add<MiniMC::CPA::Concrete::CPA> ();
  auto initialState = analysis_builder.makeInitialState({
      prgm.getEntryPoints (),
      prgm.getHeapLayout (),
      prgm});

  //ACT 
  MiniMC::Algorithms::Reachability::Reachability reachabilityChecker {analysis_builder.makeTransfer (prgm)};
  auto res = reachabilityChecker.search (mess,initialState,goal);

  //Assert 
  CHECK (res.verdict() == MiniMC::Algorithms::Reachability::Verdict::Found);
}

TEST_CASE("UGt") {
  MiniMC::Support::Messager mess;
  //Arrange
  auto loadRegistrar = makeLoader ();
  auto prgm = loadProgram (*loadRegistrar,"UGt.mmc"); 
  
  MiniMC::CPA::AnalysisBuilder analysis_builder;
  analysis_builder.add<MiniMC::CPA::Concrete::CPA> ();
  auto initialState = analysis_builder.makeInitialState({
      prgm.getEntryPoints (),
      prgm.getHeapLayout (),
      prgm});

  //ACT 
  MiniMC::Algorithms::Reachability::Reachability reachabilityChecker {analysis_builder.makeTransfer (prgm)};
  auto res = reachabilityChecker.search (mess,initialState,goal);

  //Assert 
  CHECK (res.verdict() == MiniMC::Algorithms::Reachability::Verdict::Found);
}






