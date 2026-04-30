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


TEST_CASE("main.ll") {

  MiniMC::Support::Interaction mess;
  MiniMC::ReachabilityChecker reachability{mess};
  reachability.setLoaderOption<std::vector<std::string>> ("LLVM.entry",{"main"});
  CHECK(reachability.search(std::filesystem::path {LOCATION} /"main.ll") == MiniMC::Result::NotReachable);
}

TEST_CASE("multiply2.ll") {

  MiniMC::Support::Interaction mess;
  MiniMC::ReachabilityChecker reachability{mess};
  reachability.setLoaderOption<std::vector<std::string>> ("LLVM.entry",{"main"});
  CHECK(reachability.search(std::filesystem::path {LOCATION} /"multiply1.ll") == MiniMC::Result::NotReachable); 
}

TEST_CASE("multiply2.ll") {

  MiniMC::Support::Interaction mess;
  MiniMC::ReachabilityChecker reachability{mess};
  reachability.setLoaderOption<std::vector<std::string>> ("LLVM.entry",{"main"});
  CHECK(reachability.search(std::filesystem::path {LOCATION} /"multiply2.ll") == MiniMC::Result::Reachable);
}

TEST_CASE("array.ll") {

  MiniMC::Support::Interaction mess;
  MiniMC::ReachabilityChecker reachability{mess};
  reachability.setLoaderOption<std::vector<std::string>> ("LLVM.entry",{"main"});
  CHECK(reachability.search(std::filesystem::path {LOCATION} /"array.ll") == MiniMC::Result::NotReachable);
}

TEST_CASE("arra2.ll") {

  MiniMC::Support::Interaction mess;
  MiniMC::ReachabilityChecker reachability{mess};
  reachability.setLoaderOption<std::vector<std::string>> ("LLVM.entry",{"main"});
  CHECK(reachability.search(std::filesystem::path {LOCATION} /"array2.ll") == MiniMC::Result::Reachable); 
}

TEST_CASE("switch.ll") {

  MiniMC::Support::Interaction mess;
  MiniMC::ReachabilityChecker reachability{mess};
  reachability.setLoaderOption<std::vector<std::string>> ("LLVM.entry",{"main"});
  CHECK(reachability.search(std::filesystem::path {LOCATION} /"switch.ll") == MiniMC::Result::NotReachable); 
}
