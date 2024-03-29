#ifndef _OPTIONS__
#define _OPTIONS__

#include "smt/smt.hpp"
#include "cpa/interface.hpp"
#include "cpa/location.hpp"
#include "loaders/loader.hpp"

struct CommandRegistrar;

#include <vector>
#include <string>

struct SMTOption {
  MiniMC::Support::SMT::SMTDescr selsmt;
};


struct load_options{
  std::string inputname;
  MiniMC::Loaders::LoaderRegistrar* registrar{nullptr};
};

struct transform_options {
  bool expand_nondet{false};
  std::size_t unrollLoops{0};
  std::size_t inlineFunctions{0};
  
  };


struct SetupOptions {
  SMTOption smt;
  load_options load;
  transform_options transform;
  
  
  MiniMC::CPA::AnalysisBuilder cpa{std::make_shared<MiniMC::CPA::Location::CPA> ()};
  CommandRegistrar* command = nullptr;
  bool help;
  std::string outputname = "";
};


bool parseOptions(int argc, char* argv[],SetupOptions&);

#endif
