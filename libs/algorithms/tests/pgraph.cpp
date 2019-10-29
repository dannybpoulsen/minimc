#include "model/types.hpp"
#include "support/feedback.hpp"
#include "algorithms/algorithm.hpp"
#include "algorithms/successorgen.hpp"
#include "algorithms/passedwaiting.hpp"
#include "algorithms/printgraph.hpp"
#include "cpa/compound.hpp"
#include "cpa/location.hpp"
#include "loaders/loader.hpp"



int main (int argc,char* argv[]) {
  auto loader = MiniMC::Loaders::makeLoader<MiniMC::Loaders::Type::LLVM> ();
  MiniMC::Model::TypeFactory_ptr tfac = std::make_shared<MiniMC::Model::TypeFactory64> ();
  auto prgm = loader->loadFromFile (argv[1],tfac);
  auto mess = MiniMC::Support::makeMessager (MiniMC::Support::MessagerType::Terminal);

  MiniMC::CPA::Compounds::StateQuery<0,MiniMC::CPA::Location::CPADef> tezt;
  auto hh = tezt.makeInitialState (*prgm);
  hh->output (std::cout);
  
  
  //MiniMC::Algorithms::PrintCPA<MiniMC::CPA::Location::CPADef> algorithm (*mess);
  MiniMC::Algorithms::PrintCPA<MiniMC::CPA::Compounds::CPADef<0,MiniMC::CPA::Location::CPADef>> algorithm (*mess);
  
  algorithm.run (*prgm);
  
}
