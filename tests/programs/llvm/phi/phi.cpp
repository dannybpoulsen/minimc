#include "minimc/support/feedback.hpp"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"


#include "minimc/model/cfg.hpp"
#include "minimc/cpa/interface.hpp"
#include "minimc/cpa/concrete.hpp"
#include "minimc/model/modifications/modifications.hpp"
#include "minimc/algorithms/reachability.hpp"
#include "minimc/loaders/loader.hpp"
#include <filesystem>

#include "minimc/minimc.hpp"



TEST_CASE("Phi") {

  MiniMC::Support::Interaction mess;
  MiniMC::ReachabilityChecker reachability{mess};
  reachability.setLoaderOption<std::vector<std::string>> ("LLVM.entry",{"main"});
  CHECK(reachability.search(std::filesystem::path {__FILE__}.parent_path () / "phi_atomic.ll") == MiniMC::Result::NotReachable);; 

  //CHECK (res.verdict () == MiniMC::Algorithms::Reachability::Verdict::NotFound);
}

