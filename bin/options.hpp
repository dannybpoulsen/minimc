#ifndef _OPTIONS__
#define _OPTIONS__

#include "support/smt.hpp"
#include "cpa/interface.hpp"
#include "cpa/concrete.hpp"

struct CommandRegistrar;

#include <vector>
#include <string>

struct SMTOption {
  MiniMC::Support::SMT::SMTDescr selsmt;
};



struct Modifications {
  bool expandNonDet = false;
  bool replaceNonDetUniform = false;
  bool simplifyCFG = false;
  bool replaceSub = false;
  bool splitCMPS = false;
  bool foldConstants = false;
  bool convergencePoints = false;
  bool removeAllocs = false;
  bool replacememnodet = false;
  bool removephi = false;
  std::size_t inlinefunctions = 0;
  std::size_t unrollLoops = 0;
  };

struct load_options{
  std::string inputname;
  std::vector<std::string> tasks;
};



struct SetupOptions {
  Modifications modifications;
  SMTOption smt;
  load_options load;
  
  MiniMC::CPA::CPA_ptr cpa;
  CommandRegistrar* command = nullptr;
  bool help;
  std::string outputname = "";
};


bool parseOptions(int argc, char* argv[],SetupOptions&);

#endif
