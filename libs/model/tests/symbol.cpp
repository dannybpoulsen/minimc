#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN


#include "doctest/doctest.h"


#include <model/symbol.hpp>

TEST_CASE ("Simple Symbol") {
  const std::string s = "main";
  MiniMC::Model::Symbol smb {s};
  CHECK (smb.getName () == s);
}

TEST_CASE ("Name Fullname") {
  const std::string s = "main";
  const std::string s2 = "suff";
  MiniMC::Model::Symbol smb {s};
  MiniMC::Model::Symbol smb2 {smb,s2};
  
  CHECK (smb2.getName () == s2);
  CHECK (smb2.getFullName () == s+std::string(":")+s2);
}

TEST_CASE ("From String") {
  const std::string s = "main:main:tmp";
  auto  smb = MiniMC::Model::Symbol::from_string (s); 
  CHECK (smb.getName () == "tmp");
  CHECK (smb.getFullName () == "main:main:tmp");
}


TEST_CASE ("") {
  const std::string s = "";
  auto  smb = MiniMC::Model::Symbol::from_string (s); 
  CHECK (smb.getName () == "");
  CHECK (smb.getFullName () == "");
}


