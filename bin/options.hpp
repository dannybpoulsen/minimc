#ifndef _OPTIONS__
#define _OPTIONS__

#include "minimc/smt/smt.hpp"
#include "minimc/cpa/interface.hpp"
#include "minimc/loaders/loader.hpp"

struct Command;

#include <vector>
#include <string>

struct SMTOption {
  MiniMC::Support::SMT::SMTDescr selsmt;
};


struct load_options{
  std::string inputname;
  MiniMC::Loaders::Loader_ptr loader{nullptr};
};

struct transform_options {
  bool expand_nondet{false};
  std::size_t unrollLoops{0};
  std::size_t inlineFunctions{0};
  bool lower_phi{true};
  };


struct SetupOptions {
  SMTOption smt;
  load_options load;
  transform_options transform;
  
  
  Command* command = nullptr;
  bool help;
  std::string outputname = "";
};


bool parseOptions(int argc, char* argv[],SetupOptions&);

#endif
