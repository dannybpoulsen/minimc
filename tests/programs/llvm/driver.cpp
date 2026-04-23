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



TEST_CASE("LLVM/programs/phi_atomic.ll") {

  MiniMC::Support::Interaction mess;
  MiniMC::ReachabilityChecker reachability{mess};
  reachability.setLoaderOption<std::vector<std::string>> ("LLVM.entry",{"main"});
  CHECK(reachability.search(std::filesystem::path {__FILE__}.parent_path () / "programs" /"phi_atomic.ll") == MiniMC::Result::NotReachable);; 
}

TEST_CASE("LLVM_programs_null_pointer_cmp.ll") {

  MiniMC::Support::Interaction mess;
  MiniMC::ReachabilityChecker reachability{mess};
  reachability.setLoaderOption<std::vector<std::string>> ("LLVM.entry",{"main"});
  CHECK(reachability.search(std::filesystem::path {__FILE__}.parent_path () / "programs" /"null_pointer_cmp.ll") == MiniMC::Result::NotReachable);; 
}

TEST_CASE("LLVM/programs/null_pointer_cmp_2.ll") {

  MiniMC::Support::Interaction mess;
  MiniMC::ReachabilityChecker reachability{mess};
  reachability.setLoaderOption<std::vector<std::string>> ("LLVM.entry",{"main"});
  CHECK(reachability.search(std::filesystem::path {__FILE__}.parent_path () / "programs" /"null_pointer_cmp_2.ll") == MiniMC::Result::Reachable);; 
}


TEST_CASE("LLVM/programs/pointer_conversion.ll") {

  MiniMC::Support::Interaction mess;
  MiniMC::ReachabilityChecker reachability{mess};
  reachability.setLoaderOption<std::vector<std::string>> ("LLVM.entry",{"main"});
  CHECK(reachability.search(std::filesystem::path {__FILE__}.parent_path () / "programs" /"pointer_conversion.ll") == MiniMC::Result::Reachable);; 
}

TEST_CASE("LLVM/programs/insert_extract_fail.ll") {

  MiniMC::Support::Interaction mess;
  MiniMC::ReachabilityChecker reachability{mess};
  reachability.setLoaderOption<std::vector<std::string>> ("LLVM.entry",{"main"});
  CHECK(reachability.search(std::filesystem::path {__FILE__}.parent_path () / "programs" /"insert_extract_fail.ll") == MiniMC::Result::Reachable);;
  }

TEST_CASE("LLVM/programs/insert_extract_nofai.ll") {

  MiniMC::Support::Interaction mess;
  MiniMC::ReachabilityChecker reachability{mess};
  reachability.setLoaderOption<std::vector<std::string>> ("LLVM.entry",{"main"});
  CHECK(reachability.search(std::filesystem::path {__FILE__}.parent_path () / "programs" /"insert_extract_nofai.ll") == MiniMC::Result::NotReachable);; 
}  


