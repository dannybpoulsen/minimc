#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"


#include "minimc/model/cfg.hpp"
#include "minimc/model/modifications/modifications.hpp"
#include "minimc/model/checkers/typechecker.hpp"
#include "minimc/cpa/interface.hpp"
#include "minimc/cpa/concrete.hpp"
#include "minimc/algorithms/reachability.hpp"
#include "minimc/support/feedback.hpp"
#include "minimc/loaders/loader.hpp"
#include <filesystem>


auto loadProgram (auto& loader, const std::string& s) {
  MiniMC::Support::Messager mess;
  auto path = std::filesystem::path {__FILE__}.parent_path () / s;
  
  MiniMC::Model::Modifications::ProgramManager manager;
  manager.add<MiniMC::Model::Modifications::LowerPhi> ();
  //manager.add<MiniMC::Model::Modifications::SplitAsserts> ();
  
  
  return manager(std::move(loader.loadFromFile (path,mess)).value());
  
}

auto makeLoader () {
  auto registrar = MiniMC::Loaders::findLoader ("LLVM");
  REQUIRE (registrar != nullptr);
  return registrar->makeLoader ();
}


auto goal (const MiniMC::CPA::State& state) {
  return state.isSet (MiniMC::VMT::FlagType::AssertViolated);	
};

TEST_CASE("Frame") {
  //Arrange
  MiniMC::Support::Messager mess;
  auto loadRegistrar = makeLoader ();//MiniMC::Loaders::findLoader ("LLVM");
  loadRegistrar->setOption<std::vector<std::string> > ("LLVM.entry",{"main"});
  auto prgm = loadProgram (*loadRegistrar,"insert_extract_fail.ll");
  CHECK(MiniMC::Model::Checkers::TypeChecker{mess}.Check (prgm));
  
}

TEST_CASE("Frame") {
  MiniMC::Support::Interaction mess;
  //Arrange
  auto loadRegistrar = makeLoader ();//MiniMC::Loaders::findLoader ("LLVM");
  loadRegistrar->setOption<std::vector<std::string>> ("LLVM.entry",{"main"});
  auto prgm = loadProgram (*loadRegistrar,"insert_extract_fail.ll");
  

  auto cpa = MiniMC::CPA::makeCPA<MiniMC::CPA::CPAType::Concrete> ();
  auto initialState = cpa->makeInitialState({prgm.getEntryPoints (),
      prgm.getHeapLayout (),
      prgm});

  //ACT 
  MiniMC::Algorithms::Reachability::Reachability reachabilityChecker {cpa->makeTransfer (prgm),mess};
  auto res = reachabilityChecker.search (*initialState,goal);

  //Assert 
  CHECK (res.verdict ()  == MiniMC::Algorithms::Reachability::Verdict::Found);
}

TEST_CASE("Frame") {
  MiniMC::Support::Messager mess;
  //Arrange
  auto loadRegistrar = makeLoader (); //MiniMC::Loaders::findLoader ("LLVM");
  loadRegistrar->setOption<std::vector<std::string> > ("LLVM.entry",{"main"});
  auto prgm = loadProgram (*loadRegistrar,"insert_extract_nofai.ll");
  

  auto cpa = MiniMC::CPA::makeCPA<MiniMC::CPA::CPAType::Concrete> ();
  auto initialState = cpa->makeInitialState({prgm.getEntryPoints (),
      prgm.getHeapLayout (),
      prgm});

  //ACT 
  MiniMC::Algorithms::Reachability::Reachability reachabilityChecker {cpa->makeTransfer (prgm),mess};
  auto res = reachabilityChecker.search (*initialState,goal);

  //Assert 
  CHECK (res.verdict ()== MiniMC::Algorithms::Reachability::Verdict::NotFound);
  
}
